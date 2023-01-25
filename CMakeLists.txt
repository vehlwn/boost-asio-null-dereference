cmake_minimum_required(VERSION 3.16)

project(boost-asio-null-dereference LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_COLOR_DIAGNOSTICS ON)

add_executable(a main.cpp)
find_package(Boost REQUIRED COMPONENTS coroutine)
target_include_directories(a SYSTEM PUBLIC "${Boost_INCLUDE_DIRS}")
target_link_libraries(a PUBLIC "${Boost_LIBRARIES}")

include(CheckCXXCompilerFlag)
set(FLAG -Wnull-dereference)
check_cxx_compiler_flag(${FLAG} HAS_Wnull_dereference)
if(HAS_Wnull_dereference)
  target_compile_options(a PRIVATE ${FLAG})
endif()

include(CMakePrintHelpers)
cmake_print_variables(Boost_LIBRARIES)
cmake_print_variables(Boost_INCLUDE_DIRS)
