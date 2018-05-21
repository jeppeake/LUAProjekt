# LUAProjekt


#Att göra:

**b**:
1. Integrate Lua and Irrlicht as shown in the labs.

2. Add the Irrlicht FPS controls so that they are active only when the window is in focus (i.e. if you switch to the terminal the FPS camera should stop grabbing the mouse and keyboard).
    
    Switch off back-face culling so that you do not need to respect the triangle winding order.
    
    Implement the addMesh, addBox, getNodes, camera and snapshot interfaces described below.
    
    As scene nodes are added using these interfaces names must be generated if not supplied, and unique IDs.
    
    The getNodes call return the type, name and id of each scene node in a table.
    
    Each of the interfaces must verify their arguments and generate appropriate errors for the test data given later. 
