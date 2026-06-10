cmake_minimum_required(VERSION 3.16)
project(production_flash_test_rig LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

enable_testing()

add_library(flash_rig_core
  src/FlashRig.cpp
)
target_include_directories(flash_rig_core PUBLIC include)
target_compile_options(flash_rig_core PRIVATE -Wall -Wextra -Wpedantic)

add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE flash_rig_core)
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)

add_executable(flash_rig_tests tests/FlashRigTests.cpp)
target_link_libraries(flash_rig_tests PRIVATE flash_rig_core)
target_compile_options(flash_rig_tests PRIVATE -Wall -Wextra -Wpedantic)
add_test(NAME flash_rig_tests COMMAND flash_rig_tests)
