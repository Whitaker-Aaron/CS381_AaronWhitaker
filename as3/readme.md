1. To fetch the git submodule, run the linux command: 

- git clone https://github.com/Whitaker-Aaron/CS381_AaronWhitaker.git

    then cd into the as3 folder.
- cd as3

2. To compile the code, empty out the build folder:

- rm -rf build

    and remake it. 
    
- mkdir build 

     Then run the cmake command: 

    - cmake ..

   after the cmake command is run, the only command left to run is the make command: 

- make

3. If the code compiled properly, then run 

- ./as3

    to run the executable, and the scene should load.

4. Controls:

    - Pressing the W key will increment the plane's velocity in the direction it is facing.
    - Pressing the S key will decrement the plane's velocity from the direction it is facing.
    - Pressing the A key will rotate the plane counter clockwise to the camera.
    - Pressing the D key will rotate the plane clockwise to the camera.
    - Pressing the Q key will increment the plane's velocity towards the sky, rotating the plane upwards.
    - Pressing the E key will increment the plane's velocity towards the ground, rotating the plane downwards.  
    - Pressing the left key will increment the camera's position to the left. 
    - Pressing the right key will increment the camera's position to the right.
    - Pressing the up key will increment the camera's position to the sky.
    - Pressing the down key will increment the camera's position to the ground.
    

   

Block of commands:

- git clone https://github.com/Whitaker-Aaron/CS381_AaronWhitaker.git
- cd as3  
- rm -rf build    
- cmake ..
- make
- ./as3
