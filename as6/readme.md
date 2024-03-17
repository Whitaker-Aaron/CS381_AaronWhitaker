1. To fetch the git submodule, run the linux command: 

- git clone https://github.com/Whitaker-Aaron/CS381_AaronWhitaker.git

    then cd into the as5 folder.
- cd as6

2. To compile the code, empty out the build folder:

- rm -rf build

    and remake it. 
    
- mkdir build 

     Then run the cmake command: 

    - cmake ..

   after the cmake command is run, the only command left to run is the make command: 

- make

3. If the code compiled properly, then run 

- ./as6

    to run the executable, and the scene should load.

4. Controls:

    - Pressing the W key will increment the object's velocity in the direction it is facing.
    - Pressing the S key will decrement the object's velocity from the direction it is facing.
    - Pressing the A key will rotate the plane counter clockwise to the camera.
    - Pressing the D key will rotate the plane clockwise to the camera. 
    - Pressing the TAB key will change the selection of the object.
    - Pressing SPACE will slow the object to a halt.
    - Pressing ESC will terminate the program.



   

Block of commands:

- git clone https://github.com/Whitaker-Aaron/CS381_AaronWhitaker.git
- cd as6
- rm -rf build    
- cmake ..
- make
- ./as6
