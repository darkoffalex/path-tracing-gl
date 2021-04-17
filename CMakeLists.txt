# Версия CMake
cmake_minimum_required(VERSION 3.14)

# Название проекта (решение в Visual Studio)
project(GpuPathTracing)

# Стандарт С/С++
set(CMAKE_CXX_STANDARD 14)

# Устанавливаем каталоги для бинарников
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Bin)

# Определить разрядность платформы
if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "4")
    set(PLATFORM_BIT_SUFFIX "x86")
else()
    set(PLATFORM_BIT_SUFFIX "x64")
endif()

# Старндартные C/C++ библиотеки для MinGW/GCC (не для Visual Studio)
if(NOT MSVC)
    set(CMAKE_CXX_STANDARD_LIBRARIES "-static-libgcc -static-libstdc++ -lwsock32 -lws2_32 ${CMAKE_CXX_STANDARD_LIBRARIES}")
endif()

# Библилтека вспомогательных инструментов (header-only)
add_subdirectory("Sources/Common")

# Примеры реализаций
add_subdirectory("Sources/01_Basic")
add_subdirectory("Sources/02_Progressive")