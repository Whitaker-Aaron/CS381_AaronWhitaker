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

raylib::Transform transform = raylib::Transform::Identity();


raylib::Window window(1000, 700, "CS381 - Assignment 1");
int windHeight = 0, windWidth = 0;

raylib::Model modelBread = LoadModel("../meshes/bread.glb");
modelBread.transform = raylib::Transform(modelBread.transform).Scale(30,30,30);
modelBread.transform = raylib::Transform(modelBread.transform).RotateXYZ(30,30,30);

raylib::Model modelPolyP = LoadModel("../meshes/PolyPlane.glb");

/*
raylib::Model modelShipL = LoadModel("../meshes/ShipLarge.glb");
raylib::Model modelOilT = LoadModel("../meshes/OilTanker.glb");
raylib::Model modelOrientE = LoadModel("../meshes/OrientExplorer.glb");
*/



//raylib::Model modelPolyP2 = LoadModel("../meshes/PolyPlane.glb");

/*raylib::Model modelSmitH = LoadModel("../meshes/SmitHouston_Tug.glb");
modelSmitH.transform = raylib::Transform(modelSmitH.transform).Scale(1, 2, 1);
modelSmitH.transform = raylib::Transform(modelSmitH.transform).RotateXYZ(raylib::Degree(270),0 ,raylib::Degree(270));
*/


raylib::Model modelDdg51 = LoadModel("../meshes/ddg51.glb");




raylib::Camera camera(
        raylib::Vector3(0, 120, -400),
        raylib::Vector3(0, 0, 250),
        raylib::Vector3::Up(),
        45,
        CAMERA_PERSPECTIVE
    );


float increment = 0.05; 


while(!window.ShouldClose()) { 

window.BeginDrawing();

 windWidth = window.GetWidth(); 
 windHeight = window.GetHeight();

 

 window.ClearBackground(BLACK);
   camera.BeginMode();
   {


    raylib::Transform transform = raylib::Transform::Identity();

    DrawBoundedModel(modelPolyP, [transform](raylib::Transform t) -> raylib::Transform{
        return t * transform;
    }, increment);


    transform = raylib::Transform::Identity();
    transform = transform * raylib::Transform(modelPolyP.transform).Scale(1, -1, 1);
    transform = transform * raylib::Transform(modelPolyP.transform).RotateXYZ(raylib::Degree(180), 0 ,raylib::Degree(180));
    transform = transform *  raylib::Transform(modelPolyP.transform).Translate(0, 50, 0);

    DrawBoundedModel(modelPolyP, [transform](raylib::Transform t) -> raylib::Transform{
        return t * transform;
    }, increment);

    transform = raylib::Transform::Identity();
    transform = transform * raylib::Transform(modelDdg51.transform).Scale(1, 2, 1);
    transform = transform * raylib::Transform(modelDdg51.transform).RotateXYZ(raylib::Degree(270), 0 ,raylib::Degree(270));
    transform = transform *  raylib::Transform(modelDdg51.transform).Translate(70, 150, 0);

    DrawBoundedModel(modelDdg51, [transform](raylib::Transform t) -> raylib::Transform{
        return t * transform;
    }, increment);

    transform = raylib::Transform::Identity();
    transform = transform *  raylib::Transform(modelDdg51.transform).Translate(-150, 0, 0);

    DrawBoundedModel(modelDdg51, [transform](raylib::Transform t) -> raylib::Transform{
        return t * transform;
    }, increment);

    transform = raylib::Transform::Identity();
    transform = transform * raylib::Transform(modelDdg51.transform).RotateY(raylib::Degree(90));
    transform = transform *  raylib::Transform(modelDdg51.transform).Translate(100, -20, 0);

    DrawBoundedModel(modelDdg51, [transform](raylib::Transform t) -> raylib::Transform{
        return t * transform;
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

