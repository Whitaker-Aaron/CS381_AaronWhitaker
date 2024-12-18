#include <iostream>
#include "raylib-cpp.hpp"

int main(){
    raylib::Window window(500, 500, "CS381 - Assignment 0");
    raylib::Text text;
    bool flag = false, flag2 = false;
    float animate = 40;
    int windHeight = 0, windWidth = 0;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);

    while(!window.ShouldClose()) { 
        window.BeginDrawing();
        {
            windWidth = window.GetWidth(); 
            windHeight = window.GetHeight();

           
            
             if(flag2 == false){
                window.ClearBackground(RAYWHITE);
                text.Draw("Black", (windWidth/2 - animate), (windHeight/2 - 20), 20, raylib::Color::Black());
                //flag = true;
            }

             else if(flag2 == true){
                window.ClearBackground(BLACK);
                text.Draw("White", (windWidth/2 - animate), (windHeight/2 - 20), 20, raylib::Color::White());
                //flag = false;
            }

        if(flag2 == false){
            animate += 0.1;
            if(animate >= 140){
                flag2 = true;
            }
        }

        else if(flag2 == true){
            animate -= 0.1;
            //std::cout << animate << std::endl;
            if(animate <= 0.0f){
                //std::cout << "second condition triggered" << std::endl;
                flag2 = false;
            }
        }

         
            
        }
        window.EndDrawing();

    }

    return 0;
}

//Three types of architectures: 
//- State based: Setup state, changes on state update --> OpenGL, DirectX 
//- Command based: Record and replay --> Vulcan, Metal, WebGPU
//- Immediate Based: Ask -> immediate, everytime 