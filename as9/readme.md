
# AS9 Custom Game

1. To fetch the git submodule, run the linux command: 

- git clone https://github.com/Whitaker-Aaron/CS381_AaronWhitaker.git

    then cd into the as9 folder.
- cd as9

2. To compile the code, empty out the build folder:

- rm -rf build

    and remake it. 
    
- mkdir build 

     Then run the cmake command: 

    - cmake ..

   after the cmake command is run, the only command left to run is the make command: 

- make

3. If the code compiled properly, then run 

- ./as9

    to run the executable, and the scene should load.

4. Controls:

    - Pressing the W key will move the ship upwards
    - Pressing the S key will move the ship downwards
    - Pressing the A key will move the ship left
    - Pressing the D key will move the ship right
    - Pressing SPACE key will fire the ship's laser.


Block of commands:

- git clone https://github.com/Whitaker-Aaron/CS381_AaronWhitaker.git
- cd as9
- rm -rf build    
- cmake ..
- make
- ./as9

Description of Criteria: 
- Must be interactable: 
    - Takes user input in the form of the WASD and Space keys and the ship responds accordingly.
    
- Must Draw Model or Play Sound
    - Model is drawn on screen every frame along with multiple song tracks accompanying gameplay. 

- Must Display Text 
    - Game displays player HUD with information pertaining to ship health, laser status, and boss health. 

- Window named AS9 
    - There is a window called AS9 :P

- Minimum 2 System not proposed in class
    - There is a DisplayHUD system that draws player information to screen 
    - There is a GameState system that tracks the state of the game based on data of player 
    - There is a PlayerUpdate system that tracks movement every frame 
    - There is an UpdateShip system that updates the ship's properties based on target variables, such as animating the ship's health and laser. 
    
   

