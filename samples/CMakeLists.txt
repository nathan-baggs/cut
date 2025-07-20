add_executable(
  sample
  main.cpp
)

target_include_directories(
  sample
  PRIVATE
    ${PROJECT_SOURCE_DIR}/include/
    ${PROJECT_SOURCE_DIR}/include/cut
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
    -fsanitize=address
)

target_link_libraries(
  sample
  PUBLIC
    cut
)

target_link_options(
  sample
  PRIVATE
    -fsanitize=address
)
