add_library(
  cut
  coro/event_loop.cpp
  ${PROJECT_SOURCE_DIR}/third_party/sqlite3/sqlite3.c
)

target_include_directories(
  cut
  PUBLIC
    ${PROJECT_SOURCE_DIR}/include/
    ${PROJECT_SOURCE_DIR}/include/cut
    ${PROJECT_SOURCE_DIR}/third_party
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
)


