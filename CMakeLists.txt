cmake_minimum_required(VERSION 3.0.0)
project(clipboards VERSION 0.1.0 LANGUAGES C CXX)

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

find_package(dirsystem CONFIG REQUIRED)
target_link_libraries(main PRIVATE dirsystem::dirsystem)


# set cpack destination
set(CPACK_PACKAGE_DIRECTORY "${CMAKE_BINARY_DIR}/package")
# set install dir name
set(CPACK_PACKAGE_INSTALL_DIRECTORY "${PROJECT_NAME}")



# set(CPACK_GENERATOR "ZIP")
include(CPack)

# install(TARGETS version_info RUNTIME)
# install(TARGETS print RUNTIME)

install(TARGETS main  DESTINATION bin)
file(GLOB DLL_FILES "${CMAKE_BINARY_DIR}/*.dll")
file(GLOB IMPLIB_FILES "${CMAKE_BINARY_DIR}/*.dll.a")

install(FILES ${DLL_FILES} ${IMPLIB_FILES}
        DESTINATION bin)