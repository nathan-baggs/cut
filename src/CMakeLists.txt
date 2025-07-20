add_library(
  cut
  coro/event_loop.cpp
)

target_include_directories(
  cut
  PRIVATE
    ${PROJECT_SOURCE_DIR}/include/
    ${PROJECT_SOURCE_DIR}/include/cut
)

target_compile_features(
  cut
  PRIVATE
    cxx_std_26
)

target_compile_options(
  cut
  PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Werror
    -freflection-latest
    -fsanitize=address
)


