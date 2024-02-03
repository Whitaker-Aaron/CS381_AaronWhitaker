#include <iostream>

#include "raylib-cpp.hpp"
#include "rlgl.h"




int main(){

raylib::Window window(500, 500, "CS381 - Assignment 1");
int windHeight = 0, windWidth = 0;

//raylib::Model model = LoadModel("../meshes/CargoG_HOSBrigadoon.glb");

raylib::Model modelCargoG = LoadModel("../meshes/CargoG_HOSBrigadoon.glb");
raylib::Model modelPolyP = LoadModel("../meshes/PolyPlane.glb");
//raylib::Model modelCargoG = LoadModelFromMesh("../meshes/PolyPlane.glb");
//raylib::Model modelBed = LoadModel("../meshes/BED.obj");
std::cout <<"I HAVENT CRASHED YET" << std::endl;

raylib::BoundingBox modelCargoGbounds = GetMeshBoundingBox(modelCargoG.meshes[0]); 
raylib::BoundingBox modelPolyPbounds = GetMeshBoundingBox(modelPolyP.meshes[0]); 

Vector3 modelCargoGposition = { 100.0f, 0.0f, 0.0f };
Vector3 modelPolyPposition = { 0.0f, 0.0f, 0.0f };




raylib::Camera camera(
        raylib::Vector3(0, 120, 500),
        raylib::Vector3(0, 0, 300),
        raylib::Vector3::Up(),
        45,
        CAMERA_PERSPECTIVE
    );





while(!window.ShouldClose()) { 


window.BeginDrawing();

 windWidth = window.GetWidth(); 
 windHeight = window.GetHeight();

 window.ClearBackground(BLACK);
   camera.BeginMode();
   {

    DrawModel(modelCargoG, modelCargoGposition, .05, raylib::WHITE);
    DrawModel(modelPolyP, modelPolyPposition, 1, raylib::WHITE);

    DrawBoundingBox(modelCargoGbounds, raylib::WHITE);
    DrawBoundingBox(modelPolyP, raylib::WHITE);




   }
   camera.EndMode();
   

window.EndDrawing();

}
UnloadModel(modelCargoG);
UnloadModel(modelPolyP);



    return 0;
}