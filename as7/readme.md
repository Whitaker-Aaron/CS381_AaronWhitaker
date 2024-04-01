1. To fetch the git submodule, run the linux command: 

- git clone https://github.com/Whitaker-Aaron/CS381_AaronWhitaker.git

    then cd into the as6 folder.
- cd as7

2. To compile the code, empty out the build folder:

- rm -rf build

    and remake it. 
    
- mkdir build 

     Then run the cmake command: 

    - cmake ..

   after the cmake command is run, the only command left to run is the make command: 

- make

3. If the code compiled properly, then run 

- ./as7

    to run the executable, and the scene should load.

4. Controls:

    - Pressing the W key will move the ship upwards
    - Pressing the S key will move the ship downwards
    - Pressing the A key will move the ship left
    - Pressing the D key will move the ship right
    - Pressing SPACE key will fire the ship's laser.



   

Block of commands:

- git clone https://github.com/Whitaker-Aaron/CS381_AaronWhitaker.git
- cd as7
- rm -rf build    
- cmake ..
- make
- ./as7
