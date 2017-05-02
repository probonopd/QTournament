project(QTournament_Tests)
cmake_minimum_required(VERSION 3.1)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmakeModules/")
find_package(SQLite3 REQUIRED)
include_directories(${SQLITE3_INCLUDE_DIRS})
set(LIBS ${LIBS} ${SQLITE3_LIBRARIES})

find_package(Boost REQUIRED COMPONENTS filesystem system log log_setup)
include_directories(${Boost_INCLUDE_DIRS})
set(LIBS ${LIBS} ${Boost_LIBRARIES})
add_definitions(-DBOOST_LOG_DYN_LINK=1)  # linking fails if this is not set

#
# Qt
#
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
find_package(Qt5Core REQUIRED)


#
# My lib of sloppyly implemented helper functions
#
find_library(Sloppy_LIB NAME Sloppy PATHS /usr/local/lib /usr/lib /usr/local/lib64 /usr/lib64)
set(LIBS ${LIBS} ${Sloppy_LIB})
message("Using Sloppy lib at " ${Sloppy_LIB})

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

set(LIB_SOURCES
    ../SwissLadderGenerator.cpp
)

set(UNIT_TESTS
    tstSwissLadderGenerator.cpp
    #BasicTestClass.cpp
    unitTestMain.cpp
)

add_executable(${PROJECT_NAME} ${LIB_SOURCES} ${UNIT_TESTS})
target_link_libraries(${PROJECT_NAME} ${GTEST_BOTH_LIBRARIES} ${LIBS} Qt5::Core)
target_compile_options(${PROJECT_NAME} PRIVATE "-Wall")
target_compile_options(${PROJECT_NAME} PRIVATE "-Wextra")
#target_compile_options(${PROJECT_NAME} PRIVATE "-Weffc++")

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 14)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)

