cmake_minimum_required(VERSION 3.8)


file(GLOB_RECURSE ${GTEST_HEADERS} ${PROJECT_SOURCE_DIR}/TimeTracker/gtest/include/*.h)
file(GLOB_RECURSE ${source_headers} ${PROJECT_SOURCE_DIR}/TimeTracker/*.h)
file(GLOB ${source_code} ${PROJECT_SOURCE_DIR}/TimeTracker/*.cpp)
set(SOURCES ${source_code} ${source_headers})

add_executable(FilesystemTest ${SOURCES} ${GTEST_HEADERS}
    ${PROJECT_SOURCE_DIR}/TimeTracker/test/FilesystemTest.cpp)
add_dependencies(FilesystemTest gtest)
target_link_libraries(FilesystemTest gtest)
add_test(FilesystemTest WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/TimeTracker/test)