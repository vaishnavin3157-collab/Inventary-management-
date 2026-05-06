cmake_minimum_required(VERSION 3.14)
project(HybridInventoryManager LANGUAGES C CXX)

set(CMAKE_C_STANDARD   11)
set(CMAKE_CXX_STANDARD 17)

# Compiler warnings for both languages
add_compile_options(-Wall -Wextra -pedantic)

include_directories(include)

add_executable(inventory_manager
    src/inventory.c
    src/InventoryManager.cpp
    src/main.cpp
)

# Ensure each file is compiled with the correct language compiler
set_source_files_properties(src/inventory.c PROPERTIES LANGUAGE C)
set_source_files_properties(src/InventoryManager.cpp src/main.cpp PROPERTIES LANGUAGE CXX)

install(TARGETS inventory_manager DESTINATION bin)
