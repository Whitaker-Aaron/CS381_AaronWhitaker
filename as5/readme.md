1. To fetch the git submodule, run the linux command: 

- git clone https://github.com/Whitaker-Aaron/CS381_AaronWhitaker.git

    then cd into the as5 folder.
- cd as5

2. To compile the code, empty out the build folder:

- rm -rf build

    and remake it. 
    
- mkdir build 

     Then run the cmake command: 

    - cmake ..

   after the cmake command is run, the only command left to run is the make command: 

- make

3. If the code compiled properly, then run 

- ./as5

    to run the executable, and the scene should load.

4. Controls:

    - Holding the Right Key will increment the audio of the selected slider.
    - Holding the Left Key will decrement the audio of the selected slider. 
    - Pressing the TAB key will change which slider is selected.
    - Pressing the SPACE bar will play the SFX if the volume is above 0.


   

Block of commands:

- git clone https://github.com/Whitaker-Aaron/CS381_AaronWhitaker.git
- cd as5
- rm -rf build    
- cmake ..
- make
- ./as5
