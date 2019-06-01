cmake_minimum_required (VERSION 2.6)

set ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wunreachable-code -Wpedantic -Wformat=2 -std=c99 -O3 -march=native -D_GNU_SOURCE -fshort-enums -ftrapv -Wfloat-equal -Wundef -Wshadow -Wpointer-arith -Wcast-align -Wstrict-prototypes -Wno-unused-parameter -Wno-missing-field-initializers -D_DEFAULT_SOURCE" )
set ( CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g3" )

# -fno-omit-frame-pointer -fsanitize=address

#RELEASE BUILD: cmake -DCMAKE_BUILD_TYPE=Release ..
#  DEBUG BUILD: cmake -DCMAKE_BUILD_TYPE=Debug ..

aux_source_directory(game game_srcs)

set(sim_srcs
	${game_srcs}
	util.c
	string_util.c
	rand.c
	card.c
	board.c
	spot.c
	move.c
	results.c
	game.c
	sim.c
	main.c
)

add_executable(sim ${sim_srcs})
target_link_libraries(sim "-lm -lpthread")
