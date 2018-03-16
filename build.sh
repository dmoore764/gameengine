DEBUG="true"
HEADER_SEARCH_PATHS="-I/Users/danielmoore/Github/gameengine/dependencies/include -I/Users/danielmoore/Github/gameengine/dependencies/lib"
FRAMEWORK_SEARCH_PATHS="-F/Users/danielmoore/Github/gameengine/dependencies/frameworks"

#-Wno-shift-negative-value -Wno-null-dereference -Wno-format-security -Wno-format  

BUILD_FILES="
	/Users/danielmoore/Github/gameengine/src/main.cpp
"

clang++ -g -glldb -O0 -std=c++11 -Wno-writable-strings $HEADER_SEARCH_PATHS $BUILD_FILES -o /Users/danielmoore/Github/gameengine/build/game $FRAMEWORK_SEARCH_PATHS -framework CoreServices -framework OpenGL -framework SDL2 -framework LinearMath -framework BulletDynamics -framework BulletCollision 
