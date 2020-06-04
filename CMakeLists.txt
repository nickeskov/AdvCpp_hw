cmake_minimum_required(VERSION 3.10)
project(hw)

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
            --enable=warning,style,performance,portability,unusedFunction
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

option(HW_ENABLE_HW1 "Enable hw1 tests and build all additional hw1 libraries for ${PROJECT_NAME}" OFF)
option(HW_ENABLE_HW2 "Enable hw2 tests and build all additional hw2 libraries for ${PROJECT_NAME}" OFF)
option(HW_ENABLE_HW3 "Enable hw3 tests and build all additional hw3 libraries for ${PROJECT_NAME}" OFF)
option(HW_ENABLE_HW4 "Enable hw4 tests and build all additional hw4 libraries for ${PROJECT_NAME}" OFF)
option(HW_ENABLE_HW5 "Enable hw5 tests and build all additional hw5 libraries for ${PROJECT_NAME}" OFF)
option(HW_ENABLE_HW6 "Enable hw5 tests and build all additional hw6 libraries for ${PROJECT_NAME}" OFF)
option(HW_ENABLE_ALL "Enable all hw tests and build all libraries for ${PROJECT_NAME}" OFF)

add_executable(hw
        src/main.cpp
        src/utils.cpp)

target_compile_options(hw PRIVATE -Wall -Wextra -Wpedantic -Werror -pipe)

target_include_directories(hw PRIVATE include)

add_subdirectory(lib/unixprimwrap)
target_link_libraries(hw unixprimwrap)

add_subdirectory(lib/raiiguards)

if (HW_ENABLE_ALL OR HW_ENABLE_HW1)
    add_subdirectory(lib/linuxproc)
    target_link_libraries(hw linuxproc)
    target_compile_definitions(hw PRIVATE HW_ENABLE_HW1)
endif ()

if (HW_ENABLE_ALL OR HW_ENABLE_HW2)
    add_subdirectory(lib/trivilog)
    target_link_libraries(hw trivilog)
    target_compile_definitions(hw PRIVATE HW_ENABLE_HW2)
endif ()

if (HW_ENABLE_ALL OR HW_ENABLE_HW3 OR HW_ENABLE_HW4)
    add_subdirectory(lib/tcpcon)
    target_link_libraries(hw tcpcon)

    if (HW_ENABLE_ALL OR HW_ENABLE_HW3)
        target_compile_definitions(hw PRIVATE HW_ENABLE_HW3)
    endif ()

    if (HW_ENABLE_ALL OR HW_ENABLE_HW4)
        target_compile_definitions(hw PRIVATE HW_ENABLE_HW4)
    endif ()
endif ()

if (HW_ENABLE_ALL OR HW_ENABLE_HW5)
    add_subdirectory(lib/shmem)
    target_link_libraries(hw shmem)
    target_compile_definitions(hw PRIVATE HW_ENABLE_HW5)
endif ()

if (HW_ENABLE_ALL OR HW_ENABLE_HW6)
    add_subdirectory(lib/coroutine)
    add_subdirectory(lib/tinyhttp)
    target_link_libraries(hw tinyhttp)
    target_compile_definitions(hw PRIVATE HW_ENABLE_HW6)
endif ()
