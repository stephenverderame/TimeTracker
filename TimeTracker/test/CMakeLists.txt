cmake_minimum_required(VERSION 3.8)


file(GLOB_RECURSE ${GTEST_HEADERS} ${PROJECT_SOURCE_DIR}/TimeTracker/gtest/include/*.h)

add_executable(StreamTest ${SOURCES} ${GTEST_HEADERS}
    ${PROJECT_SOURCE_DIR}/TimeTracker/test/StreamTest.cpp)
add_dependencies(StreamTest gtest)
target_link_libraries(StreamTest gtest)
add_test(streamTest StreamTest WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/TimeTracker/test)