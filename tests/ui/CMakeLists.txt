set(UI_TEST_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/qtests_main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/logdata_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/logfiltereddata_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/crawlerwidget_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/translation_test.cpp
)

if(NOT APPLE)
    LIST(APPEND UI_TEST_SOURCES  ${CMAKE_CURRENT_SOURCE_DIR}/mainwindow_test.cpp)
endif()

add_executable(klogg_itests ${UI_TEST_SOURCES} ${BACKWARD_ENABLE})

target_link_libraries(klogg_itests klogg_ui klogg_utils Catch2 Qt${QT_VERSION_MAJOR}::Test test_utils)
set_target_properties(klogg_itests PROPERTIES AUTOMOC ON)

# OptionsDialog's public header includes the UI header generated for klogg_ui.
get_target_property(KLOGG_UI_AUTOGEN_DIR klogg_ui AUTOGEN_BUILD_DIR)
if(NOT KLOGG_UI_AUTOGEN_DIR)
    get_target_property(KLOGG_UI_BINARY_DIR klogg_ui BINARY_DIR)
    set(KLOGG_UI_AUTOGEN_DIR "${KLOGG_UI_BINARY_DIR}/klogg_ui_autogen")
endif()
get_property(KLOGG_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(KLOGG_MULTI_CONFIG)
    target_include_directories(klogg_itests PRIVATE "${KLOGG_UI_AUTOGEN_DIR}/include_$<CONFIG>")
else()
    target_include_directories(klogg_itests PRIVATE "${KLOGG_UI_AUTOGEN_DIR}/include")
endif()

# Exercise the same embedded catalogs that are shipped in the application.
get_target_property(KLOGG_APP_BINARY_DIR klogg BINARY_DIR)
target_sources(klogg_itests PRIVATE
    ${PROJECT_SOURCE_DIR}/src/app/klogg.qrc
    ${KLOGG_APP_BINARY_DIR}/app_translations.qrc
    ${KLOGG_APP_BINARY_DIR}/qt_translations.qrc
)
set_target_properties(klogg_itests PROPERTIES AUTORCC ON)
add_dependencies(klogg_itests klogg)

add_backward(klogg_itests)

add_test(
    NAME klogg_itests
    COMMAND klogg_itests "~[i18n]" -platform offscreen
)

# Start localization in a fresh process: shortcut labels are cached on first use.
add_test(
    NAME klogg_i18n
    COMMAND klogg_itests "[i18n]" -platform offscreen
)
set_tests_properties(klogg_itests klogg_i18n PROPERTIES RESOURCE_LOCK klogg_ui_settings)
