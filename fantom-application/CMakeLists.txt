cmake_minimum_required(VERSION 3.5)

project(
  fantom-application
  VERSION 1.0.0
  LANGUAGES CXX)
# set the project name
set(REPO_URL "https://github.com/djpiper28/fantom")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools)

# compiler flags
set(COMPILER_FLAGS "-Wall -Wextra -Wpedantic")
set(CMAKE_C_FLAGS_DEBUG "-g -O0 ${COMPILER_FLAGS} -DDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set(CMAKE_C_FLAGS_RELEASE "-O2 ${COMPILER_FLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})

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
  OUTPUT_STRIP_TRAILING_WHITESPACE)

# Get the latest abbreviated commit hash of the working branch
execute_process(
  COMMAND git log -1 --format=%h
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_COMMIT_HASH
  OUTPUT_STRIP_TRAILING_WHITESPACE)

# Version info
add_definitions("-DGIT_COMMIT_HASH=\"${GIT_COMMIT_HASH}\"")
add_definitions("-DGIT_BRANCH=\"${GIT_BRANCH}\"")
add_definitions(
  "-DVERSION=\"${CMAKE_PROJECT_VERSION} (${GIT_COMMIT_HASH}@${GIT_BRANCH})\"")
add_definitions("-DOS=\"${CMAKE_SYSTEM_NAME}\"")
add_definitions("-DREPO_URL=\"${REPO_URL}\"")
add_definitions("-DBUFFER_LENGTH=4096")

# __FILENAME__
string(LENGTH "${CMAKE_SOURCE_DIR}/" SOURCE_PATH_SIZE)
add_definitions("-DSOURCE_PATH_SIZE=${SOURCE_PATH_SIZE}")
add_definitions("-D__FILENAME__=(__FILE__ + SOURCE_PATH_SIZE)")

set(TS_FILES fantom-application_en_GB.ts)

set(PROJECT_SOURCES main.cpp mainwindow.cpp mainwindow.h mainwindow.ui
                    ${TS_FILES})

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/scripts/cmake)

if(CMAKE_BUILD_TYPE STREQUAL "TEST")
  add_definitions(-DTEST=1)
  include(CodeCoverage)

  # Setup coverage (using gcovr so needs gcc/g++) set(CMAKE_C_COMPILER
  # /usr/bin/gcc) set(CMAKE_CXX_COMPILER /usr/bin/g++)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_DEBUG} -DTEST")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG} -DTEST")

  append_coverage_compiler_flags()
  setup_target_for_coverage_gcovr_xml(
    NAME
    coverage
    EXECUTABLE
    fantom-tests
    EXCLUDE
    "build/*"
    "tests/*")
endif()

set(LIBS
    pthread # threads
    crypto # password hashing
    curl m)

# Build JWT library
add_subdirectory(lib/l8w8jwt/)

# Copy test scripts
file(COPY mem_tests.sh DESTINATION ${CMAKE_BINARY_DIR})
file(COPY mem_tests.py DESTINATION ${CMAKE_BINARY_DIR})

# Make tests when needed
if(CMAKE_BUILD_TYPE STREQUAL "TEST")
  # tests, set the flags manually
  set(CMAKE_C_FLAGS_RELEASE ${CMAKE_C_FLAGS_DEBUG})
  set(CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_DEBUG})

  set(TEST_FILES ${SRC_FILES} tests/main.c tests/testing.c tests/curl_utils.c
                 tests/test_security.c tests/test_utils.c)
  add_executable(fantom-tests ${TEST_FILES})
  target_link_libraries(
    fantom-tests
    PUBLIC ${LIBS}
    PRIVATE l8w8jwt)
  target_include_directories(fantom-tests PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

  enable_testing()
  add_test(NAME init_tests COMMAND bash -c "rm -f *.json")
  add_test(NAME fantom-tests COMMAND fantom-tests)
  add_test(NAME memory-tests COMMAND bash mem_tests.sh)

  message(STATUS "TEST build setup")
else()
  set(CMAKE_AUTOUIC ON)
  set(CMAKE_AUTOMOC ON)
  set(CMAKE_AUTORCC ON)
  if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(fantom-application MANUAL_FINALIZATION ${PROJECT_SOURCES})
    # Define target properties for Android with Qt 6 as: set_property(TARGET
    # fantom-application APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
    # ${CMAKE_CURRENT_SOURCE_DIR}/android) For more information, see
    # https://doc.qt.io/qt-6/qt-add-executable.html#target-creation

    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
  else()
    if(ANDROID)
      add_library(fantom-application SHARED ${PROJECT_SOURCES})
      # Define properties for Android with Qt 5 after find_package() calls as:
      # set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
      add_executable(fantom-application ${PROJECT_SOURCES})
    endif()

    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
  endif()

  target_link_libraries(fantom-application
                        PRIVATE Qt${QT_VERSION_MAJOR}::Widgets ${LIBS})

  set_target_properties(
    fantom-application
    PROPERTIES MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
               MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
               MACOSX_BUNDLE_SHORT_VERSION_STRING
               ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
               MACOSX_BUNDLE TRUE
               WIN32_EXECUTABLE TRUE)

  if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(fantom-application)
  endif()
endif()
