cmake_minimum_required(VERSION 3.16)

project(DB VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 下面这两行提高编译速度指令除单元测试时均可删除
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g0")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O1")

# 添加头文件搜索路径
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools)

set(TS_FILES DB_zh_CN.ts)

set(PROJECT_SOURCES
    main.cpp
    highlighttextedit.h
    highlighttextedit.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    lexer.cpp
    lexer.h
    loginwindow.cpp
    loginwindow.h
    loginwindow.ui
    resource.qrc
    utils.cpp
    utils.h
    dbManager.cpp
    dbManager.h
    databaselistdialog.cpp
    databaselistdialog.h
    usermanage.cpp
    usermanage.h
    wrong.cpp
    wrong.ui
    wrong.h
    admin.cpp
    admin.h
    ${TS_FILES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(DB
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )

    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    if(ANDROID)
        add_library(DB SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(DB
            ${PROJECT_SOURCES}
        )
    endif()

    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

target_link_libraries(DB PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_include_directories(DB PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.DB)
endif()

set_target_properties(DB PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS DB
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(DB)
endif()
