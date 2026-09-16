/*
 * Copyright (C) 2026 klogg contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <catch2/catch.hpp>

#include <QApplication>
#include <QCoreApplication>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDir>
#include <QFile>
#include <QFontDatabase>
#include <QLocale>
#include <QPushButton>
#include <QPixmap>
#include <QSettings>
#include <QSignalSpy>
#include <QTabWidget>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QTest>
#include <QTranslator>

#include "configuration.h"
#include "mainwindow.h"
#include "optionsdialog.h"
#include "recentfiles.h"
#include "scratchpad.h"
#include "sessioninfo.h"
#include "test_utils.h"

TEST_CASE( "Default interface language follows the system script", "[i18n]" )
{
    const auto simplified = { "zh_CN", "zh_SG", "zh_Hans", "zh_Hans_CN" };
    for ( const auto* locale : simplified ) {
        CAPTURE( locale );
        REQUIRE( Configuration::defaultLanguageForLocale( QLocale( locale ) ) == "zh_CN" );
    }

    const auto traditional = { "zh_TW", "zh_HK", "zh_MO", "zh_Hant" };
    for ( const auto* locale : traditional ) {
        CAPTURE( locale );
        REQUIRE( Configuration::defaultLanguageForLocale( QLocale( locale ) ) == "zh_TW" );
    }

    const auto other = { "en_US", "en_GB", "fr_FR", "ja_JP", "C" };
    for ( const auto* locale : other ) {
        CAPTURE( locale );
        REQUIRE( Configuration::defaultLanguageForLocale( QLocale( locale ) ) == "en" );
    }
}

TEST_CASE( "Saved interface language overrides the first-run default", "[i18n]" )
{
    QTemporaryDir directory;
    REQUIRE( directory.isValid() );
    QSettings settings( directory.filePath( "language.ini" ), QSettings::IniFormat );

    Configuration firstRun;
    firstRun.retrieveFromStorage( settings );
    REQUIRE( firstRun.language()
             == Configuration::defaultLanguageForLocale( QLocale::system() ) );

    const auto languages = { "en", "zh_CN", "zh_TW" };
    for ( const auto* language : languages ) {
        CAPTURE( language );
        firstRun.setLanguage( language );
        firstRun.saveToStorage( settings );
        settings.sync();
        REQUIRE( settings.status() == QSettings::NoError );

        QSettings reloadedSettings( directory.filePath( "language.ini" ), QSettings::IniFormat );
        Configuration reloaded;
        reloaded.retrieveFromStorage( reloadedSettings );
        REQUIRE( reloaded.language() == language );
    }
}

TEST_CASE( "Embedded catalogs translate Chinese and restore English", "[i18n]" )
{
    struct RestoreEnglish {
        ~RestoreEnglish()
        {
            MainWindow::installLanguage( "en" );
        }
    } restoreEnglish;

    REQUIRE( QFile::exists( ":/i18n/zh_CN.qm" ) );
    REQUIRE( QFile::exists( ":/i18n/qt_zh_CN.qm" ) );
    QTranslator traditional;
    REQUIRE( traditional.load( ":/i18n/zh_TW.qm" ) );

    REQUIRE( MainWindow::installLanguage( "en" ) == 0 );
    QDialogButtonBox englishButtons( QDialogButtonBox::Cancel );
    const auto englishCancel = englishButtons.button( QDialogButtonBox::Cancel )->text();
    REQUIRE( QCoreApplication::translate( "OptionsDialog", "Restore Default Shortcuts" )
             == "Restore Default Shortcuts" );

    REQUIRE( MainWindow::installLanguage( "zh_CN" ) == 0 );
    const auto translated
        = QCoreApplication::translate( "OptionsDialog", "Restore Default Shortcuts" );
    REQUIRE_FALSE( translated.isEmpty() );
    REQUIRE( translated != "Restore Default Shortcuts" );
    REQUIRE( QCoreApplication::translate( "ScratchPad", "From base64" ) != "From base64" );
    QDialogButtonBox chineseButtons( QDialogButtonBox::Cancel );
    REQUIRE( chineseButtons.button( QDialogButtonBox::Cancel )->text() != englishCancel );

    REQUIRE( MainWindow::installLanguage( "en" ) == 0 );
    REQUIRE( QCoreApplication::translate( "OptionsDialog", "Restore Default Shortcuts" )
             == "Restore Default Shortcuts" );
    QDialogButtonBox restoredButtons( QDialogButtonBox::Cancel );
    REQUIRE( restoredButtons.button( QDialogButtonBox::Cancel )->text() == englishCancel );
}

TEST_CASE( "Chinese dialogs render with the packaged resources", "[i18n]" )
{
    struct RestoreUiState {
        QString original = Configuration::get().language();
        QFont originalFont = QApplication::font();
        RecentFiles recentFiles = RecentFiles::getSynced();
        SessionInfo sessionInfo = SessionInfo::getSynced();
        int addedFont = -1;
        ~RestoreUiState()
        {
            Configuration::get().setLanguage( original );
            MainWindow::installLanguage( "en" );
            QApplication::setFont( originalFont );
            if ( addedFont >= 0 ) {
                QFontDatabase::removeApplicationFont( addedFont );
            }
            RecentFiles::get() = recentFiles;
            RecentFiles::get().save();
            SessionInfo::get() = sessionInfo;
            SessionInfo::get().save();
        }
    } restoreUiState;

    Configuration::get().setLanguage( "zh_CN" );
    REQUIRE( MainWindow::installLanguage( "zh_CN" ) == 0 );

#ifdef Q_OS_WIN
    // The offscreen plugin may not enumerate Windows font fallbacks itself.
    const auto fontDirectory = qEnvironmentVariable( "WINDIR" ) + "/Fonts/";
    const auto chineseFonts = { "msyh.ttc", "simsun.ttc", "msjh.ttc", "mingliu.ttc" };
    for ( const auto* fileName : chineseFonts ) {
        const auto fontId = QFontDatabase::addApplicationFont( fontDirectory + fileName );
        if ( fontId >= 0 ) {
            const auto families = QFontDatabase::applicationFontFamilies( fontId );
            if ( !families.isEmpty() ) {
                QApplication::setFont( QFont( families.front(), 9 ) );
                restoreUiState.addedFont = fontId;
                break;
            }
            QFontDatabase::removeApplicationFont( fontId );
        }
    }
    if ( restoreUiState.addedFont < 0 ) {
        WARN( "No Windows CJK font found; screenshots may contain missing-glyph boxes." );
    }
#endif

    const auto directory = QCoreApplication::applicationDirPath() + "/i18n-screenshots";
    REQUIRE( QDir().mkpath( directory ) );
    auto saveScreenshot = [ &directory ]( QWidget& widget, const QString& name ) {
        widget.show();
        QCoreApplication::processEvents();
        REQUIRE( widget.grab().save( directory + "/" + name + ".png" ) );
    };

    QTemporaryFile sampleLog( QDir::temp().filePath( "klogg-i18n-XXXXXX.log" ) );
    REQUIRE( sampleLog.open() );
    const QByteArray sampleData(
        u8"2026-09-16 09:00:00 INFO  English log: application started\n"
        u8"2026-09-16 09:00:01 INFO  \u4e2d\u6587\u65e5\u5fd7\uff1a\u6587\u4ef6\u52a0\u8f7d\u6210\u529f\n"
        u8"2026-09-16 09:00:02 ERROR \u6df7\u5408 English \u65e5\u5fd7 TraceId=123 \U0001F600\n" );
    REQUIRE( sampleLog.write( sampleData ) == sampleData.size() );
    REQUIRE( sampleLog.flush() );
    sampleLog.close();

    auto session = std::make_shared<Session>();
    MainWindow mainWindow( WindowSession{ session, "i18n", 0 } );
    mainWindow.resize( 1100, 700 );
    mainWindow.show();
    mainWindow.loadInitialFile( sampleLog.fileName(), false );
    auto crawler = mainWindow.findChild<CrawlerWidget*>();
    REQUIRE( crawler != nullptr );
    SafeQSignalSpy loaded( crawler, SIGNAL( loadingFinished( LoadingStatus ) ) );
    REQUIRE( loaded.isValid() );
    REQUIRE( loaded.safeWait() );
    REQUIRE( loaded.back().front().value<LoadingStatus>() == LoadingStatus::Successful );
    saveScreenshot( mainWindow, "main-window-zh_CN" );

    OptionsDialog options( &mainWindow );
    options.resize( 1000, 740 );
    auto languages = options.findChild<QComboBox*>( "languageComboBox" );
    REQUIRE( languages != nullptr );
    REQUIRE( languages->findData( "en" ) >= 0 );
    REQUIRE( languages->findData( "zh_CN" ) >= 0 );
    REQUIRE( languages->findData( "zh_TW" ) >= 0 );
    REQUIRE( languages->currentData().toString() == "zh_CN" );
    auto tabs = options.findChild<QTabWidget*>( "tabWidget" );
    REQUIRE( tabs != nullptr );
    for ( int index = 0; index < tabs->count(); ++index ) {
        tabs->setCurrentIndex( index );
        saveScreenshot( options, QString( "options-%1-zh_CN" ).arg( index ) );
    }
    options.hide();

    ScratchPad scratchpad;
    scratchpad.resize( 1100, 650 );
    scratchpad.addData( QString::fromUtf8( sampleData ) );
    saveScreenshot( scratchpad, "scratchpad-zh_CN" );
}
