add_library(
  cut
)

target_sources(
  cut
  PUBLIC
  FILE_SET cxx_modules TYPE CXX_MODULES
  FILES
  simple.cpp
)

set_target_properties(
  cut
  PROPERTIES
  CXX_STANDARD 26
  CXX_STANDARD_REQUIRED ON
)

target_compile_features(
  cut
  PUBLIC
  cxx_std_26
)

target_compile_options(
  cut
  PUBLIC
  -Wall
  -Wextra
  -Wpedantic
)

