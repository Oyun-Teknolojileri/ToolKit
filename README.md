
# ToolKit

ToolKit is a 3d editor & interactive application development platform. It allows users to create 3d scenes and bring in the interactivity via C++ plug-ins.

## Editor High Lights

- 3d editor is capable of importing various models via drag & drop from desktop.
- Multiple perspective view ports and an orthographic view port.
- Create materials, manipulate and assign materials to objects.
- Hierarchical transformations (Parent – child relation for objects).
- Load & Save scenes.
- All transform manipulations (Translate – Rotate – Scale) in 3 spaces (Parent – Local – World).
- Asset Browser. Allows multiple Asset browser in the editor. (Such as Mesh & Material Panels can be open on the same time)
- Scene Outliner. Parent / Child relation can be set dragging a node & dropping it onto parent.
- Entity inspector. Allows to see & modify every aspect of the scene elements via the inspector menu. (Name/Tag, Transforms, Mesh, Material)
- Snaping to grid & Fix delta moves for transform manipulators.
- Console window with various useful commands.
- Undo - Redo of most actions.

![Editor Footage](https://github.com/afraidofdark/ToolKit/blob/master/tk_ed_21.gif?raw=true "Editor")
![Another Editor Footage](https://github.com/afraidofdark/ToolKit/blob/master/tk_ed_22.gif?raw=true "Editor")
## Engine High Lights

- Default fragment & vertex shaders and ability to use custom shaders for materials.
- Sprite Sheets & Sprite Animation support.
- Key frame animation support.
- Skeletal animation support.
- Resource serialization & deserialization to xml format.
- Scene management.

![Game Sample](https://github.com/afraidofdark/ToolKit/blob/master/yes_10.gif?raw=true "Game")
## Build & Run

ToolKit comes with a visual studio solution. It should compile fine with Visual Studio 2019 community. All the required dependencies are in the project as prebuild for Windows 10. Features from C++11 to 17 has been utilized and I am planing to utilize co routines, modules and utf8 string from C++ 20.  

## Platforms

Once upon a time, I have successfully compile the engine with emscriptend and see it playing on my web browser. I expect that it can still be easily compiled.
There is no windows dependency. It can be built on linux and mac however my main focus is windows and web for know.

## Dependencies
- stb_image - MIT 
- SDL 2.0 - Zlib
- rapidxml - MIT
- openal-soft - LGPL (Dynamicyl linked)
- glm - MIT
- glew - BSD, MIT
- Dear imgui - MIT
- Assimp - BSD
- lodepng - Zlib

## Final Words

Project is in active development and I am planing to use it in my current and feature projects. Feel free to play around with it and get in touch with me.

Enjoy!
