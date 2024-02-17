1. To fetch the git submodule, run the linux command: 

- git clone https://github.com/Whitaker-Aaron/CS381_AaronWhitaker.git

    then cd into the as1 folder.
- cd as2

2. To compile the code, empty out the build folder:

- rm -rf build

    and remake it. 
    
- mkdir build 

     Then run the cmake command: 

    - cmake ..

   after the cmake command is run, the only command left to run is the make command: 

- make

3. If the code compiled properly, then run 

- ./as2

    to run the executable, and the small animation should play in the window.

4. Controls:

    - Pressing the W key will increment the plane's velocity away from the camera.
    - Pressing the S key will increment the plane's velocity toward the camera.
    - Pressing the A key will increment the plane's velocity to the left of the camera.
    - Pressing the D key will increment the plane's velocity to the right of the camera.
    - Pressing the Q key will increment the plane's velocity towards the sky.
    - Pressing the E key will increment the plane's velocity towards the ground.  
    - Pressing the left key will increment the camera's position to the left. 
    - Pressing the right key will increment the camera's position to the right.
    - Pressing the up key will increment the camera's position to the sky.
    - Pressing the down key will increment the camera's position to the ground.
    

   

Block of commands:

- git clone https://github.com/Whitaker-Aaron/CS381_AaronWhitaker.git
- cd as2  
- rm -rf build    
- cmake ..
- make
- ./as2

Reponse to question 5: 
-
We can not simplify the problem by limiting the movement to an axis because the movement in that scenario would be artificial. If we were to press W to only move it along the Z axis, then we could no longer move the plane along the other axes until resetting the position at the origin. This is not ideal for movement in a 3D space. 