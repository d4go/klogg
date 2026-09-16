"""Small regression fixtures for the translation validation gate."""

from pathlib import Path
import tempfile
import unittest
from xml.sax.saxutils import escape

from check_translation import validate


class TranslationValidationTests(unittest.TestCase):
    def check(self, source, translation, attribute="", numerus=False):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "test.ts"
            content = (translation if numerus else escape(translation))
            path.write_text(
                '<?xml version="1.0"?><TS language="zh_CN"><context><name>Test</name>'
                f'<message><source>{escape(source)}</source>'
                f'<translation {attribute}>{content}</translation></message></context></TS>',
                encoding="utf-8",
            )
            return validate(path)

    def test_complete_translation_and_case_insensitive_mnemonic(self):
        result = self.check("&Open %1", "打开(&O) %1")
        self.assertEqual(result["errors"], [])
        self.assertEqual(self.check("Find &next", "查找下一个(&N)")["errors"], [])

    def test_unfinished_and_empty_are_rejected(self):
        result = self.check("Open", "", 'type="unfinished"')
        self.assertEqual(result["statistics"]["unfinished"], 1)
        self.assertEqual(result["statistics"]["empty"], 1)
        self.assertTrue(result["errors"])

    def test_placeholder_multiplicity_and_localized_forms(self):
        self.assertTrue(self.check("%1 %1 %L2 %n", "%1 %L2 %n")["errors"])
        self.assertTrue(self.check("%L1", "%1")["errors"])
        self.assertEqual(self.check("%L1 %2", "%2 %L1")["errors"], [])

    def test_numerus_forms_are_each_validated(self):
        result = self.check("%n file(s)", "<numerusform>%n 个文件</numerusform>", numerus=True)
        self.assertEqual(result["errors"], [])
        result = self.check("%n file(s)", "<numerusform>文件</numerusform>", numerus=True)
        self.assertTrue(result["errors"])

    def test_rich_text_attributes_and_linebreaks_are_preserved(self):
        source = '<p><a href="https://example.com">Open %1</a></p>\n'
        self.assertEqual(self.check(source, '<p><a href="https://example.com">打开 %1</a></p>\n')["errors"], [])
        self.assertTrue(self.check(source, '<p><a href="https://invalid.example">打开 %1</a></p>\n')["errors"])
        self.assertTrue(self.check(source, '<p><a href="https://example.com">打开 %1</a></p>')["errors"])

    def test_obsolete_entries_do_not_block_release(self):
        result = self.check("Removed", "", 'type="vanished"')
        self.assertEqual(result["statistics"]["active"], 0)
        self.assertEqual(result["errors"], [])

    def test_untranslated_prose_is_reported_but_units_are_allowed(self):
        self.assertTrue(self.check("Open file", "Open file")["warnings"])
        self.assertEqual(self.check("GiB", "GiB")["warnings"], [])


if __name__ == "__main__":
    unittest.main()
