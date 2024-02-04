#include <iostream>

#include "raylib-cpp.hpp"
#include "rlgl.h"




int main(){

raylib::Window window(1000, 700, "CS381 - Assignment 1");
int windHeight = 0, windWidth = 0;

//raylib::Model model = LoadModel("../meshes/CargoG_HOSBrigadoon.glb");

raylib::Model modelCargoG = LoadModel("../meshes/CargoG_HOSBrigadoon.glb");
raylib::Model modelShipL = LoadModel("../meshes/ShipLarge.glb");
raylib::Model modelOilT = LoadModel("../meshes/OilTanker.glb");
raylib::Model modelOrientE = LoadModel("../meshes/OrientExplorer.glb");
raylib::Model modelPolyP = LoadModel("../meshes/PolyPlane.glb");
raylib::Model modelPolyP2 = LoadModel("../meshes/PolyPlane.glb");
raylib::Model modelSmitH = LoadModel("../meshes/SmitHouston_Tug.glb");
raylib::Model modelDdg51 = LoadModel("../meshes/ddg51.glb");

//raylib::Model modelCargoG = LoadModelFromMesh("../meshes/PolyPlane.glb");
//raylib::Model modelBed = LoadModel("../meshes/BED.obj");
std::cout <<"I HAVENT CRASHED YET" << std::endl;

raylib::BoundingBox modelCargoGbounds = GetMeshBoundingBox(modelCargoG.meshes[0]); 
raylib::BoundingBox modelPolyPbounds = GetMeshBoundingBox(modelPolyP.meshes[0]); 
raylib::BoundingBox modelPolyP2bounds = GetMeshBoundingBox(modelPolyP2.meshes[0]);
raylib::BoundingBox modelOilTbounds = GetMeshBoundingBox(modelOilT.meshes[0]); 
raylib::BoundingBox modelShipLbounds = GetMeshBoundingBox(modelShipL.meshes[0]);
raylib::BoundingBox modelOrientEbounds = GetMeshBoundingBox(modelOrientE.meshes[0]);
raylib::BoundingBox modelSmitHbounds = GetMeshBoundingBox(modelSmitH.meshes[0]);
raylib::BoundingBox modelDdg51bounds = GetMeshBoundingBox(modelDdg51.meshes[0]);



Vector3 modelCargoGposition = { 100.0f, 0.0f, 0.0f };
Vector3 modelPolyPposition = { 0.0f, 0.0f, 0.0f };
Vector3 modelOilTposition = { 0.0f, 50.0f, 0.0f };
Vector3 modelShipLposition = { 0.0f, 0.0f, 0.0f };
Vector3 modelOrientEposition = { 200.0f, 0.0f, 0.0f };
Vector3 modelSmitHposition = { -100.0f, 25.0f, 0.0f };
Vector3 modelDdg51position = {-200.0f, 0.0f, 0.0f};



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

    //DrawModel(modelCargoG, modelCargoGposition, .05, raylib::WHITE);
    //DrawModel(modelPolyP, modelPolyPposition, 1, raylib::WHITE);
    //DrawModel(modelSmitH, modelSmitHposition, 1, raylib::WHITE);
    //DrawModel(modelDdg51, modelDdg51position, 1, raylib::WHITE);
    //DrawModel(modelOilT, modelOilTposition, 1, raylib::WHITE);
    DrawModelEx(modelOrientE, modelOrientEposition, raylib::Vector3(0.0f, 1.f, 0.0f), 90.0f, raylib::Vector3(0.05f, 0.05f, 0.05f), raylib::WHITE);

    DrawBoundingBox(modelCargoGbounds, raylib::WHITE);
    DrawBoundingBox(modelPolyP, raylib::WHITE);




   }
   camera.EndMode();
   

window.EndDrawing();

}
UnloadModel(modelCargoG);
UnloadModel(modelPolyP);
UnloadModel(modelSmitH);
UnloadModel(modelDdg51);
UnloadModel(modelOrientE);
UnloadModel(modelOilT);
UnloadModel(modelShipL);

    return 0;
}