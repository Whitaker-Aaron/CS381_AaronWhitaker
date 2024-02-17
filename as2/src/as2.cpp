#include <iostream>

#include "raylib-cpp.hpp"
#include "rlgl.h"
#include "skybox.hpp"

template<typename T>
concept Transformer = requires(T t, raylib::Transform m) {
   {t.operator()(m) } -> std::convertible_to<raylib::Transform>;
};

void DrawBoundedModel(raylib::Model& model, Transformer auto transformer){
    raylib::Transform backupTransform = model.transform;
    model.transform = transformer(model.transform);
    model.Draw({});
    //model.GetTransformedBoundingBox().Draw();
    model.transform = backupTransform;
}

int main(){

raylib::Window window(1000, 700, "CS381 - Assignment 2");
int windHeight = 0, windWidth = 0;
bool playSound = false;

InitAudioDevice();

raylib::Model modelPolyP = LoadModel("../meshes/PolyPlane.glb");
modelPolyP.transform = raylib::Transform(modelPolyP.transform).Scale(1.5, 1.5, 1.5);

raylib::Model modelIgloo = LoadModel("../meshes/low_poly_igloo.glb");

raylib::Model modelTree = LoadModel("../meshes/12150_Christmas_Tree_V2_L2.obj");
raylib::Texture tree("../textures/tree.jpg");
 tree.SetFilter(TEXTURE_FILTER_BILINEAR);
 tree.SetWrap(TEXTURE_WRAP_REPEAT);
 modelTree.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = tree;

Music planeWav = LoadMusicStream("../sounds/airplane-fly-by-02.wav");

raylib::Camera camera(
        raylib::Vector3(0, 120, -400),
        raylib::Vector3(0, 0, 250),
        raylib::Vector3::Up(),
        45,
        CAMERA_PERSPECTIVE
    );

cs381::SkyBox skybox("textures/skybox.png");

 auto mesh = raylib::Mesh::Plane(10'000, 10'000, 50, 50, 25);
 raylib::Model ground = ((raylib::Mesh*)&mesh)->LoadModelFrom();
 raylib::Texture grass("textures/ice2.jpg");
 grass.SetFilter(TEXTURE_FILTER_BILINEAR);
 grass.SetWrap(TEXTURE_WRAP_REPEAT);
 ground.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = grass;

 raylib::Vector3 position = {0, 0, 0};
 raylib::Vector3 velocity = {0, 0, 0};


 SetTargetFPS(60);
 SetMusicVolume(planeWav, .3f);
 
 
PlayMusicStream(planeWav);
std::cout << IsMusicStreamPlaying(planeWav) << std::endl;
while(!window.ShouldClose()) { 

if(IsKeyPressed(KEY_W)){
  velocity.z += 20;
  playSound = true; 
 }

if(IsKeyPressed(KEY_UP)){
  camera.position.y += 5; 
  
 } 

 if(IsKeyPressed(KEY_S)){
  velocity.z -= 20;
  playSound = true;  
 }

 if(IsKeyPressed(KEY_DOWN)){
  camera.position.y -= 5;
  
 }

 if(IsKeyPressed(KEY_A)){
  velocity.x += 20;
  playSound = true;  
 }

 if(IsKeyPressed(KEY_LEFT)){
  camera.position.x += 5; 
 }

 if(IsKeyPressed(KEY_D)){
  velocity.x -= 20;
  playSound = true;  
 }

 if(IsKeyPressed(KEY_RIGHT)){
  camera.position.x -= 5; 
 }

 if(IsKeyPressed(KEY_Q)){
  velocity.y += 20;
  playSound = true;  
 }

 if(IsKeyPressed(KEY_E)){
  velocity.y -= 20;
  playSound = true;  
 }

 position = position + velocity*window.GetFrameTime();

window.BeginDrawing();

 windWidth = window.GetWidth(); 
 windHeight = window.GetHeight();

 window.ClearBackground(BLACK);
 
 
 camera.BeginMode();
 {
    if(playSound == true){
    //std::cout << "Playing sound" << std::endl;
    
    
    playSound = false;
    }
    //std::cout << IsSoundPlaying(planeMp3) << std::endl;

    skybox.Draw();
    raylib::Transform transform = raylib::Transform::Identity();
    raylib::Transform transform_igloo = raylib::Transform::Identity();
    raylib::Transform transform_tree = raylib::Transform::Identity();
    
    //Place the igloo in the scene
    transform_igloo = transform_igloo * raylib::Transform(modelIgloo.transform).Scale(40, 40, 40);
    transform_igloo = transform_igloo * raylib::Transform(modelIgloo.transform).RotateY(raylib::Degree(270));
    transform_igloo = transform_igloo * raylib::Transform(modelIgloo.transform).Translate(0, 0, 300);

    //Place the trees in the scene
    transform_tree = transform_tree * raylib::Transform(modelTree.transform).RotateX(raylib::Degree(270));
    transform_tree = transform_tree * raylib::Transform(modelTree.transform).Translate(100, 0, -200);
    

    transform = transform *  raylib::Transform(modelPolyP.transform).Translate(position.x, position.y, position.z);

    DrawBoundedModel(modelIgloo, [transform_igloo](raylib::Transform t) -> raylib::Transform{
        return t * transform_igloo; 
    });

    DrawBoundedModel(modelTree, [transform_tree](raylib::Transform t) -> raylib::Transform{
        return t * transform_tree; 
    });

    transform_tree = raylib::Transform::Identity();
    transform_tree = transform_tree * raylib::Transform(modelTree.transform).RotateX(raylib::Degree(270));
    transform_tree = transform_tree * raylib::Transform(modelTree.transform).Translate(150, 0, 350);

    DrawBoundedModel(modelTree, [transform_tree](raylib::Transform t) -> raylib::Transform{
        return t * transform_tree; 
    });

    transform_tree = raylib::Transform::Identity();
    transform_tree = transform_tree * raylib::Transform(modelTree.transform).RotateX(raylib::Degree(270));
    transform_tree = transform_tree * raylib::Transform(modelTree.transform).Translate(-200, 0, 200);

    DrawBoundedModel(modelTree, [transform_tree](raylib::Transform t) -> raylib::Transform{
        return t * transform_tree; 
    });

    DrawBoundedModel(modelPolyP, [transform](raylib::Transform t) -> raylib::Transform{
        return t * transform; 
    });

    ground.Draw({});



 }
 camera.EndMode();
   

window.EndDrawing();

}











    return 0;
}