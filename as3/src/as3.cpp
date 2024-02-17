#include <iostream>

#include "raylib-cpp.hpp"
#include "rlgl.h"
#include "skybox.hpp"

template<typename T>
concept Transformer = requires(T t, raylib::Transform m) {
   {t.operator()(m) } -> std::convertible_to<raylib::Transform>;
};

void DrawBoundedModel(raylib::Model& model, Transformer auto transformer, bool drawBox){
    raylib::Transform backupTransform = model.transform;
    model.transform = transformer(model.transform);
    model.Draw({});
    if(drawBox) model.GetTransformedBoundingBox().Draw();
    model.transform = backupTransform;
}

int main(){

raylib::Window window(1000, 700, "CS381 - Assignment 3");
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


 float speed = 10, targetSpeed = 10;
 raylib::Degree heading = 10, targetHeading = 10;

 //raylib::Vector3 testVelocity = {speed*cos(heading.RadianValue()), 0, -speed*sin(heading.RadianValue())};
 //position += velocity * window.GetFrameTime();
 //float acceleration = 10;
 //if(targetSpeed > speed)
 // speed += acceleration * window.GetFrameTime();
 // if(targetSpeed < speed)
 // speed -= acceleration * window.GetFrameTime(); 
 // NOT USING SPEED STUFF FOR NOW 

 //if(targeHeading > heading)
 // heading += acceleration * window.GetFrameTime();
 // if(targetHeading < heading)
 // heading -= acceleration * window.GetFrameTime(); 

 //if(IsKeyPressed(KEY_W))
 //     targetSpeed += 10;
 //if(IsKeyPressed(KEY_A))
 //       targetHeading+= 60;
 //if(IsKeyPressed(KEY_A))
 //       targetHeading-= 60;
 //Zoom recording: 38:26

 raylib::Vector3 position = {0, 0, 0};
 raylib::Vector3 position2 = {0, 0, 0};
 raylib::Vector3 position3 = {0, 0, 0};

 raylib::Vector3 velocity = {0, 0, 0};
 raylib::Vector3 velocity2 = {0, 0, 0};
 raylib::Vector3 velocity3 = {0, 0, 0};



 SetTargetFPS(60);
 SetMusicVolume(planeWav, .3f);
 
 
PlayMusicStream(planeWav);
std::cout << IsMusicStreamPlaying(planeWav) << std::endl;

int planeIndex = 0;
bool plane1 = false, plane2 = false, plane3 = false;


while(!window.ShouldClose()) { 

if(IsKeyPressed(KEY_TAB)){ 
 planeIndex++;
 if(planeIndex > 2){
    planeIndex = 0;
 }
}

switch(planeIndex){
    case 0:
    plane1 = true;
    plane2 = false;
    plane3 = false;
    break;

    case 1:
    plane1 = false;
    plane2 = true;
    plane3 = false;
    break;

    case 2:
    plane1 = false;
    plane2 = false;
    plane3 = true;
    break;
}


if(IsKeyPressed(KEY_W)){
  if(plane1)
  velocity.z += 20;

  if(plane2)
  velocity2.z += 20;

  if(plane3)
  velocity3.z += 20;

  playSound = true; 
 }

if(IsKeyPressed(KEY_UP)){
  camera.position.y += 5; 
  
 } 

 if(IsKeyPressed(KEY_S)){
  if(plane1)
  velocity.z -= 20;

  if(plane2)
  velocity2.z -= 20;

  if(plane3)
  velocity3.z -= 20;
  playSound = true;  
 }

 if(IsKeyPressed(KEY_DOWN)){
  camera.position.y -= 5;
  
 }

 if(IsKeyPressed(KEY_A)){
  if(plane1)
  velocity.x += 20;
  heading += 60;

  if(plane2)
  velocity2.x += 20;

  if(plane3)
  velocity3.x += 20;
  playSound = true;  
 }

 if(IsKeyPressed(KEY_LEFT)){
  camera.position.x += 5; 
 }

 if(IsKeyPressed(KEY_D)){
  if(plane1)
  velocity.x -= 20;

  if(plane2)
  velocity2.x -= 20;

  if(plane3)
  velocity3.x -= 20;
  playSound = true;  
 }

 if(IsKeyPressed(KEY_RIGHT)){
  camera.position.x -= 5; 
 }

 if(IsKeyPressed(KEY_Q)){
  if(plane1)
  velocity.y += 20;

  if(plane2)
  velocity2.y += 20;

  if(plane3)
  velocity3.y += 20;
  playSound = true;  
 }

 if(IsKeyPressed(KEY_E)){
  if(plane1)
  velocity.y -= 20;

  if(plane2)
  velocity2.y -= 20;

  if(plane3)
  velocity3.y -= 20;
  
  playSound = true;  
 }

 if(IsKeyPressed(KEY_SPACE)){
  if(plane1)
  velocity = raylib::Vector3{0, 0, 0};

  if(plane2)
  velocity2 = raylib::Vector3{0, 0, 0};

  if(plane3)
  velocity3 = raylib::Vector3{0, 0, 0};
 }

 position = position + velocity*window.GetFrameTime();
 position2 = position2 + velocity2*window.GetFrameTime();
 position3 = position3 + velocity3*window.GetFrameTime(); 
 
  

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
    raylib::Transform plane_transform = raylib::Transform::Identity();
    plane_transform = plane_transform * raylib::Transform(modelPolyP.transform).Translate(0,0,0);

    raylib::Transform plane_transform2 = raylib::Transform::Identity();
    plane_transform2 = plane_transform2 * raylib::Transform(modelPolyP.transform).Translate(100,0,0);

    raylib::Transform plane_transform3 = raylib::Transform::Identity();
    plane_transform3 = plane_transform3 * raylib::Transform(modelPolyP.transform).Translate(-100,0,0);

    raylib::Transform transform_igloo = raylib::Transform::Identity();
    raylib::Transform transform_tree = raylib::Transform::Identity();
    
    //Place the igloo in the scene
    transform_igloo = transform_igloo * raylib::Transform(modelIgloo.transform).Scale(40, 40, 40);
    transform_igloo = transform_igloo * raylib::Transform(modelIgloo.transform).RotateY(raylib::Degree(270));
    transform_igloo = transform_igloo * raylib::Transform(modelIgloo.transform).Translate(0, 0, 300);

    //Place the trees in the scene
    transform_tree = transform_tree * raylib::Transform(modelTree.transform).RotateX(raylib::Degree(270));
    transform_tree = transform_tree * raylib::Transform(modelTree.transform).Translate(300, 0, 200);
    

    plane_transform = plane_transform *  raylib::Transform(modelPolyP.transform).Translate(position.x, position.y, position.z);
    plane_transform2 = plane_transform2 *  raylib::Transform(modelPolyP.transform).Translate(position2.x, position2.y, position2.z);
    plane_transform3 = plane_transform3 *  raylib::Transform(modelPolyP.transform).Translate(position3.x, position3.y, position3.z);

    DrawBoundedModel(modelIgloo, [transform_igloo](raylib::Transform t) -> raylib::Transform{
        return t * transform_igloo;
    }, false);

    DrawBoundedModel(modelTree, [transform_tree](raylib::Transform t) -> raylib::Transform{
        return t * transform_tree; 
    }, false);

    transform_tree = raylib::Transform::Identity();
    transform_tree = transform_tree * raylib::Transform(modelTree.transform).RotateX(raylib::Degree(270));
    transform_tree = transform_tree * raylib::Transform(modelTree.transform).Translate(150, 0, 350);

    DrawBoundedModel(modelTree, [transform_tree](raylib::Transform t) -> raylib::Transform{
        return t * transform_tree; 
    }, false);

    transform_tree = raylib::Transform::Identity();
    transform_tree = transform_tree * raylib::Transform(modelTree.transform).RotateX(raylib::Degree(270));
    transform_tree = transform_tree * raylib::Transform(modelTree.transform).Translate(-200, 0, 200);

    DrawBoundedModel(modelTree, [transform_tree](raylib::Transform t) -> raylib::Transform{
        return t * transform_tree; 
    }, false);

    DrawBoundedModel(modelPolyP, [plane_transform, heading](raylib::Transform t) -> raylib::Transform{
        return t * plane_transform.RotateY(heading); 
    }, plane1);

    DrawBoundedModel(modelPolyP, [plane_transform2](raylib::Transform t) -> raylib::Transform{
        return t * plane_transform2; 
    }, plane2);

    DrawBoundedModel(modelPolyP, [plane_transform3](raylib::Transform t) -> raylib::Transform{
        return t * plane_transform3; 
    }, plane3);


    ground.Draw({});



 }
 camera.EndMode();
   

window.EndDrawing();

}











    return 0;
}