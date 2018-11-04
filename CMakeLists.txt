project(crawler)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_STANDARD 11)
set(O2_FLAG "-O2")

# Point to our own cmake modules
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMakeModules)

# Add global definitions
add_definitions("-Wall")

#Add MPI
find_package(MPI REQUIRED)
include_directories(${MPI_INCLUDE_PATH})

add_executable(crawlerSEQ crawlerSequencial.cpp)
target_link_libraries(crawlerSEQ "-lcurl -lboost_regex")
set_target_properties(crawlerSEQ PROPERTIES COMPILE_FLAGS ${O2_FLAG} LINK_FLAGS ${O2_FLAG})

add_executable(crawlerPAR crawlerParalelo.cpp)
target_link_libraries(crawlerPAR "-lcurl -lboost_regex -lpthread")
set_target_properties(crawlerPAR PROPERTIES COMPILE_FLAGS ${O2_FLAG} LINK_FLAGS ${O2_FLAG})

add_executable(crawlerDIS crawlerDistribuido.cpp)
target_link_libraries(crawlerDIS ${MPI_LIBRARIES} "-lcurl -lboost_regex -lboost_mpi -lboost_serialization")
set_target_properties(crawlerDIS PROPERTIES COMPILE_FLAGS ${O2_FLAG} LINK_FLAGS ${O2_FLAG})