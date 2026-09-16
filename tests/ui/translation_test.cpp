/*
 * Copyright (C) 2026 klogg contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <catch2/catch.hpp>

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
#include <QTabWidget>
#include <QTemporaryDir>
#include <QTranslator>

#include "configuration.h"
#include "mainwindow.h"
#include "optionsdialog.h"
#include "scratchpad.h"

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
    struct RestoreLanguage {
        QString original = Configuration::get().language();
        ~RestoreLanguage()
        {
            Configuration::get().setLanguage( original );
            MainWindow::installLanguage( "en" );
        }
    } restoreLanguage;

    Configuration::get().setLanguage( "zh_CN" );
    REQUIRE( MainWindow::installLanguage( "zh_CN" ) == 0 );

#ifdef Q_OS_WIN
    // The offscreen plugin may not enumerate Windows font fallbacks itself.
    const auto chineseFont = qEnvironmentVariable( "WINDIR" ) + "/Fonts/msyh.ttc";
    if ( QFile::exists( chineseFont ) ) {
        QFontDatabase::addApplicationFont( chineseFont );
    }
#endif

    const auto directory = QCoreApplication::applicationDirPath() + "/i18n-screenshots";
    REQUIRE( QDir().mkpath( directory ) );
    auto saveScreenshot = [ &directory ]( QWidget& widget, const QString& name ) {
        widget.show();
        QCoreApplication::processEvents();
        REQUIRE( widget.grab().save( directory + "/" + name + ".png" ) );
    };

    auto session = std::make_shared<Session>();
    MainWindow mainWindow( WindowSession{ session, "i18n", 0 } );
    mainWindow.resize( 1100, 700 );
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
    saveScreenshot( scratchpad, "scratchpad-zh_CN" );
}
