cmake_minimum_required(VERSION 3.12)

# set the project name
project(fantom VERSION 1.0.0)
set(REPO_URL "https://github.com/djpiper28/fantom")

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message(STATUS "Building ${CMAKE_PROJECT_NAME}")
message(STATUS "Git repo: ${REPO_URL}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Version: ${CMAKE_PROJECT_VERSION}")
message(STATUS "Build OS: ${CMAKE_SYSTEM_NAME}")

# Get the branch name
execute_process(
  COMMAND git rev-parse --abbrev-ref HEAD
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_BRANCH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Get the latest abbreviated commit hash of the working branch
execute_process(
  COMMAND git log -1 --format=%h
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_COMMIT_HASH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Version info
add_definitions("-DGIT_COMMIT_HASH=\"${GIT_COMMIT_HASH}\"")
add_definitions("-DGIT_BRANCH=\"${GIT_BRANCH}\"")
add_definitions("-DVERSION=\"${CMAKE_PROJECT_VERSION} (${GIT_COMMIT_HASH}@${GIT_BRANCH})\"")
add_definitions("-DOS=\"${CMAKE_SYSTEM_NAME}\"")
add_definitions("-DREPO_URL=\"${REPO_URL}\"")
add_definitions("-DBUFFER_LENGTH=4096")

# __FILENAME__
string(LENGTH "${CMAKE_SOURCE_DIR}/" SOURCE_PATH_SIZE)
add_definitions("-DSOURCE_PATH_SIZE=${SOURCE_PATH_SIZE}")
add_definitions("-D__FILENAME__=(__FILE__ + SOURCE_PATH_SIZE)")

# Mongoose settings
add_definitions(-DMG_ENABLE_IPV6=1)
add_definitions(-DMG_ENABLE_LINES=1)
# Enable file serving for the static build of the web app
add_definitions(-DMG_ENABLE_DIRECTORY_LISTING=1)
add_definitions(-DMG_ENABLE_FS=1)

# compiler flags
set(COMPILER_FLAGS "-Wall -Wextra -Wpedantic")
set(CMAKE_C_FLAGS_DEBUG "-g -O0 ${COMPILER_FLAGS} -DDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set(CMAKE_C_FLAGS_RELEASE "-O2 ${COMPILER_FLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/scripts/cmake)

if (CMAKE_BUILD_TYPE STREQUAL "TEST")
    include(CodeCoverage)

    # Setup coverage (using gcovr so needs gcc/g++)
    set(CMAKE_C_COMPILER gcc)
    set(CMAKE_CXX_COMPILER g++)

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_DEBUG} -DTEST")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG} -DTEST")

    append_coverage_compiler_flags()
    setup_target_for_coverage_gcovr_xml(NAME coverage
        EXECUTABLE fantom-tests
        EXCLUDE "build/*" "tests/*")
endif()

# src
set(MAIN_FILE
    src/main.c
    )
set(SRC_FILES
		src/banner.c
    src/config.c
    src/fantom_str.c
    src/security.c
    src/logger.c
    src/server.c
    src/db.c
    src/mongoose.c
    )

set(LIBS
    pthread    # threads
    crypto     # password hashing
    mbedtls    # v
    mbedcrypto # mongoose crypto
    mbedx509   # ^
    jansson    # json
    sqlite3    # database
    )

add_executable(fantom ${SRC_FILES} ${MAIN_FILE})
target_link_libraries(fantom ${LIBS})
target_include_directories(fantom PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR})


# Copy static content (images, favicon, etc.)
file(GLOB STATIC_IMG
  "assets/**"
)
file(COPY ${STATIC_IMG} DESTINATION "${CMAKE_BINARY_DIR}/assets/")

# Make tests when needed
if (CMAKE_BUILD_TYPE STREQUAL "TEST")
    # tests, set the flags manually
    set(CMAKE_C_FLAGS_RELEASE ${CMAKE_C_FLAGS_DEBUG})
    set(CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_DEBUG})

    set (TEST_FILES
        ${SRC_FILES}
        tests/main.c
        tests/testing.c
        tests/test_db.c
        )
    add_executable(fantom-tests ${TEST_FILES})
    target_link_libraries(fantom-tests
        ${LIBS})
    target_include_directories(fantom-tests PUBLIC
        ${CMAKE_CURRENT_BINARY_DIR})

    enable_testing ()
    add_test(NAME fantom-tests COMMAND fantom-tests)
endif()

