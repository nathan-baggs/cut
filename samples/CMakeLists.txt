add_executable(
  sample
  main.cpp
)

target_include_directories(
  sample
  PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

target_compile_features(
  sample
  PRIVATE
    cxx_std_26
)

target_compile_options(
  sample
  PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Werror
   -freflection-latest
)

