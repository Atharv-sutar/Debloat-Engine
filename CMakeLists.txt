cmake_minimum_required(VERSION 3.16)
project(DeBloat)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

set(DEBLOAT_BACKEND_SOURCES
    src/CommandExecutor.cpp
    src/AdbManager.cpp
    src/PackageManager.cpp
    src/PackageClassifier.cpp
    src/UserInterface.cpp
    src/RemovalEngine.cpp
    src/Logger.cpp
)

add_library(DebloatBackend STATIC ${DEBLOAT_BACKEND_SOURCES})
target_include_directories(DebloatBackend PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
if(MSVC)
    target_compile_options(DebloatBackend PRIVATE /W4)
else()
    target_compile_options(DebloatBackend PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(DeBloat src/main_production.cpp)
target_link_libraries(DeBloat PRIVATE DebloatBackend)
target_include_directories(DeBloat PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

include(FetchContent)

FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.89.9
)
FetchContent_MakeAvailable(imgui)

find_package(OpenGL REQUIRED)

add_library(imgui_static STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl2.cpp
)

target_include_directories(imgui_static PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

target_link_libraries(imgui_static PUBLIC OpenGL::GL)

add_executable(DeBloatGui src/main_gui.cpp src/GuiApp.cpp)
target_link_libraries(DeBloatGui PRIVATE DebloatBackend imgui_static OpenGL::GL user32 gdi32 dwmapi)
target_include_directories(DeBloatGui PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
