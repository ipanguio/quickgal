cmake_minimum_required(VERSION 3.0.0)
project(quickgal C CXX)

# Automatically create moc files
set(CMAKE_AUTOMOC ON)

execute_process(
    COMMAND dpkg-architecture -qDEB_HOST_MULTIARCH
    OUTPUT_VARIABLE ARCH_TRIPLET
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

set(QT_IMPORTS_DIR "lib/${ARCH_TRIPLET}")

set(PROJECT_NAME "quickgal")
set(FULL_PROJECT_NAME "quickgal.nacho")
set(DATA_DIR /)
set(DESKTOP_FILE_NAME ${PROJECT_NAME}.desktop)

# Select QML components depending on Ubuntu Touch framework
if("$ENV{CLICK_FRAMEWORK}" MATCHES "16\\.04")
    set(QML_COMPONENTS_IMPORT "import Ubuntu.Components 1.3")
    set(QML_POPUPS_IMPORT "import Ubuntu.Components.Popups 1.3")
    set(QML_THUMBNAILER_IMPORT "import Ubuntu.Thumbnailer 0.1")
    set(QML_CONTENT_IMPORT "import Ubuntu.Content 1.3")
else()
    set(QML_COMPONENTS_IMPORT "import Lomiri.Components 1.3")
    set(QML_POPUPS_IMPORT "import Lomiri.Components.Popups 1.3")
    set(QML_THUMBNAILER_IMPORT "import Lomiri.Thumbnailer 0.1")
    set(QML_CONTENT_IMPORT "import Lomiri.Content 1.3")
endif()

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/qml/Main.qml.in
    ${CMAKE_CURRENT_BINARY_DIR}/qml/Main.qml
    @ONLY
)

# Translations
file(GLOB_RECURSE I18N_SRC_FILES
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/po
    qml/*.qml
    qml/*.qml.in
    qml/*.js
)

list(APPEND I18N_SRC_FILES
    ${DESKTOP_FILE_NAME}.in.h
)

# Find intltool BEFORE processing po/CMakeLists.txt 
find_program(INTLTOOL_MERGE intltool-merge)
IF (NOT INTLTOOL_MERGE)
    message(FATAL_ERROR "Could not find intltool-merge, please install the intltool package")
ENDIF()

find_program(INTLTOOL_EXTRACT intltool-extract)
IF (NOT INTLTOOL_EXTRACT)
    message(FATAL_ERROR "Could not find intltool-extract, please install the intltool package")
ENDIF()

add_subdirectory(po)

configure_file(
    manifest.json.in
    ${CMAKE_CURRENT_BINARY_DIR}/manifest.json
)
install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/manifest.json
    DESTINATION .
)
install(
    FILES ${PROJECT_NAME}.apparmor
    DESTINATION ${DATA_DIR}
)
install(DIRECTORY assets DESTINATION ${DATA_DIR})
install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/qml/Main.qml
    DESTINATION qml
)

add_custom_target(${DESKTOP_FILE_NAME} ALL
    COMMENT "Merging translations into ${DESKTOP_FILE_NAME}..."
    COMMAND LC_ALL=C ${INTLTOOL_MERGE} -d -u ${CMAKE_SOURCE_DIR}/po ${CMAKE_SOURCE_DIR}/${DESKTOP_FILE_NAME}.in ${DESKTOP_FILE_NAME}
    COMMAND sed -i 's/${PROJECT_NAME}-//g' ${CMAKE_CURRENT_BINARY_DIR}/${DESKTOP_FILE_NAME}
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${DESKTOP_FILE_NAME} DESTINATION ${DATA_DIR})

# Make source files visible in QtCreator
file(GLOB_RECURSE PROJECT_SRC_FILES
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    qml/*.qml
    qml/*.js
    src/*
    *.json
    *.json.in
    *.apparmor
    *.desktop.in
)

add_custom_target(${PROJECT_NAME}_FILES ALL SOURCES ${PROJECT_SRC_FILES})

find_package(Qt5 REQUIRED COMPONENTS
    Core
    Gui
    Qml
    Quick
    Multimedia
)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/MediaScanner.cpp
    src/AlbumModel.cpp
    src/MediaModel.cpp
    src/AppController.cpp
    src/ThumbnailProvider.cpp
)

target_link_libraries(${PROJECT_NAME}
    Qt5::Core
    Qt5::Gui
    Qt5::Qml
    Qt5::Quick
    Qt5::Multimedia
)

install(
    TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION .
)
