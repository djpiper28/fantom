cmake_minimum_required(VERSION 3.12)

# set the project name
project(fantom-server VERSION 1.0.0 LANGUAGES CXX)
set(REPO_URL "https://github.com/djpiper28/fantom")

message(STATUS "Building ${CMAKE_PROJECT_NAME}")
message(STATUS "Git repo: ${REPO_URL}")
message(STATUS "Build type is ${CMAKE_BUILD_TYPE} for version ${CMAKE_PROJECT_VERSION}")

# Detect OS
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  message (STATUS "Operating system detected: Linux")
  set (LINUX TRUE)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  message (STATUS "Operating system detected: OSX")
  set (OSX TRUE)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  message (STATUS "Operating system detected: Windows")
  set (WINDOWS TRUE)
else ()
  message (ERROR "Operating system not recognised. Expected Windows, Linux or Darwin, but found ${CMAKE_SYSTEM_NAME}")
endif ()

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# compiler flags
set(COMPILER_FLAGS "-Wall -Wextra -Wpedantic")
set(CMAKE_C_FLAGS_DEBUG "-g -O0 ${COMPILER_FLAGS} -DDEBUG -Wno-unused-parameter")
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
        EXECUTABLE fantom-server-tests
        EXCLUDE "build/*" "tests/*" "src/main.c")

endif()

# src
set(MAIN
    src/main.cpp
    )
set(SRC_FILES
    src/config.c
    src/fantom_str.c
    src/security.c
    src/logger.c
    )

set(LIBS
    pthread
    crypto
    )

add_executable(fantom-server ${SRC_FILES} ${MAIN})
set_target_properties(fantom-server PROPERTIES LINKER_LANGUAGE CXX)

target_link_libraries(fantom-server ${LIBS})
target_include_directories(fantom-server PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR})

if (CMAKE_BUILD_TYPE STREQUAL "TEST")
    # tests, set the flags manually
    set(CMAKE_C_FLAGS_RELEASE ${CMAKE_C_FLAGS_DEBUG})
    set(CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_DEBUG})

    set(TEST_FILES
       ${SRC_FILES}
       tests/runner.cpp
       )
    add_executable(fantom-server-tests ${TEST_FILES})
    set_target_properties(fantom-server-tests PROPERTIES LINKER_LANGUAGE CXX)

    target_link_libraries(fantom-server-tests
        cppunit ${LIBS})
    target_include_directories(fantom-server-tests PUBLIC
        ${CMAKE_CURRENT_BINARY_DIR})

    enable_testing ()
    add_test(NAME fantom-server-tests COMMAND fantom-server-tests)
endif()
