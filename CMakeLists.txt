cmake_minimum_required(VERSION 3.0.0)
project(windows_mix VERSION 0.1.0 LANGUAGES C CXX)

include(CTest)
enable_testing()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

add_executable(main watch_clipboard.cpp)
find_package(leveldb CONFIG REQUIRED)
target_link_libraries(main PRIVATE leveldb::leveldb)

find_package(unofficial-concurrentqueue CONFIG REQUIRED)
target_link_libraries(main PRIVATE unofficial::concurrentqueue::concurrentqueue)
