#include <iostream>

#include "raylib-cpp.hpp"
#include "rlgl.h"

template<typename T>
concept Transformer = requires(T t, raylib::Transform m) {
   {t.operator()(m) } -> std::convertible_to<raylib::Transform>;
};

void DrawBoundedModel(raylib::Model& model, Transformer auto transformer, float index){
    raylib::Transform backupTransform = model.transform;
    model.transform = transformer(model.transform);
    model.Draw({});
    model.GetTransformedBoundingBox().Draw();
    model.transform = backupTransform;
}


int main(){

raylib::Window window(1000, 700, "CS381 - Assignment 1");
int windHeight = 0, windWidth = 0;

//raylib::Model model = LoadModel("../meshes/CargoG_HOSBrigadoon.glb");

raylib::Model modelDiscordIcon = LoadModel("../meshes/3d icon discord.glb");
raylib::Model modelBread = LoadModel("../meshes/bread.glb");
modelBread.transform = raylib::Transform(modelBread.transform).Scale(30,30,30);
modelBread.transform = raylib::Transform(modelBread.transform).RotateXYZ(30,30,30);

raylib::Model modelCargoG = LoadModel("../meshes/CargoG_HOSBrigadoon.glb");
modelCargoG.transform = raylib::Transform(modelCargoG.transform).Scale(0.02,0.02,0.02);

raylib::Model modelShipL = LoadModel("../meshes/ShipLarge.glb");
raylib::Model modelOilT = LoadModel("../meshes/OilTanker.glb");
raylib::Model modelOrientE = LoadModel("../meshes/OrientExplorer.glb");
raylib::Model modelPolyP = LoadModel("../meshes/PolyPlane.glb");

raylib::Model modelPolyP2 = LoadModel("../meshes/PolyPlane.glb");
modelPolyP2.transform = raylib::Transform(modelPolyP2.transform).RotateXYZ(raylib::Degree(180),0 ,raylib::Degree(180));
modelPolyP2.transform = raylib::Transform(modelPolyP2.transform).Scale(1,-1,1);

raylib::Model modelSmitH = LoadModel("../meshes/SmitHouston_Tug.glb");
modelSmitH.transform = raylib::Transform(modelSmitH.transform).Scale(1, 2, 1);
modelSmitH.transform = raylib::Transform(modelSmitH.transform).RotateXYZ(raylib::Degree(270),0 ,raylib::Degree(270));



raylib::Model modelDdg51 = LoadModel("../meshes/ddg51.glb");




raylib::Camera camera(
        raylib::Vector3(0, 120, -500),
        raylib::Vector3(0, 0, 300),
        raylib::Vector3::Up(),
        45,
        CAMERA_PERSPECTIVE
    );


float increment = 0.05; 


while(!window.ShouldClose()) { 

/*if(increment >= 360){
    increment = 0;
    
}
    increment += 0.01 * window.GetFrameTime();
    std::cout << increment << std::endl;*/

window.BeginDrawing();

 windWidth = window.GetWidth(); 
 windHeight = window.GetHeight();

 

 window.ClearBackground(BLACK);
   camera.BeginMode();
   {


    DrawBoundedModel(modelCargoG, [](raylib::Transform t) -> raylib::Transform{
        return t.Translate({100, 0, 0});
    }, increment);

    DrawBoundedModel(modelPolyP, [](raylib::Transform t) -> raylib::Transform{
        return t.Translate({0, 0, 0});
    }, increment);

    DrawBoundedModel(modelPolyP2, [](raylib::Transform t) -> raylib::Transform{
        return t.Translate({-100, 100, 0});
    }, increment);

    DrawBoundedModel(modelSmitH, [](raylib::Transform t) -> raylib::Transform{
        return t.Translate({100, 100, 0});
    }, increment);

    DrawBoundedModel(modelDdg51, [](raylib::Transform t) -> raylib::Transform{
        return t.Translate({-200, 0, 0});
    }, increment);


    modelBread.transform = raylib::Transform(modelBread.transform).RotateXYZ(increment,increment,increment);
    DrawBoundedModel(modelBread, [](raylib::Transform t) -> raylib::Transform{
        return t.Translate({0, -100, 0});
    }, increment);

    


   }
   camera.EndMode();
   

window.EndDrawing();

}

    return 0;
}

