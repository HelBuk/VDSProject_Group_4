project(VDSProject_verify CXX C)
cmake_minimum_required(VERSION 3.10)


add_executable(VDSProject_verify main_verify.cpp)
add_executable(VDSProject_verify_all main_verify_all.cpp)


include_directories({$CMAKE_SOURCE_DIR}/src/verify/)
link_directories({$CMAKE_SOURCE_DIR}/src/verify/)

target_link_libraries(VDSProject_verify Manager)
target_link_libraries(VDSProject_verify Benchmark)

# Check GCC version
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 10)
        target_link_libraries(VDSProject_verify_all stdc++fs)
    endif()
endif()
