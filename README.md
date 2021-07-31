# ToolKit

ToolKit is a 3d editor & interactive application development platform. It allows users to create 3d scenes.

## Editor High Lights

 - 3d editor is capable of importing various models via drag & drop from desktop.
 - Multiple perspective viewports and an orthographic viewport.
 - Create materials, manipulate and asign materials to objects.
 - Hierarchical transformations (Parent - child relation for objects).
 - Load & Save scenes.
 - All transform manupilations (Translate - Rotate - Scale) in 3 spaces (Parent - Local - World).
 - Asset Browser & Allow multiple Asset browser in the editor. (Such as Mesh & Material Panels can be open on the same time)
 - Scene Outliner. Parent / Child relation can be set draging a node & droping it on to potential parent.
 - Entity inspector. Allows to see & modify every aspect of the scene elements via the inspector menu. (Name/Tag, Transforms, Mesh, Material)
 - Snaping to grid & Fix delta moves for transform manipulators.
 - Console window with various usefull commands.
![Editor Footage](https://github.com/afraidofdark/ToolKit/blob/master/tk_ed_21.gif?raw=true "Editor")
![Another Editor Footage](https://github.com/afraidofdark/ToolKit/blob/master/tk_ed_22.gif?raw=true "Editor")

# Engine High Lights
- Default pixel & vertex shaders and ability to use Custom shaders for materials.
- Sprite Sheets & Sprite Animation support.
- Key frame animation support.
- Skeletal animation support.
- Resource serialization & deserialization to xml format.
- Scene management.

![Game Sample](https://github.com/afraidofdark/ToolKit/blob/master/yes_10.gif?raw=true "Game"))

## Build & Run

Project comes with a visual studio solution. It should compile fine with VS-2019 community. All the required dependencies are in the project and visual studio is adjusted as it should. However there is an executable to import assets. That executable need Assimp to be compiled. I left it to you if you wish to compile importer. Features from C++11 to 17 has been utilized and I am planing to utilize coroutines, modules and utf8 string from C++ 20.

## Platforms

Once upon a time, I have sucessfully compile the engine with emscriptend and see it playing on my web browser. I expect that it can still be easly compiled.
There is no windows dependency. It can be built on linux and mac however my main focus is windows and web for know.

## Final Words

Its in actively development and I am planing to use / actively use it in my current and feature projects. Feel free to play around with it and get in touch with me.
Enjoy !
