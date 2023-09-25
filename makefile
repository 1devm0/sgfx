winflags = -lpsapi -Lbuild/exe/lib -lSDL2main -lSDL2 -lSDL2_ttf -lSDL2_mixer -lSDL2_image 
# unnecessary apparently libs = -lmingw32 -lpsapi -Lbuild/exe/lib -lSDL2main -lSDL2 -lSDL2_ttf -lSDL2_mixer -lSDL2_image -lm -lgdi32 -luser32 -lkernel32 -lopengl32 
macflags = -framework Cocoa -framework IOKit -framework CoreFoundation -framework OpenGL -framework CoreVideo -lSDL2main -lSDL2 -lSDL2_ttf -lSDL2_mixer -lSDL2_image 
# linux -lGL -lX11 -lpthread -lXrandr -lXi -ldl
linuxflags = -lm -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer
emcc2flags = -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS="['png']" -s USE_SDL_TTF=2 -s USE_SDL_MIXER=2 -s ALLOW_MEMORY_GROWTH=1 -s WASM=1
emccflags = -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS="['png']" -s USE_SDL_TTF=2 -s USE_SDL_MIXER=2 -s ALLOW_MEMORY_GROWTH=1 -s FULL_ES3=1 -s USE_WEBGL2=1 -s MIN_WEBGL_VERSION=2 -s WASM=1 --preload-file build/exe/res

dbg = -O0 -g -DDBG
fast = -std=c11 -O3 -finline-functions -funroll-loops -fprofile-generate
compile_lvl = $(dbg)
includes = -Isrc/engine/deps
flags = $(includes) $(compile_lvl)

DEP_FILES = src/engine/engine.c src/engine/deps/utils/utils.c src/engine/deps/sdl_gfx/SDL2_gfxPrimitives.c src/engine/deps/sdl_gfx/SDL2_framerate.c src/engine/deps/sdl_gfx/SDL2_imageFilter.c src/engine/deps/sdl_gfx/SDL2_rotozoom.c
MAIN = src/main.c src/examples/3d_model.c 



ASSET_FILES = 0

win:
	gcc $(MAIN) $(DEP_FILES) $(flags) $(winflags) -o build/exe/game.exe

winmain:
	gcc $(MAIN) build/obj/deps.a $(flags) $(winflags) -o build/exe/game.exe

windeps:
	gcc -c ${flags} src/engine/engine.c -o build/obj/engine.obj
	gcc -c ${flags} src/engine/deps/utils/utils.c -o build/obj/utils.obj
	gcc -c ${flags} src/engine/deps/sdl_gfx/SDL2_gfxPrimitives.c -o build/obj/gfxprim.obj
	gcc -c ${flags} src/engine/deps/sdl_gfx/SDL2_framerate.c -o build/obj/frames.obj
	gcc -c ${flags} src/engine/deps/sdl_gfx/SDL2_imageFilter.c -o build/obj/imgfilter.obj
	gcc -c ${flags} src/engine/deps/sdl_gfx/SDL2_rotozoom.c -o build/obj/roto.obj
	ar -rc build/obj/deps.a build/obj/engine.obj build/obj/utils.obj build/obj/gfxprim.obj build/obj/frames.obj build/obj/imgfilter.obj build/obj/roto.obj	

web: 	
	emcc $(MAIN) $(DEP_FILES) $(includes) $(emccflags) -o build/web/index.html

webmain:
	emcc $(MAIN) build/obj/deps.a $(includes) $(flags) $(emccflags) -o build/web/index.js

webdeps:
	emcc -c ${flags} $(includes) $(emccflags) src/engine/engine.c -o build/obj/engine.obj
	emcc -c ${flags} $(includes) $(emccflags) src/engine/deps/utils/utils.c -o build/obj/utils.obj
	emcc -c ${flags} $(includes) $(emccflags) src/engine/deps/sdl_gfx/SDL2_gfxPrimitives.c -o build/obj/gfxprim.obj
	emcc -c ${flags} $(includes) $(emccflags) src/engine/deps/sdl_gfx/SDL2_framerate.c -o build/obj/frames.obj
	emcc -c ${flags} $(includes) $(emccflags) src/engine/deps/sdl_gfx/SDL2_imageFilter.c -o build/obj/imgfilter.obj
	emcc -c ${flags} $(includes) $(emccflags) src/engine/deps/sdl_gfx/SDL2_rotozoom.c -o build/obj/roto.obj
	emar -rc build/obj/deps.a build/obj/engine.obj build/obj/utils.obj build/obj/gfxprim.obj build/obj/frames.obj build/obj/imgfilter.obj build/obj/roto.obj	

clean:
	rm main.gcda

