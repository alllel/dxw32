cmake_minimum_required(VERSION 3.28)
project(dxw32)

set(CMAKE_CXX_STANDARD 20)

file(GLOB DXW_SOURCES "*.cpp")
add_executable(dxw32 ${DXW_SOURCES} dxw.o)
target_sources(dxw32 PRIVATE dxw.rc)

target_link_options(dxw32 PUBLIC -mwindows -static-libgcc -static-libstdc++)
#target_link_libraries(dxw32 -lwinmm -lgdi32 -lcomdlg32)

add_custom_command(
        OUTPUT dxw.o
        COMMAND i686-w64-mingw32-windres ${CMAKE_CURRENT_SOURCE_DIR}/dxw.rc dxw.o
        DEPENDS dxw.rc
)