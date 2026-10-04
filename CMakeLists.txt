cmake_minimum_required(VERSION 3.22)
project(silk)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

FetchContent_Declare(
    cxxopts
    GIT_REPOSITORY https://github.com/jarro2783/cxxopts
    GIT_TAG v3.3.1
)
# Skip cxxopts' own examples/tests
set(CXXOPTS_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(CXXOPTS_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(cxxopts)

add_executable(${PROJECT_NAME}
    main.cpp
    tgaimage.h
    tgaimage.cpp
    model.h
    model.cpp
    config.h
    config.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE cxxopts::cxxopts)
