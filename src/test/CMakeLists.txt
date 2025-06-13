cmake_minimum_required(VERSION 3.10)
project(VDSProject_test CXX C)

add_executable(VDSProject_test manager_test.cpp)

target_link_libraries(VDSProject_test
        Manager
        gtest
        gtest_main
        pthread
)

# Apply coverage flags
enable_coverage(VDSProject_test)
