cmake_minimum_required(VERSION 3.8)


file(GLOB_RECURSE ${GTEST_HEADERS} ${PROJECT_SOURCE_DIR}/TimeTracker/gtest/include/*.h)

add_executable(StreamTest ${SOURCES} ${GTEST_HEADERS}
    ${PROJECT_SOURCE_DIR}/TimeTracker/test/StreamTest.cpp)
target_include_directories(StreamTest PRIVATE ${INCLUDE_DIRECTORIES})
add_dependencies(StreamTest gtest)
target_link_libraries(StreamTest gtest)
add_test(NAME streamTest COMMAND StreamTest)

add_executable(TrackerTest ${SOURCES} ${GTEST_HEADERS}
    ${PROJECT_SOURCE_DIR}/TimeTracker/test/TrackerTest.cpp)
target_include_directories(TrackerTest PRIVATE ${INCLUDE_DIRECTORIES})
add_dependencies(TrackerTest gtest)
target_link_libraries(TrackerTest gtest)
add_test(NAME trackerTest COMMAND TrackerTest)


 add_executable(ParserTest ${SOURCES} ${GTEST_HEADERS}
    ${PROJECT_SOURCE_DIR}/TimeTracker/test/DateParser.cpp)
target_include_directories(ParserTest PRIVATE ${INCLUDE_DIRECTORIES})
 add_dependencies(ParserTest gtest)
 target_link_libraries(ParserTest gtest)
 add_test(NAME parserTest COMMAND ParserTest)