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