cmake_minimum_required(VERSION 3.10)
project(VDSProject_test_reachability CXX C)

add_executable(VDSProject_reachability
        main_test.cpp
        Reachability.cpp
        Reachability.h
        Tests.h
        ReachabilityInterface.h
)

target_link_libraries(VDSProject_reachability
        Manager
        gtest
        gtest_main
        pthread
)

# Apply coverage flags cleanly
enable_coverage(VDSProject_reachability)
