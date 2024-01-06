cmake_minimum_required(VERSION 3.11) # FetchContent is available in 3.11+
project(unbrumed_c LANGUAGES C CXX)

# Set this to the minimal version you want to support
find_package(raylib 3.0 QUIET) # Let CMake search for a raylib-config.cmake


# You could change the QUIET above to REQUIRED and remove this if() clause
# This part downloads raylib and builds it if it's not installed on your system
if (NOT raylib_FOUND) # If there's none, fetch and build raylib
  include(FetchContent)
  FetchContent_Declare(
    raylib
    URL https://github.com/raysan5/raylib/archive/master.tar.gz
  )

  FetchContent_GetProperties(raylib)
  if (NOT raylib_POPULATED) # Have we downloaded raylib yet?
    set(FETCHCONTENT_QUIET NO)
    FetchContent_Populate(raylib)

    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples

    # build raylib
    add_subdirectory(${raylib_SOURCE_DIR} ${raylib_BINARY_DIR})

  endif()

endif()

# This is the main part:

file(COPY "res" DESTINATION ".")

include_directories(external external/box2c/include)
add_subdirectory(external/box2c)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED YES)
set(CMAKE_C_EXTENSIONS YES)
set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

set(SOURCE_FILES main.c "src/game_d231012.c"  "src/tynroar_rcmd.c" "src/app.c" "src/game_tynmaze.c")
add_executable(${PROJECT_NAME} ${SOURCE_FILES} "src/tyncommons.c" "src/test_shader_0.c" "src/test_networksim_0.c" "src/test_render_0.c" "src/tynstage_template.c" "src/test_physics_0.c" "src/shader_sdf2d.c")

#set(raylib_VERBOSE 1)
target_link_libraries(${PROJECT_NAME} raylib box2d)

IF (WIN32)
    #target_link_libraries(${PROJECT_NAME} msvcrt)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /experimental:c11atomics")
  # set stuff for windows
ELSE()
  # set stuff for other systems
    target_link_libraries(${PROJECT_NAME} m)
ENDIF()

# Checks if OSX and links appropriate frameworks (Only required on MacOS)
if (APPLE)
    target_link_libraries(${PROJECT_NAME} "-framework IOKit")
    target_link_libraries(${PROJECT_NAME} "-framework Cocoa")
    target_link_libraries(${PROJECT_NAME} "-framework OpenGL")
endif()

# That's it! You should have an example executable that you can run. Have fun!
