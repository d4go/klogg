#!/usr/bin/env python3
"""Validate Qt TS translations without requiring Qt or third-party packages."""

import argparse
from collections import Counter
from html.parser import HTMLParser
import json
from pathlib import Path
import re
import sys
import xml.etree.ElementTree as ET


DEFAULT_TS = Path(__file__).resolve().parents[1] / "src/app/i18n/zh_CN.ts"
PLACEHOLDER = re.compile(r"%(?:L?[1-9][0-9]*|L?n)")
RICH_TEXT = re.compile(r"</?(?:html|body|p|br|b|i|a|span|div|style|head|h[1-6])\b", re.I)
# Reviewed UI strings that are intentionally language-independent.
UNCHANGED = {"klogg", "<h2>klogg</h2>", "MB", "TiB", "GiB", "MiB", "KiB", "B", "ms", "s", "v", "...",
             "%1 (%2)", "%1 - %2", "%1", "Alt+A", "Alt+S", "UTF-8", "UTF-16",
             "GB18030", "ASCII", "URL", "HTTP", "HTTPS", "SSL", "AVX", "SSE",
             "Qt", "Hyperscan", "Unicode"}


class Markup(HTMLParser):
    def __init__(self):
        super().__init__(convert_charrefs=True)
        self.structure = []
        self.text = []

    def handle_starttag(self, tag, attrs):
        self.structure.append(("start", tag, sorted(attrs)))

    def handle_startendtag(self, tag, attrs):
        self.structure.append(("empty", tag, sorted(attrs)))

    def handle_endtag(self, tag):
        self.structure.append(("end", tag))

    def handle_data(self, data):
        self.text.append(data)


def visible_text(value):
    if not RICH_TEXT.search(value):
        return value
    parser = Markup()
    parser.feed(value)
    return "".join(parser.text)


def mnemonics(value):
    return Counter(letter.casefold() for letter in
                   re.findall(r"&([^&\s])", visible_text(value).replace("&&", "")))


def validate(path):
    root = ET.parse(path).getroot()
    stats = Counter(total=0, active=0, completed=0, unfinished=0, empty=0,
                    identical=0, obsolete=0, vanished=0)
    errors, warnings = [], []
    keys = set()
    for context in root.findall("context"):
        context_name = context.findtext("name", "")
        for message in context.findall("message"):
            stats["total"] += 1
            source = message.findtext("source", "")
            translation = message.find("translation")
            state = translation.get("type", "") if translation is not None else "unfinished"
            if state in ("obsolete", "vanished"):
                stats[state] += 1
                continue
            stats["active"] += 1
            location = message.find("location")
            where = context_name
            if location is not None:
                where += f" ({location.get('filename', '')}:{location.get('line', '')})"
            label = f"{where}: {source!r}"
            key = (context_name, source, message.findtext("comment", ""))
            if key in keys:
                errors.append(f"Duplicate active message: {label}")
            keys.add(key)
            if state == "unfinished":
                stats["unfinished"] += 1
                errors.append(f"Unfinished: {label}")
            forms = translation.findall("numerusform") if translation is not None else []
            values = ["".join(form.itertext()) for form in forms]
            if not forms:
                values = ["".join(translation.itertext()) if translation is not None else ""]
            if any(not value.strip() for value in values):
                stats["empty"] += 1
                errors.append(f"Empty translation: {label}")
            elif state != "unfinished":
                stats["completed"] += 1
            if all(value == source for value in values):
                stats["identical"] += 1
                if source.strip() not in UNCHANGED and re.search(r"[A-Za-z]{2,}", source):
                    warnings.append(f"Review unchanged text: {label}")
            for value in values:
                if Counter(PLACEHOLDER.findall(source)) != Counter(PLACEHOLDER.findall(value)):
                    errors.append(f"Placeholder mismatch: {label}")
                if mnemonics(source) != mnemonics(value):
                    errors.append(f"Mnemonic mismatch: {label}")
                if source.count("\n") != value.count("\n"):
                    errors.append(f"Newline mismatch: {label}")
                if RICH_TEXT.search(source):
                    original, translated = Markup(), Markup()
                    original.feed(source)
                    translated.feed(value)
                    if original.structure != translated.structure:
                        errors.append(f"Rich-text structure/attribute mismatch: {label}")
                elif (re.search(r"[A-Za-z]{3,}(?:[ ,]+[A-Za-z]{3,}){3,}", value)
                      and not re.search(r"[\u3400-\u9fff]", value)
                      and source != value):
                    warnings.append(f"Review English text: {label}")
    return {"file": str(path), "statistics": dict(stats), "errors": errors, "warnings": warnings}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("path", nargs="?", type=Path, default=DEFAULT_TS)
    parser.add_argument("--report-json", type=Path)
    args = parser.parse_args()
    try:
        result = validate(args.path)
    except (OSError, ET.ParseError) as error:
        print(f"ERROR: {error}", file=sys.stderr)
        return 1
    print(json.dumps(result["statistics"], ensure_ascii=False))
    for severity in ("errors", "warnings"):
        for issue in result[severity]:
            print(f"{severity.upper()}: {issue}")
    if args.report_json:
        args.report_json.write_text(json.dumps(result, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    print(f"Translation validation: {'FAIL' if result['errors'] else 'PASS'}")
    return int(bool(result["errors"]))


if __name__ == "__main__":
    sys.exit(main())
