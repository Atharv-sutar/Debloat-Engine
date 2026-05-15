cmake_minimum_required(VERSION 3.16)
project(DeBloat)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

set(SOURCES
    src/main_production.cpp
    src/CommandExecutor.cpp
    src/AdbManager.cpp
    src/PackageManager.cpp
    src/PackageClassifier.cpp
    src/UserInterface.cpp
)

add_executable(DeBloat ${SOURCES})

if(MSVC)
    target_compile_options(DeBloat PRIVATE /W4)
else()
    target_compile_options(DeBloat PRIVATE -Wall -Wextra -Wpedantic)
endif()
