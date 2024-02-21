cmake_minimum_required(VERSION 3.22)
include(FetchContent)

project(tynbox
	VERSION 0.0.0
	DESCRIPTION "tynbox"
	HOMEPAGE_URL "https://github.com/tynrare/tynbox"
	LANGUAGES C
)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

find_package(raylib 3.0 QUIET) # Let CMake search for a raylib-config.cmake
find_package(box2d QUIET) 

if (NOT box2d_FOUND) # If there's none, fetch and build raylib
	FetchContent_Declare(
		box2d
		GIT_REPOSITORY https://github.com/erincatto/box2c.git
		GIT_TAG main
		GIT_SHALLOW TRUE
		GIT_PROGRESS TRUE
	)
	FetchContent_MakeAvailable(box2d)
endif()

if (NOT raylib_FOUND) # If there's none, fetch and build raylib
	FetchContent_Declare(
		raylib
		GIT_REPOSITORY https://github.com/raysan5/raylib.git
		GIT_TAG master
		GIT_SHALLOW TRUE
		GIT_PROGRESS TRUE
	)
	FetchContent_MakeAvailable(raylib)
endif()

file(COPY "res" DESTINATION ".")

set(SOURCE_FILES 
	main.c 
	"src/game_d231012.c" 
	"src/tynroar_rcmd.c" 
	"src/app.c" 
	"src/game_tynmaze.c" 
	"src/tyncommons.c" 
	"src/test_shader_0.c" 
	"src/test_networksim_0.c" 
	"src/test_render_0.c" 
	"src/tynstage_template.c" 
	#"src/test_physics_0.c" 
	"src/shader_sdf2d.c" 
	"src/test_collisions_0.c")

add_executable(tynbox ${SOURCE_FILES})
target_link_libraries(tynbox PRIVATE box2d raylib)
#source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX "box2d" FILES "${box2d_SOURCE_DIR}/src")
target_include_directories(tynbox PUBLIC "${box2d_SOURCE_DIR}/src")
