cmake_minimum_required(VERSION 3.20)
project(chain LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---- SDL3: скачивается и собирается вместе с проектом (статически, без DLL) ----
include(FetchContent)
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON  CACHE BOOL "" FORCE)
set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    SDL3
    URL https://github.com/libsdl-org/SDL/archive/refs/tags/release-3.4.20.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(SDL3)

add_executable(chain
    main.cpp
    DE2solver.cpp
    plot.cpp
    plotwindow.cpp
)
target_link_libraries(chain PRIVATE SDL3::SDL3-static)

if (MSVC)
    target_compile_options(chain PRIVATE /utf-8)
endif()
