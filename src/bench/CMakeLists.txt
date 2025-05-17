add_library(Benchmark
        BenchParser.cpp
        BenchmarkLib.cpp
        CircuitToBDD.cpp
        bench_grammar.hpp
        skip_parser.hpp)

#Boost
cmake_policy(SET CMP0167 OLD)  #suppress warning
find_package(Boost)

#Executable
add_executable(VDSProject_bench main_bench.cpp)
target_link_libraries(VDSProject_bench Manager)
target_link_libraries(VDSProject_bench Benchmark)
target_link_libraries(VDSProject_bench ${Boost_LIBRARIES})

include_directories(${CMAKE_SOURCE_DIR}/src/bench)
link_directories(${CMAKE_SOURCE_DIR}/src/bench/)

# Include Boost headers
include_directories(${Boost_INCLUDE_DIRS})



