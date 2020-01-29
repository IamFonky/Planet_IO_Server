cmake_minimum_required(VERSION 3.10)
project(planetIO)

set(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_FLAGS -pthread)

add_executable(planetIO main.cpp  stellarPrimitives.cpp stellarEngine.cpp Websocket/dataServer.cpp Websocket/eventServer.cpp Websocket/serverHelpers.cpp)

# see https://cmake.org/cmake/help/latest/module/FindBoost.html
find_package(Boost REQUIRED COMPONENTS)

include_directories(${Boost_INCLUDE_DIR})
include_directories(rapidjson)
# Note: a target should be already defined using 'add_executable' or 'add_library'
target_link_libraries(planetIO ${Boost_LIBRARIES})
