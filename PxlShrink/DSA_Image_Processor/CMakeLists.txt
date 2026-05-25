cmake_minimum_required(VERSION 3.16)

project(DSA_Image_Project
    VERSION 2.4.0
    LANGUAGES CXX
    DESCRIPTION "Image Processing & Compression System — Bahria University DSA Project"
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt6 Auto-tools
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Find Qt6 Widgets
find_package(Qt6 REQUIRED COMPONENTS Widgets)

# ---- Sources ----
set(SOURCES
    main.cpp
    MainWindow.cpp
    DecompressDialog.cpp
    Image.cpp
    ImageProcessor.cpp
    HuffmanCompressor.cpp
)

# ---- Headers (for MOC) ----
set(HEADERS
    MainWindow.h
    DecompressDialog.h
    Image.h
    ImageProcessor.h
    HuffmanCompressor.h
    Matrix.h
    Pixel.h
)

# ---- Executable ----
add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
)

# Link Qt6 Widgets
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets)

# Windows: hide console in Release builds
if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE $<CONFIG:Release>
    )
endif()
