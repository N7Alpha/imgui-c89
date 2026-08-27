cmake_minimum_required(VERSION 3.20)
project(imgui_c89_translator LANGUAGES C CXX)

find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)

if(NOT LLVM_VERSION_MAJOR EQUAL 22)
    message(FATAL_ERROR
        "The translator currently pins LLVM/Clang major 22; found ${LLVM_PACKAGE_VERSION}")
endif()

message(STATUS "Using LLVM ${LLVM_PACKAGE_VERSION}")

add_executable(imgui-clang-extract clang_extract/main.cpp)
target_compile_features(imgui-clang-extract PRIVATE cxx_std_17)
target_include_directories(imgui-clang-extract PRIVATE
    ${LLVM_INCLUDE_DIRS}
    ${CLANG_INCLUDE_DIRS})
target_compile_definitions(imgui-clang-extract PRIVATE ${LLVM_DEFINITIONS})
target_link_libraries(imgui-clang-extract PRIVATE clang-cpp LLVM)

enable_testing()
