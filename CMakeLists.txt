cmake_minimum_required (VERSION 2.8.9)
project (god-damn-debugger)
add_executable(gdd ./src/main.c)
target_include_directories(gdd PRIVATE ./includes)
target_sources(gdd 
	PRIVATE ./src/breakpoint.c ./src/debug.c ./src/utils.c
)
