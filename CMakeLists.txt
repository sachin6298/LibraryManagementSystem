cmake_minimum_required(VERSION 3.22)

project(LibraryManagementSystem VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()
include(GNUInstallDirs)

qt_add_executable(LibraryManagementSystem
    main.cpp
    mainwindow.cpp
    mainwindow.h
    include/Book.h
    include/BookList.h
    src/BookList.cpp
    resources/library_resources.qrc
)

target_include_directories(LibraryManagementSystem PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
target_link_libraries(LibraryManagementSystem PRIVATE Qt6::Core Qt6::Widgets)

# The .ico is embedded into the native Windows executable only.
if(WIN32)
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/resources/library_icon.rc.in"
        "${CMAKE_CURRENT_BINARY_DIR}/library_icon.rc" @ONLY
    )
    target_sources(LibraryManagementSystem PRIVATE
        "${CMAKE_CURRENT_BINARY_DIR}/library_icon.rc"
    )
endif()

install(TARGETS LibraryManagementSystem
    BUNDLE DESTINATION .
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
)

# Qt supplies platform-specific runtime dependency deployment.
qt_generate_deploy_app_script(
    TARGET LibraryManagementSystem
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT "${deploy_script}")
