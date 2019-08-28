project(svgBracketChecker)
cmake_minimum_required(VERSION 2.8)

#find_package(Threads REQUIRED)
#set(LIBS ${LIBS} ${CMAKE_THREAD_LIBS_INIT})

#find_package(Boost REQUIRED COMPONENTS filesystem system log log_setup)
#find_package(Boost REQUIRED COMPONENTS log log_setup thread system)
#include_directories("." ${GTEST_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})
#set(LIBS ${LIBS} ${Boost_LIBRARIES})
#add_definitions(-DBOOST_LOG_DYN_LINK=1)  # linking fails if this is not set

find_library(Sloppy_LIB NAME Sloppy PATHS /usr/local/lib /usr/lib /usr/local/lib64 /usr/lib64)
set(LIBS ${LIBS} ${Sloppy_LIB})
message("Using Sloppy lib at " ${Sloppy_LIB})

#find_library(SqliteOverlay_LIB NAME SqliteOverlay PATHS /usr/local/lib /usr/lib /usr/local/lib64 /usr/lib64)
#set(LIBS ${LIBS} ${SqliteOverlay_LIB})
#message("Using SqliteOverlay lib at " ${SqliteOverlay_LIB})

#find_library(Wrappers_LIB NAME SloppyWrappers PATHS /usr/local/lib /usr/lib /usr/local/lib64 /usr/lib64)
#set(LIBS ${LIBS} ${Wrappers_LIB})
#message("Using SloppyWrappers lib at " ${Wrappers_LIB})

#set(LIBS ${LIBS} sqlite3)


#
# The checker
#
set(PRG_SOURCES
  main.cpp
  ../SvgBracket.h
  ../SvgBracket.cpp
  )


add_executable(${PROJECT_NAME} ${PRG_SOURCES})
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)

set(BIN_VERSION_MAJOR 0)
set(BIN_VERSION_MINOR 1)
set(BIN_VERSION_PATCH 0)
set(BIN_VERSION_STRING ${BIN_VERSION_MAJOR}.${BIN_VERSION_MINOR}.${BIN_VERSION_PATCH})

target_link_libraries(${PROJECT_NAME} ${LIBS})

