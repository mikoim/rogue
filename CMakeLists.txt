cmake_minimum_required(VERSION 2.8.4)
project(rogue)

set(CMAKE_C_FLAGS "-std=c99")
set(CMAKE_BUILD_TYPE Debug)

add_executable(client client.c utils.c linkedList.c socket.c packet.c map.c vector.c)
add_executable(server server.c utils.c linkedList.c socket.c packet.c map.c auth.c game.c mob.c vector.c)

find_package(Curses REQUIRED)
find_package(Threads)

target_link_libraries(client m ${CMAKE_THREAD_LIBS_INIT} ${CURSES_LIBRARY})
target_link_libraries(server m ${CMAKE_THREAD_LIBS_INIT})