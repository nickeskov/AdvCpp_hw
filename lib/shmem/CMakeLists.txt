cmake_minimum_required(VERSION 3.10)
project(shmem)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(GCC_COVERAGE_COMPILE_FLAGS "--coverage -fprofile-arcs -ftest-coverage -fPIC")
set(GCC_COVERAGE_LINK_FLAGS "-lgcov --coverage")
set(GCC_SANITIZE_FLAGS "-fsanitize=address")


message("Build type for ${PROJECT_NAME}: ${CMAKE_BUILD_TYPE}")

if (CMAKE_BUILD_TYPE STREQUAL Debug)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g3")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -g3")
endif ()

if (CMAKE_BUILD_TYPE STREQUAL Release)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -march=native")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -march=native")
endif ()

option(ENABLE_COVERAGE "Enable code coverage compiler options to ${PROJECT_NAME} target(s)" OFF)

if (ENABLE_COVERAGE)
    message("Code coverage ENABLED for ${PROJECT_NAME}")

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}")
endif ()

option(ENABLE_SANITIZE "Enable sanitizers to ${PROJECT_NAME} target(s)" OFF)

if (ENABLE_SANITIZE)
    message("Sanitizers ENABLED for ${PROJECT_NAME}")

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_SANITIZE_FLAGS}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${GCC_SANITIZE_FLAGS}")
endif ()

option(ENABLE_CPPCHECK "Enable cppcheck checker as custom target ${PROJECT_NAME}_check" OFF)

if (ENABLE_CPPCHECK)
    message("CPPCHECK ENABLED for ${PROJECT_NAME}")

    # set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

    list(APPEND ${PROJECT_NAME}_CPPCHECK_ARGS
            --inline-suppr # only for this library
            --enable=warning,style,performance,portability
            --check-library
            --std=c++11
            --verbose
            --error-exitcode=1
            --language=c++
            -I ${PROJECT_SOURCE_DIR}/include
            -I{} # for xargs utility
            )

    add_custom_target(
            ${PROJECT_NAME}_check
            COMMENT "running cppcheck for ${PROJECT_NAME}"
            COMMAND
            find ${PROJECT_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR}/include -name *?.cpp -or -name *?.h
            | xargs cppcheck ${${PROJECT_NAME}_CPPCHECK_ARGS}
    )
endif ()

# ------------------------------------------------------------------------------

add_library(shmem STATIC
        src/errors.cpp
        src/concurrentsync/semaphore.cpp
        src/concurrentsync/mutex.cpp
        )

target_include_directories(shmem PUBLIC include)

find_package(Threads REQUIRED)

target_link_libraries(shmem PRIVATE ${CMAKE_THREAD_LIBS_INIT})

target_compile_options(shmem PRIVATE -Wall -Wextra -Wpedantic -Werror -pipe)
