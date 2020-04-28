project(boost_log_wrapper)
cmake_minimum_required(VERSION 3.0)

set(CMAKE_CXX_STANDARD 17)
find_package(Boost COMPONENTS log REQUIRED)

message(STATUS ${Boost_LIBRARIES})

add_executable(boost_log_wrapper
        log.hpp
        test.cpp
)

target_compile_definitions(boost_log_wrapper PRIVATE BOOST_LOG_DYN_LINK)
target_link_libraries(boost_log_wrapper PUBLIC ${Boost_LIBRARIES})