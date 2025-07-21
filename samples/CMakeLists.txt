add_executable(
  sample
  main.cpp
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

target_link_libraries(
  sample
  PUBLIC
    cut
)
