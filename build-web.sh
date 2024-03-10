source "../emsdk/emsdk_env.sh"
emcc src/main.c src/*.c \
	-o builds/web/index.html \
	--shell-file shell_minimal.html \
	--preload-file res \
	-Os -Wall -DPLATFORM_WEB \
	-I. -I../raylib/src \
	-L. -L..../raylib/src \
	-s USE_GLFW=3 -s ASYNCIFY
