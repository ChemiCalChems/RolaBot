project(gestion)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules")

set (CMAKE_CXX_FLAGS "-std=c++14 -Werror -Wall -pthread")

set(SRC_FILES src/main.cpp)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable(gestion ${SRC_FILES})

find_package(Qt5Widgets REQUIRED)
find_package(MySQLpp REQUIRED)

include_directories(${Qt5Widgets_INCLUDE_DIRS})
include_directories(${MYSQLPP_INCLUDE_DIR})
include_directories(${MYSQL_INCLUDE_DIR})
  
# target_link_libraries(gestion Qt5::Widgets)
target_link_libraries(gestion ${MYSQLPP_LIBRARIES})

