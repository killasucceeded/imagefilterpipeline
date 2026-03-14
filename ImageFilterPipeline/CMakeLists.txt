cmake_minimum_required(VERSION 3.22)
project(ImageFilterPipeline VERSION 1.0.0 LANGUAGES CXX)

# ── C++ standard ──────────────────────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ── Compiler warnings ─────────────────────────────────────────────────────────
add_compile_options(
  -Wall -Wextra -Wpedantic
  -Wshadow -Wnon-virtual-dtor
  -Wold-style-cast -Wcast-align
  -Woverloaded-virtual
  $<$<CONFIG:Debug>:-g>
  $<$<CONFIG:Release>:-O2>
)

# ── Main executable ───────────────────────────────────────────────────────────
add_executable(ifp
  src/main.cpp
)

# src/ for project headers, third_party/ for stb_image
target_include_directories(ifp PRIVATE src third_party)

# ── Tests ─────────────────────────────────────────────────────────────────────
include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.14.0
)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(ifp_tests
  tests/test_main.cpp
)

target_include_directories(ifp_tests PRIVATE src third_party)

target_link_libraries(ifp_tests
  PRIVATE
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(ifp_tests)
