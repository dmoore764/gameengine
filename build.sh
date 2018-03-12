DEBUG="true"
HEADER_SEARCH_PATHS="-I/Users/danielmoore/Github/gameengine/dependencies/include -I/Users/danielmoore/Github/gameengine/dependencies/lib"
FRAMEWORK_SEARCH_PATHS="-F/Users/danielmoore/Github/gameengine/dependencies/frameworks"

#-Wno-shift-negative-value -Wno-null-dereference -Wno-format-security -Wno-format  

BUILD_FILES="
	/Users/danielmoore/Github/gameengine/src/main.cpp
	/Users/danielmoore/Github/gameengine/src/game_math.cpp
	/Users/danielmoore/Github/gameengine/src/window.cpp
	/Users/danielmoore/Github/gameengine/src/opengl.cpp
	/Users/danielmoore/Github/gameengine/src/file_utilities.cpp
	/Users/danielmoore/Github/gameengine/src/memory_arena.cpp
	/Users/danielmoore/Github/gameengine/src/xml.cpp
	/Users/danielmoore/Github/gameengine/src/collada.cpp
	/Users/danielmoore/Github/gameengine/src/mesh.cpp
	/Users/danielmoore/Github/gameengine/src/object3d.cpp
	/Users/danielmoore/Github/gameengine/src/render.cpp
	/Users/danielmoore/Github/gameengine/src/game_world.cpp
	/Users/danielmoore/Github/gameengine/src/assets.cpp
	/Users/danielmoore/Github/gameengine/src/physics.cpp
	/Users/danielmoore/Github/gameengine/src/imgui_demo.cpp
	/Users/danielmoore/Github/gameengine/src/imgui_draw.cpp
	/Users/danielmoore/Github/gameengine/src/imgui.cpp
	/Users/danielmoore/Github/gameengine/src/imgui_impl_sdl_gl3.cpp
	/Users/danielmoore/Github/gameengine/src/hash.cpp
"

clang++ -g -glldb -O0 -std=c++11 -Wno-writable-strings $HEADER_SEARCH_PATHS $BUILD_FILES -o /Users/danielmoore/Github/gameengine/build/game $FRAMEWORK_SEARCH_PATHS -framework CoreServices -framework OpenGL -framework SDL2 -framework LinearMath -framework BulletDynamics -framework BulletCollision 
