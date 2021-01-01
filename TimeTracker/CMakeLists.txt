# CMakeList.txt : CMake project for TimeTracker, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
	add_definitions("-W4" "-WX")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GCC")
	add_definitions("-Wall" "-Werror")
endif()

# Add source to this project's executable.
file(GLOB source_code ${PROJECT_SOURCE_DIR}/TimeTracker/src/*.cpp)
file(GLOB_RECURSE source_headers ${PROJECT_SOURCE_DIR}/TimeTracker/include/*.h)
set(SOURCES ${source_code} ${source_headers})
include_directories(${PROJECT_SOURCE_DIR}/TimeTracker/include)
add_executable (TimeTracker ${SOURCES} ${PROJECT_SOURCE_DIR}/TimeTracker/Main.cpp)

set(gtest_force_shared_crt TRUE)
add_subdirectory(gtest)

add_subdirectory(test)