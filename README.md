﻿# LUAProjekt

**A LUA implementation in an existing 3D engine made for a scripting course at BTH. Group: Gustaf Fagerström & Jesper Åkesson**

#Att göra:

**E:**
1. Integrate Lua and Irrlicht as shown in the labs.*

2. Add the Irrlicht FPS controls so that they are active only when the window is in focus (i.e. if you switch to the terminal the FPS camera should stop grabbing the mouse and keyboard).*

3. Switch off back-face culling so that you do not need to respect the triangle winding order.*
    
4. Implement: 

**addMesh** (J)*

**addBox** (G)*

**getNodes** (J)*

**camera** (G)*

**snapshot** (J)*
    
5. As scene nodes are added using these interfaces names must be generated if not supplied, and unique IDs.*
    
6. The getNodes call return the type, name and id of each scene node in a table.*
    
7. Each of the interfaces must verify their arguments and generate appropriate errors for the test data given later. *

**D:**

1. Implement the addTexture, and bind calls as described below.* 

**C:**

1. Define the scene configuration language from the example data given below.*

2. Implement the loadScene interface so that it destroys any current scene and instantiates the scene described in the given file.*

3. The Lua(< ... >) construct is not necessary for this grade. *
