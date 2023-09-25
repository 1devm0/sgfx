# SGFX
- SDL Graphics Engine for making cross-platform games using OpenGL at light speed

## Tools Used
- SDL2 (all sub-packages like SDL_image, SDL_ttf, SDL_mixer)
- OpenGL
- Custom Utility Framework 
- MinGW (for Windows Compiler)
- Make (for compiling)
- Emscripten (for compiling for web)
- RenderDoc (for debugging)

## Platform Support
- Windows 10 (not tested on Windows 11)
- Web (using Emscripten)
- Linux (not tested but theoretically it should support it as well since there is not really any platform-specific code)
> Note for Linux users, I have not yet added a command like `make linux` so you may have to implement that on your own

## Compiling
- Clone the repository
- Go into the `sgfx` folder
- If you are on Windows, run: `tools\scripts\run.bat` in Command Prompt
- If you want to compile for the web, run `make web` in Command Prompt
- For the web, you will have to host the server and all the build files can be found in the build folder
> Note for Windows users, in order to properly compile the project examples or game, you will have to get all the SDL DLLs and library downloaded from the official SDL website or some package manager. 

## Rendering Method / How It Works (Oversimplified)
- The way the renderer works is inspired heavily by Voxel Rifts OpenGL Renderer but I made some changes as I found things like storing the projection matrix a bit restrictive for the user. The method it uses to render is basically batch rendering and any time a primitive (2d or 3d shape) is added to the renderer, it simply sends that data to the dynamically updating Vertex Buffer using glBufferSubData. I would appreciate if you guys have any feedback!

## Features
- Quite simple, and mainly relies on batch rendering to render most of the stuff
- Some examples:
    - [x] 2D Rendering, Batch Rendering, Textures (`2d_textures.c`)
    - [x] Textured Cubes, 3D Rendering in single example (`3d_textured.c`)
    - [x] Pyramid Model Loading (`3d_model.c`)
    - [x] [A Small 2D Game](https://github.com/1dev-mha/Dodgy-3quares)
- The renderer supports web compilation for examples such as the cubes and textured squares. Some of the examples such as the OBJ lodaer and the game don't work however due to various memory issues. 


## Heavily Used Resources  
- [OpenGL Renderer Tutorial by VoxelRifts](https://www.youtube.com/watch?v=NPnQF4yABwg&t=94s)
- [learnopengl.com](https://learnopengl.com/)
- [OpenGL Emscripten Example](https://gist.github.com/mortennobel/0e9e90c9bbc61cc99d5c3e9c038d8115)
- [Khronos OpenGL Documentation](https://registry.khronos.org/OpenGL-Refpages/gl4/html/)

