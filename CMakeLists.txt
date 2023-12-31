cmake_minimum_required(VERSION 3.23.0)
project(ccp VERSION 0.0.1)
if(MSVC)
    add_compile_options("/utf-8")
endif()
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
include(CTest)
set(CMAKE_CXX_STANDARD 20)


add_executable(ccp main.cpp)
find_package(spdlog CONFIG REQUIRED)
target_link_libraries(ccp  PRIVATE spdlog::spdlog)
find_package(unofficial-concurrentqueue CONFIG REQUIRED)
target_link_libraries(ccp PRIVATE unofficial::concurrentqueue::concurrentqueue)
find_package(cy CONFIG REQUIRED)
target_link_libraries(ccp PRIVATE cy::cy)
find_package(win-manifests CONFIG REQUIRED)
target_link_libraries(ccp PRIVATE win-manifests::win-manifests)
find_package(CLI11 CONFIG REQUIRED)
target_link_libraries(ccp PUBLIC CLI11::CLI11)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

include(GNUInstallDirs)
