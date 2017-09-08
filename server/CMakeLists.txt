project(TournamentServer)
cmake_minimum_required(VERSION 3.2)

set(BUILD_TESTS 0)

find_package(Threads REQUIRED)
set(LIBS ${LIBS} ${CMAKE_THREAD_LIBS_INIT})

find_package(Boost REQUIRED COMPONENTS log log_setup thread date_time filesystem system)
include_directories(${Boost_INCLUDE_DIRS})
set(LIBS ${LIBS} ${Boost_LIBRARIES})
add_definitions(-DBOOST_LOG_DYN_LINK=1)  # linking fails if this is not set

#
# My lib of sloppyly implemented helper functions
#
find_library(Sloppy_LIB NAME Sloppy PATHS /usr/local/lib /usr/lib /usr/local/lib64 /usr/lib64)
set(LIBS ${LIBS} ${Sloppy_LIB})
message("Using Sloppy lib at " ${Sloppy_LIB})

#
# The VMime library
#
find_library(VMIME_LIB NAME vmime PATHS /usr/lib /usr/local/lib)
set(LIBS ${LIBS} ${VMIME_LIB})
message("The vmime lib is: " ${VMIME_LIB})

#
# My Sqlite helper
#
find_library(DbOverlay_LIB NAME SqliteOverlay PATHS /usr/local/lib /usr/lib /usr/local/lib64 /usr/lib64)
set(LIBS ${LIBS} ${DbOverlay_LIB})
message("Using SqliteOverlay lib at " ${DbOverlay_LIB})

#
# The wrapper around the Mongoose server
#
find_library(Mongoose_LIB NAME MongooseWrapper PATHS /usr/local/lib /usr/lib /usr/local/lib64 /usr/lib64)
set(LIBS ${LIBS} ${Mongoose_LIB})
message("Using Mongoose lib at " ${Mongoose_LIB})

set(BACKEND_SRC
    serverMain.cpp
    TournamentServer.cpp
    ServerConfig.cpp
    #SMTP_Mailer.cpp
)

add_executable(${PROJECT_NAME} ${BACKEND_SRC})
target_compile_options(${PROJECT_NAME} PRIVATE "-Wall")
target_compile_options(${PROJECT_NAME} PRIVATE "-Wextra")
target_link_libraries(${PROJECT_NAME} ${LIBS})
#target_compile_options(${PROJECT_NAME} PRIVATE "-Weffc++")
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 14)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)


#
# Unit tests
#
#set(LIB_SOURCES_TST
#    tests/unitTestMain.cpp
#    tests/tstManager.cpp
#    tests/tstHttpServer.cpp
#)

find_package(GTest)
message("Google Test found: ${GTEST_FOUND}")

if (GTEST_FOUND AND BUILD_TESTS)
    include_directories(${GTEST_INCLUDE_DIRS})

    #add_executable(${PROJECT_NAME}_Tests ${LIB_SOURCES} ${LIB_SOURCES_TST})
    add_executable(${PROJECT_NAME}_Tests ${LIB_SOURCES_TST})
    set_property(TARGET ${PROJECT_NAME}_Tests PROPERTY CXX_STANDARD 14)
    set_property(TARGET ${PROJECT_NAME}_Tests PROPERTY CXX_STANDARD_REQUIRED ON)

    set_target_properties(${PROJECT_NAME}_Tests PROPERTIES
      IMPORTED_LOCATION "${CMAKE_BINARY_DIR}/lib${PROJECT_NAME}.so"
    )

    set(LIBS ${LIBS} ${PROJECT_NAME})

    message("Using these libs: ${LIBS}")
    message("Writing unit test binary to: ${CMAKE_BINARY_DIR}")

    target_link_libraries(${PROJECT_NAME}_Tests ${LIBS} ${GTEST_BOTH_LIBRARIES})
endif (GTEST_FOUND AND BUILD_TESTS)

