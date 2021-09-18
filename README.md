

# ToolKit

ToolKit is a 3d editor & interactive application development platform. It allows users to create 3d scenes and bring in the interactivity via C++ plug-ins.

## Motivation

### Simplicity

Most games, projects using Unity, UE4 or even Godot, don't need all those tools and functionalities. Having a far simple game engine, increases your comprehansion of the framework & tools, wich in return gives you more freedom to do things your way. This approach may not be suitable for professional game studios, however it is very suitable for indies whom after unique projects.

This project has a unique goal which is keeping all the source code under 30k lines of code excluding dependencies. Instead of adding more capabilites, tools and getting more complicated, ToolKit will be oriented towards simplicity, performance and being/staying modern.

### Community

Bringing like minded people together around a project outputs invaluable assets as one can observe from projects like Blender & Godot. So the project's motivation is to bring people together who after uniqueness, simplicity, modernitiy and colloborative effort to create someting exceptional.

### Self Development

- While working on the ToolKit, you'll learn all about new frameworks, improvements in the language and tecniques used in the industry. 
- Working on ToolKit keeps your ability to learn, adapt to new things and accept the change.
- It teaches you all about project planning and using resources efficiently.
- It teaches you your limits and to be passionate.
- It teaches you to enjoy the road, not the destination.
- It teaches you to accept imperfect.

## Editor High Lights
- All required functionalites for scene creation, manipuation, save & load.
- All required utilities for Asset management. Create / Save / Refresh / Browse resource directories.
- Support having a workspace & multiple projects.
- Multiple Prespective & Orthographic views.
- Scene outliner to observe / interact entities in the scene.
- Entity inspector & interactive resource manipulation trough the inspector.
- Import whole scenes from various programs & formats including sketchfab, blender, glb, fbx ...
- Console window along with usefull scene inspection commands and easy command creation.

![Editor Footage 1](https://github.com/afraidofdark/ToolKit/blob/master/tk_ed_21.gif?raw=true "Editor")
![Editor Footage 2](https://github.com/afraidofdark/ToolKit/blob/master/tk_ed_22.gif?raw=true "Editor")
![Editor Footage 3](https://github.com/afraidofdark/ToolKit/blob/master/tk_ed_23.jpeg?raw=true "Editor")
## Engine High Lights

- Default fragment & vertex shaders and ability to create custom shaders for materials.
- Sprite Sheets & Sprite Animation.
- Skeletal animation & Key frame animation.
- Resource serialization & deserialization to xml format.
- Scene management.

![Game Sample](https://github.com/afraidofdark/ToolKit/blob/master/yes_10.gif?raw=true "Game")
## Build & Run

ToolKit comes with a Visual Studio solution. The solution has been created with Visual Studio 2019 community. All the required dependencies are in the project as prebuilt for Windows 10. 

### Creating new project
- Copy the visual studio template "ToolKit\ToolKitProject.zip" to  "_%USERPROFILE%\Documents\Visual Studio 2019\Templates\ProjectTemplates_
- Restart the Visual Studio and open the solution "ToolKit\ToolKit.sln"
- In the Solution Explorer, right click the solution "ToolKit"
- Add -> New Project
- Search for ToolKitProject and select it
- Fallow the instructions, this will create a new blank project

### Projects in the solution

#### Engine
- *ToolKit project* is the engine it self.
- *Editor project* is the 3d editor to create conent for your projects.
- *Import project* is the executable that Editor is utilizing for importing the resources for projects.

#### Sample
- *SpaceShooter project* is a unit test project and also serving as a showcase.
- *Template project* is a Visual Studio project template. You can make your adjustments to the template and export it to use as your project template. 

## Platforms

Currently, ToolKit can only be build for Windows. However it has been tested for web builds and can generate web outputs with Emscripten. The second platfrom that it gives output will be web. ToolKit does not have any Windows dependency and can be build for  Mac-Os and Linux. Each os. should provide a convinient workflow by providing a code editor and well established compile system. For this reason Mac & Linux won't be handled until a stable release published.

### Web Platform

This feature is not stable & in active development.
- Install emscripten, download ninja and place it in, emsdk / upstream / emscripten. 
- In the ToolKit directory, create a Build directory. 
- Enter Build. Open cmd and Run "emcmake cmake -S .."
- Than run ninja
- Install chrome
- Install chrome C/C++ DevTools Support (Dwarf)
- From the developer tools settings (wheel icon *)
- Preferences / Sources / Check: "Enable javascript source maps"
- Experiments /  Check: "WebAssembly Debugging: Enable DWARF support"
- Go to ToolKit / Bin open cmd run "emrun SpaceShooter.html --browser chrome

You can see the c++ code, call stack, local - global scope and set break points.

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

## Roadmap

### Renderer
- Currently I am using Opengl ES 2.0 along with Sdl. I have not isolated renderer from the underlying api. That is, renderer, texture, mesh ect ... makes direct calls to the Opengl. This isolation is needed. 
- Forward renderer. 
  - Sort objects to minimize redundant state changes.
  - Support transparency by sorting objects by depth.
  - Support double sided transparent objects.
  - Dynamic batch small objects in polygon size.
  - Utilize instantiate for bigger objects in polygon size.
  - Environment lighting trough global light probe.
  - Simple shadow maps for directional lights.
  - Allow, pick 8 lights per object.
  - Point, spot, directional light entities for the editor.

### Scene manager
 - Implement Non-uniform octree scene patitioning.
 - Implement frustum culling.
 - Update picking queries to utilze octree.

### Editor
- Editor Improvements
  - Transform gizmo improvements.
  - Viewport orbit view improvements.
  - Sound & Image import with drag & drop.
  - Asset viewer improvements.
- Game Plug-In
  - Play - pause - stop the current scene which require, compiling & loading of the game play plugin and run it within editor. 

### Engine
- Play 3d sound.
- Play background sound.
- Replace openal with  MojoAL.
- Create sound entity for the editor.

## Final Words

Project is in active development and I am planing to use it in my current and feature projects. Feel free to play around with it and get in touch with me.

Enjoy!
