cmake_minimum_required(VERSION 2.8.4)
project(rogue)

set(CMAKE_C_FLAGS "-std=c99")
set(CMAKE_BUILD_TYPE Debug)

add_executable(client client.c utils.c linkedList.c socket.c packet.c map.c vector.c)
add_executable(server server.c utils.c linkedList.c socket.c packet.c map.c auth.c game.c mob.c vector.c)

add_executable(map_test map_test.c map.c vector.c)
add_executable(utils_test utils_test.c utils.c)
add_executable(game_test game_test.c game.c linkedList.c map.c mob.c packet.c socket.c vector.c)

find_package(Curses REQUIRED)
find_package(Threads)

target_link_libraries(client m ${CMAKE_THREAD_LIBS_INIT} ${CURSES_LIBRARY})
target_link_libraries(server m ${CMAKE_THREAD_LIBS_INIT})

target_link_libraries(map_test m)
target_link_libraries(game_test m)