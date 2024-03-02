1. To fetch the git submodule, run the linux command: 

- git clone https://github.com/Whitaker-Aaron/CS381_AaronWhitaker.git

    then cd into the as4 folder.
- cd as4

2. To compile the code, empty out the build folder:

- rm -rf build

    and remake it. 
    
- mkdir build 

     Then run the cmake command: 

    - cmake ..

   after the cmake command is run, the only command left to run is the make command: 

- make

3. If the code compiled properly, then run 

- ./as4

    to run the executable, and the scene should load.

4. Controls:

    - Holding the W key will move the ship upwards.
    - Holding the S key will move the ship downwards.
    - Holding the A key will move the ship left.
    - Holding the D key will move the ship right.
    - Holding the up key will move the camera's perspective upwards
    - Holding the down key will move the camera's perspective downwards.
    - Pressing the SPACE bar will boost the ship right, only if the right key is being held.

5. Objective: 
    - The camera will pan automatically to the right. Don't get hit by the meteors or your health will deteriorate. 
      When your health falls into destroyed, the game is over. Reach the goal and you win. 
    

   

Block of commands:

- git clone https://github.com/Whitaker-Aaron/CS381_AaronWhitaker.git
- cd as4
- rm -rf build    
- cmake ..
- make
- ./as4
