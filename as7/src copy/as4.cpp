//Author: Aaron Whitaker
//Assignment: AS4
//Date: 3-3-2024


#include <iostream>

#include "raylib-cpp.hpp"
#include "rlgl.h"
#include "skybox.hpp"

template<typename T>
concept Transformer = requires(T t, raylib::Transform m) {
   {t.operator()(m) } -> std::convertible_to<raylib::Transform>;
};

//Takes a model and draws it according to its transformation.
void DrawBoundedModel(raylib::Model& model, Transformer auto transformer, bool drawBox){
    raylib::Transform backupTransform = model.transform;
    model.transform = transformer(model.transform);
    model.Draw({});
    if(drawBox) model.GetTransformedBoundingBox().Draw();

    model.transform = backupTransform;
}

//Takes two models and checks to see if theirs translated meshes overlap based on their bounding boxes.
bool CheckCollisions(raylib::Model& model, raylib::Model& model2, Transformer auto transformer, Transformer auto transformer2){
    raylib::Transform backupTransform = model.transform;
    raylib::Transform backupTransform2 = model2.transform;

    model.transform = transformer(model.transform);
    model2.transform = transformer2(model2.transform); 
    //model.Draw({});
    //if(drawBox) model.GetTransformedBoundingBox().Draw();

    bool collision = false;

    if(CheckCollisionBoxes(model.GetTransformedBoundingBox(), model2.GetTransformedBoundingBox())){
      std::cout << "Collision Detected" << std::endl;
      collision = true;
    }

    

    model.transform = backupTransform;
    model2.transform = backupTransform2;

    

      return collision;
}

int main(){

//Setup for scene; i.e. window, model loading, etc.
raylib::Window window(1000, 700, "CS381 - Assignment 4");
raylib::Text text;
int windHeight = 0, windWidth = 0;
bool playSound = false;

InitAudioDevice();

raylib::Model modelPolyP = LoadModel("../meshes/PolyPlane.glb");
modelPolyP.transform = raylib::Transform(modelPolyP.transform).Scale(1.5, 1.5, 1.5);

raylib::Model modelSpaceShip = LoadModel("../meshes/drone.glb");
modelSpaceShip.transform = raylib::Transform(modelSpaceShip.transform).Scale(2, 2, 2);

raylib::Model modelMeteor = LoadModel("../meshes/meteorite.glb");
modelMeteor.transform = raylib::Transform(modelMeteor.transform).Scale(3, 3, 3);


raylib::Model modelIgloo = LoadModel("../meshes/low_poly_igloo.glb");

raylib::Model modelTree = LoadModel("../meshes/12150_Christmas_Tree_V2_L2.obj");
raylib::Texture tree("../textures/tree.jpg");
 tree.SetFilter(TEXTURE_FILTER_BILINEAR);
 tree.SetWrap(TEXTURE_WRAP_REPEAT);
 modelTree.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = tree;

Music planeWav = LoadMusicStream("../sounds/airplane-fly-by-02.wav");

raylib::Camera camera(
        raylib::Vector3(0, 0, -350),
        raylib::Vector3(0, 0, 0),
        raylib::Vector3::Up(),
        45,
        CAMERA_PERSPECTIVE
    );

cs381::SkyBox skybox("textures/deepbluespace.png");

 auto mesh = raylib::Mesh::Plane(10'000, 10'000, 50, 50, 25);
 raylib::Model ground = ((raylib::Mesh*)&mesh)->LoadModelFrom();
 raylib::Texture grass("textures/ice2.jpg");
 grass.SetFilter(TEXTURE_FILTER_BILINEAR);
 grass.SetWrap(TEXTURE_WRAP_REPEAT);
 ground.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = grass;


 float speed = 10, targetSpeed = 10;
 float acceleration = 240;
 float rotation = 0;

 //Creation of vectors to hold positional and movement data.
 raylib::Vector3 position = {0, 0, 0};
 raylib::Vector3 position2 = {0, 0, 0};
 raylib::Vector3 position3 = {0, 0, 0};

 raylib::Vector3 boost = {0, 0, 0};
 raylib::Vector3 targetBoost = {0, 0, 0};

 raylib::Vector3 velocity = {0, 0, 0};
 raylib::Vector3 velocity2 = {0, 0, 0};
 raylib::Vector3 velocity3 = {0, 0, 0};

 raylib::Vector3 shipVelocity = {-50, 0, 0};
 raylib::Vector3 cameraVelocity = {-50, 0, 0};

 raylib::Vector3 shipPosition = {0, 0, 0};


 //Bools for state machine
 int shipDamage = 0;
 bool shipDestroyed = false;
 bool victoryFlag = false;
 bool drawBoost = false, startGame = false;


 SetTargetFPS(60);
 



//Begin running window
while(!window.ShouldClose()) { 
//If the user has not pressed enter on the start screen, main screen will loop, otherwise game will run.
if(!startGame){
  window.BeginDrawing();
  {
    text.Draw("To initiate liftoff, press enter", (window.GetWidth()/2), (window.GetHeight()/2), 20, raylib::Color::White());
  }
  if(IsKeyPressed(KEY_ENTER)){
    startGame = true;
  }
  window.EndDrawing();
}

else{

if(!shipDestroyed && !victoryFlag){

//Scan keyboard for corressponding inputs for movement of ship and camera.

if(IsKeyDown(KEY_W)){
  //We use GetWorldToScreen to ensure that our ship does not leave the area the camera is currently viewing.
  if(GetWorldToScreen(shipPosition, camera).y < 0){
  

  }
  else{
  shipPosition.y += 2;
  }
 }

if(IsKeyDown(KEY_UP)){
  camera.position.y += 2; 
  
 } 

 if(IsKeyDown(KEY_S)){
  if(GetWorldToScreen(shipPosition, camera).y > window.GetHeight()){
  

  }
  else{
   shipPosition.y -= 2;
  }
  }


 

 if(IsKeyDown(KEY_DOWN)){
  
  
  camera.position.y -= 2;
  
  
 }

 if(IsKeyDown(KEY_A)){
  
  if(GetWorldToScreen(shipPosition, camera).x < 0){
  

  }
  else{
    shipPosition.x += 3;
  }
  }

 

 if(IsKeyDown(KEY_D)){

  if(GetWorldToScreen(shipPosition, camera).x > window.GetWidth()){
  

  }

  else{
   shipPosition.x -= 3;  
   if(IsKeyPressed(KEY_SPACE)){
   targetBoost.x -= 60; 
   }
  }
 }


 if(IsKeyPressed(KEY_Q)){
  
 }

 if(IsKeyPressed(KEY_E)){

  
  

 }

 
 

 rotation += 0.5 * window.GetFrameTime();

 //Rotation for meteors.
 if(rotation > 360){
  rotation = 0;
 }

 //Boost smoother for animation purposes.
 if(boost.x > targetBoost.x){
  shipPosition.x -= acceleration * window.GetFrameTime();
  boost.x -= acceleration * window.GetFrameTime();
  drawBoost = true;
 }

 //Update position per tick.
 position = position + velocity*window.GetFrameTime();
 position2 = position2 + velocity2*window.GetFrameTime();
 position3 = position3 + velocity3*window.GetFrameTime(); 

 shipPosition = shipPosition + shipVelocity*window.GetFrameTime();



 //Steady scrolling camera
 camera.position = (raylib::Vector3)camera.position + cameraVelocity*window.GetFrameTime();
 camera.target = (raylib::Vector3)camera.target + cameraVelocity*window.GetFrameTime();


 //Create meteor positions
 raylib::Vector3 meteorPos1 = {-300, 0, 0}; 
 raylib::Vector3 meteorPos2 = {-600, 50, 0}; 
 raylib::Vector3 meteorPos3 = {-900, -20, 0}; 
 raylib::Vector3 meteorPos4 = {-1000, 70, 0}; 
 raylib::Vector3 meteorPos5 = {-1200, -30, 0}; 
 raylib::Vector3 meteorPos6 = {-1400, 60, 0}; 
 raylib::Vector3 meteorPos7 = {-1400, -60, 0}; 
 raylib::Vector3 meteorPos8 = {-1500, -30, 0};
 raylib::Vector3 meteorPos9 = {-1650, 10, 0};
 raylib::Vector3 meteorPos10 = {-1770, 40, 0};
 raylib::Vector3 meteorPos11 = {-1850, 90, 0};
 raylib::Vector3 meteorPos12 = {-1950, 0, 0};
 raylib::Vector3 meteorPos13 = {-2050, 95, 0};
 raylib::Vector3 meteorPos14 = {-1980, -50, 0};

 
 
  
//Begin drawing the scene
window.BeginDrawing();

 windWidth = window.GetWidth(); 
 windHeight = window.GetHeight();

 window.ClearBackground(BLACK);
 
 //Begin displaying the camera
 camera.BeginMode();
 {

    
    
    if(playSound == true){
    //std::cout << "Playing sound" << std::endl;
    
    
    playSound = false;
    }
    //std::cout << IsSoundPlaying(planeMp3) << std::endl;

    skybox.Draw();


    //Draw ships and meteors. If a meteor is not currently within the view space of the camera, we don't draw it to 
    //help frame rate.
    raylib::Transform ship_transform = raylib::Transform::Identity();
    ship_transform = ship_transform * raylib::Transform(modelSpaceShip.transform).Translate(shipPosition.x, shipPosition.y, shipPosition.z);
    raylib::Transform meteor_transform = raylib::Transform::Identity();


    DrawBoundedModel(modelSpaceShip, [ship_transform](raylib::Transform t) -> raylib::Transform{
        return t * ship_transform.RotateX(raylib::Degree(180));
    }, false);

    if(drawBoost){
      DrawSphere(((raylib::Vector3){(shipPosition.x  + GetRandomValue(10, 80)), (shipPosition.y + GetRandomValue(5, 10)), shipPosition.z}), GetRandomValue(5, 15), raylib::Color::Blue());
      drawBoost = false;
    }


    if(GetWorldToScreen(meteorPos1, camera).x > 0 && GetWorldToScreen(meteorPos1, camera).x < window.GetWidth() + 50){

    meteor_transform = raylib::Transform::Identity();
    meteor_transform = meteor_transform * raylib::Transform(modelMeteor.transform).Translate(meteorPos1);

     DrawBoundedModel(modelMeteor, [meteor_transform, rotation](raylib::Transform t) -> raylib::Transform{
         return t * meteor_transform.RotateX(rotation).RotateY(rotation);
     }, false);

     

     if(CheckCollisions(modelSpaceShip, modelMeteor, [ship_transform](raylib::Transform t) -> raylib::Transform{
        return t * ship_transform.RotateX(raylib::Degree(180));
     }, [meteor_transform, rotation](raylib::Transform t) -> raylib::Transform{
        return t * meteor_transform.RotateX(rotation).RotateY(rotation);
     })){
      DrawSphere(((raylib::Vector3){(shipPosition.x  + GetRandomValue(-13, 13)), (shipPosition.y + GetRandomValue(-13, 13)), shipPosition.z}), GetRandomValue(5, 15), raylib::Color::Red());
      shipDamage += 1;
     }
    }

    //std::cout << window.GetWidth() << std::endl;
  


    
    if(GetWorldToScreen(meteorPos2, camera).x > 0 && GetWorldToScreen(meteorPos2, camera).x < window.GetWidth() + 50){
    meteor_transform = raylib::Transform::Identity();
    meteor_transform = meteor_transform * raylib::Transform(modelMeteor.transform).Translate(meteorPos2);

    DrawBoundedModel(modelMeteor, [meteor_transform, rotation](raylib::Transform t) -> raylib::Transform{
        return t * meteor_transform.RotateX(rotation).RotateZ(rotation);
    }, false);

    if(CheckCollisions(modelSpaceShip, modelMeteor, [ship_transform](raylib::Transform t) -> raylib::Transform{
        return t * ship_transform.RotateX(raylib::Degree(180));
    }, [meteor_transform, rotation](raylib::Transform t) -> raylib::Transform{
        return t * meteor_transform.RotateX(rotation).RotateZ(rotation);
    })){
      DrawSphere(((raylib::Vector3){(shipPosition.x  + GetRandomValue(-13, 13)), (shipPosition.y + GetRandomValue(-13, 13)), shipPosition.z}), GetRandomValue(5, 15), raylib::Color::Red());
      shipDamage += 1;
    }
    }


    

    if(GetWorldToScreen(meteorPos3, camera).x > 0 && GetWorldToScreen(meteorPos3, camera).x < window.GetWidth() + 50){
    meteor_transform = raylib::Transform::Identity();
    meteor_transform = meteor_transform * raylib::Transform(modelMeteor.transform).Translate(meteorPos3);

    DrawBoundedModel(modelMeteor, [meteor_transform, rotation](raylib::Transform t) -> raylib::Transform{
        return t * meteor_transform.RotateY(rotation).RotateZ(rotation);
    }, false);

    if(CheckCollisions(modelSpaceShip, modelMeteor, [ship_transform](raylib::Transform t) -> raylib::Transform{
        return t * ship_transform.RotateX(raylib::Degree(180));
    }, [meteor_transform, rotation](raylib::Transform t) -> raylib::Transform{
        return t * meteor_transform.RotateY(rotation).RotateZ(rotation);
    })){
      DrawSphere(((raylib::Vector3){(shipPosition.x  + GetRandomValue(-13, 13)), (shipPosition.y + GetRandomValue(-13, 13)), shipPosition.z}), GetRandomValue(5, 15), raylib::Color::Red());
      shipDamage += 1;
    }
    }


    if(GetWorldToScreen(meteorPos4, camera).x > 0 && GetWorldToScreen(meteorPos4, camera).x < window.GetWidth() + 50){
    meteor_transform = raylib::Transform::Identity();
    meteor_transform = meteor_transform * raylib::Transform(modelMeteor.transform).Translate(meteorPos4);

    DrawBoundedModel(modelMeteor, [meteor_transform, rotation](raylib::Transform t) -> raylib::Transform{
        return t * meteor_transform.RotateX(rotation).RotateY(rotation).RotateZ(rotation);
    }, false);

    if(CheckCollisions(modelSpaceShip, modelMeteor, [ship_transform](raylib::Transform t) -> raylib::Transform{
        return t * ship_transform.RotateX(raylib::Degree(180));
    }, [meteor_transform, rotation](raylib::Transform t) -> raylib::Transform{
        return t * meteor_transform.RotateX(rotation).RotateY(rotation).RotateZ(rotation);
    })){
      DrawSphere(((raylib::Vector3){(shipPosition.x  + GetRandomValue(-13, 13)), (shipPosition.y + GetRandomValue(-13, 13)), shipPosition.z}), GetRandomValue(5, 15), raylib::Color::Red());
      shipDamage += 1;
    }
    }

    if(GetWorldToScreen(meteorPos5, camera).x > 0 && GetWorldToScreen(meteorPos5, camera).x < window.GetWidth() + 50){
    meteor_transform = raylib::Transform::Identity();
    meteor_transform = meteor_transform * raylib::Transform(modelMeteor.transform).Translate(meteorPos5);

    DrawBoundedModel(modelMeteor, [meteor_transform, rotation](raylib::Transform t) -> raylib::Transform{
        return t * meteor_transform.RotateY(rotation).RotateZ(rotation);
    }, false);

    if(CheckCollisions(modelSpaceShip, modelMeteor, [ship_transform](raylib::Transform t) -> raylib::Transform{
        return t * ship_transform.RotateX(raylib::Degree(180));
    }, [meteor_transform, rotation](raylib::Transform t) -> raylib::Transform{
        return t * meteor_transform.RotateY(rotation).RotateZ(rotation);
    })){
      DrawSphere(((raylib::Vector3){(shipPosition.x  + GetRandomValue(-13, 13)), (shipPosition.y + GetRandomValue(-13, 13)), shipPosition.z}), GetRandomValue(5, 15), raylib::Color::Red());
      shipDamage += 1;
    }
    }

    if(GetWorldToScreen(meteorPos6, camera).x > 0 && GetWorldToScreen(meteorPos6, camera).x < window.GetWidth() + 50){
    meteor_transform = raylib::Transform::Identity();
    meteor_transform = meteor_transform * raylib::Transform(modelMeteor.transform).Translate(meteorPos6);

    DrawBoundedModel(modelMeteor, [meteor_transform, rotation](raylib::Transform t) -> raylib::Transform{
        return t * meteor_transform.RotateX(rotation + raylib::Degree(30)).RotateY(rotation);
    }, false);

    if(CheckCollisions(modelSpaceShip, modelMeteor, [ship_transform](raylib::Transform t) -> raylib::Transform{
        return t * ship_transform.RotateX(raylib::Degree(180));
    }, [meteor_transform, rotation](raylib::Transform t) -> raylib::Transform{
        return t * meteor_transform.RotateX(rotation + raylib::Degree(30)).RotateY(rotation);
    })){
      DrawSphere(((raylib::Vector3){(shipPosition.x  + GetRandomValue(-13, 13)), (shipPosition.y + GetRandomValue(-13, 13)), shipPosition.z}), GetRandomValue(5, 15), raylib::Color::Red());
      shipDamage += 1;
    }
    }

    if(GetWorldToScreen(meteorPos7, camera).x > 0 && GetWorldToScreen(meteorPos7, camera).x < window.GetWidth() + 50){
    meteor_transform = raylib::Transform::Identity();
    meteor_transform = meteor_transform * raylib::Transform(modelMeteor.transform).Translate(meteorPos7);

    DrawBoundedModel(modelMeteor, [meteor_transform, rotation](raylib::Transform t) -> raylib::Transform{
        return t * meteor_transform.RotateY(rotation).RotateZ(rotation + raylib::Degree(45));
    }, false);

    if(CheckCollisions(modelSpaceShip, modelMeteor, [ship_transform](raylib::Transform t) -> raylib::Transform{
        return t * ship_transform.RotateX(raylib::Degree(180));
    }, [meteor_transform, rotation](raylib::Transform t) -> raylib::Transform{
        return t * meteor_transform.RotateY(rotation).RotateZ(rotation + raylib::Degree(45));
    })){
      DrawSphere(((raylib::Vector3){(shipPosition.x  + GetRandomValue(-13, 13)), (shipPosition.y + GetRandomValue(-13, 13)), shipPosition.z}), GetRandomValue(5, 15), raylib::Color::Red());
      shipDamage += 1;
    }
    }

    if(GetWorldToScreen(meteorPos8, camera).x > 0 && GetWorldToScreen(meteorPos8, camera).x < window.GetWidth() + 50){
    meteor_transform = raylib::Transform::Identity();
    meteor_transform = meteor_transform * raylib::Transform(modelMeteor.transform).Translate(meteorPos8);

    DrawBoundedModel(modelMeteor, [meteor_transform, rotation](raylib::Transform t) -> raylib::Transform{
        return t * meteor_transform.RotateY(rotation + raylib::Degree(50)).RotateZ(rotation);
    }, false);

    if(CheckCollisions(modelSpaceShip, modelMeteor, [ship_transform](raylib::Transform t) -> raylib::Transform{
        return t * ship_transform.RotateX(raylib::Degree(180));
    }, [meteor_transform, rotation](raylib::Transform t) -> raylib::Transform{
        return t * meteor_transform.RotateY(rotation + raylib::Degree(50)).RotateZ(rotation);
    })){
      DrawSphere(((raylib::Vector3){(shipPosition.x  + GetRandomValue(-13, 13)), (shipPosition.y + GetRandomValue(-13, 13)), shipPosition.z}), GetRandomValue(5, 15), raylib::Color::Red());
      shipDamage += 1;
    }
    }

    if(GetWorldToScreen(meteorPos9, camera).x > 0 && GetWorldToScreen(meteorPos9, camera).x < window.GetWidth() + 50){
    meteor_transform = raylib::Transform::Identity();
    meteor_transform = meteor_transform * raylib::Transform(modelMeteor.transform).Translate(meteorPos9);

    DrawBoundedModel(modelMeteor, [meteor_transform, rotation](raylib::Transform t) -> raylib::Transform{
        return t * meteor_transform.Scale(1.5,1.5,1.5).RotateX(rotation).RotateZ(rotation + raylib::Degree(20));
    }, false);

    if(CheckCollisions(modelSpaceShip, modelMeteor, [ship_transform](raylib::Transform t) -> raylib::Transform{
        return t * ship_transform.RotateX(raylib::Degree(180));
    }, [meteor_transform, rotation](raylib::Transform t) -> raylib::Transform{
        return t * meteor_transform.Scale(1.5,1.5,1.5).RotateX(rotation).RotateZ(rotation + raylib::Degree(20));
    })){
      DrawSphere(((raylib::Vector3){(shipPosition.x  + GetRandomValue(-13, 13)), (shipPosition.y + GetRandomValue(-13, 13)), shipPosition.z}), GetRandomValue(5, 15), raylib::Color::Red());
      shipDamage += 1;
    }
    }

    if(GetWorldToScreen(meteorPos10, camera).x > 0 && GetWorldToScreen(meteorPos10, camera).x < window.GetWidth() + 50){
    meteor_transform = raylib::Transform::Identity();
    meteor_transform = meteor_transform * raylib::Transform(modelMeteor.transform).Translate(meteorPos10);

    DrawBoundedModel(modelMeteor, [meteor_transform, rotation](raylib::Transform t) -> raylib::Transform{
        return t * meteor_transform.RotateZ(rotation);
    }, false);

    if(CheckCollisions(modelSpaceShip, modelMeteor, [ship_transform](raylib::Transform t) -> raylib::Transform{
        return t * ship_transform.RotateX(raylib::Degree(180));
    }, [meteor_transform, rotation](raylib::Transform t) -> raylib::Transform{
        return t * meteor_transform.RotateZ(rotation);
    })){
      DrawSphere(((raylib::Vector3){(shipPosition.x  + GetRandomValue(-13, 13)), (shipPosition.y + GetRandomValue(-13, 13)), shipPosition.z}), GetRandomValue(5, 15), raylib::Color::Red());
      shipDamage += 1;
    }
    }

    if(GetWorldToScreen(meteorPos11, camera).x > 0 && GetWorldToScreen(meteorPos11, camera).x < window.GetWidth() + 50){
    meteor_transform = raylib::Transform::Identity();
    meteor_transform = meteor_transform * raylib::Transform(modelMeteor.transform).Translate(meteorPos11);

    DrawBoundedModel(modelMeteor, [meteor_transform, rotation](raylib::Transform t) -> raylib::Transform{
        return t * meteor_transform.RotateX(rotation + raylib::Degree(30)).RotateY(rotation);
    }, false);

    if(CheckCollisions(modelSpaceShip, modelMeteor, [ship_transform](raylib::Transform t) -> raylib::Transform{
        return t * ship_transform.RotateX(raylib::Degree(180));
    }, [meteor_transform, rotation](raylib::Transform t) -> raylib::Transform{
        return t * meteor_transform.RotateX(rotation + raylib::Degree(30)).RotateY(rotation);
    })){
      DrawSphere(((raylib::Vector3){(shipPosition.x  + GetRandomValue(-13, 13)), (shipPosition.y + GetRandomValue(-13, 13)), shipPosition.z}), GetRandomValue(5, 15), raylib::Color::Red());
      shipDamage += 1;
    }
    }

    if(GetWorldToScreen(meteorPos12, camera).x > 0 && GetWorldToScreen(meteorPos12, camera).x < window.GetWidth() + 50){
    meteor_transform = raylib::Transform::Identity();
    meteor_transform = meteor_transform * raylib::Transform(modelMeteor.transform).Translate(meteorPos12);

    DrawBoundedModel(modelMeteor, [meteor_transform, rotation](raylib::Transform t) -> raylib::Transform{
        return t * meteor_transform.RotateY(rotation).RotateZ(rotation);
    }, false);

    if(CheckCollisions(modelSpaceShip, modelMeteor, [ship_transform](raylib::Transform t) -> raylib::Transform{
        return t * ship_transform.RotateX(raylib::Degree(180));
    }, [meteor_transform, rotation](raylib::Transform t) -> raylib::Transform{
        return t * meteor_transform.RotateY(rotation).RotateZ(rotation);
    })){
      DrawSphere(((raylib::Vector3){(shipPosition.x  + GetRandomValue(-13, 13)), (shipPosition.y + GetRandomValue(-13, 13)), shipPosition.z}), GetRandomValue(5, 15), raylib::Color::Red());
      shipDamage += 1;
    }
    }

    if(GetWorldToScreen(meteorPos13, camera).x > 0 && GetWorldToScreen(meteorPos13, camera).x < window.GetWidth() + 50){
    meteor_transform = raylib::Transform::Identity();
    meteor_transform = meteor_transform * raylib::Transform(modelMeteor.transform).Translate(meteorPos13);

    DrawBoundedModel(modelMeteor, [meteor_transform, rotation](raylib::Transform t) -> raylib::Transform{
        return t * meteor_transform.Scale(2, 2, 2).RotateX(rotation).RotateY(rotation);
    }, false);

    if(CheckCollisions(modelSpaceShip, modelMeteor, [ship_transform](raylib::Transform t) -> raylib::Transform{
        return t * ship_transform.RotateX(raylib::Degree(180));
    }, [meteor_transform, rotation](raylib::Transform t) -> raylib::Transform{
        return t * meteor_transform.Scale(1.6, 1.6, 1.6).RotateX(rotation).RotateY(rotation);
    })){
      DrawSphere(((raylib::Vector3){(shipPosition.x  + GetRandomValue(-13, 13)), (shipPosition.y + GetRandomValue(-13, 13)), shipPosition.z}), GetRandomValue(5, 15), raylib::Color::Red());
      shipDamage += 1;
    }
    }

    if(GetWorldToScreen(meteorPos14, camera).x > 0 && GetWorldToScreen(meteorPos14, camera).x < window.GetWidth() + 50){
    meteor_transform = raylib::Transform::Identity();
    meteor_transform = meteor_transform * raylib::Transform(modelMeteor.transform).Translate(meteorPos14);

    DrawBoundedModel(modelMeteor, [meteor_transform, rotation](raylib::Transform t) -> raylib::Transform{
        return t * meteor_transform.Scale(1.3, 1.3, 1.3).RotateY(rotation).RotateZ(rotation);
    }, false);

    if(CheckCollisions(modelSpaceShip, modelMeteor, [ship_transform](raylib::Transform t) -> raylib::Transform{
        return t * ship_transform.RotateX(raylib::Degree(180));
    }, [meteor_transform, rotation](raylib::Transform t) -> raylib::Transform{
        return t * meteor_transform.Scale(1.3, 1.3, 1.3).RotateY(rotation).RotateZ(rotation);
    })){
      DrawSphere(((raylib::Vector3){(shipPosition.x  + GetRandomValue(-13, 13)), (shipPosition.y + GetRandomValue(-13, 13)), shipPosition.z}), GetRandomValue(5, 15), raylib::Color::Red());
      shipDamage += 1;
    }
    }

    //Draw goal
    DrawCubeWires(raylib::Vector3{-2600, 0, 0}, 1000.f, 700.f, 30.f, raylib::Color::White());

    
    

    /*if(CheckCollisionBoxes((modelSpaceShip * (ship_transform.RotateX(raylib::Degree(180)))).GetTransformedBoundingBox(), GetModelBoundingBox(modelMeteor))){
      std::cout << "Models have collided" << std::endl;
    }*/


    //ground.Draw({});
    std::cout << shipPosition.x << std::endl;
    if(shipPosition.x <= -2100){
      victoryFlag = true;
    }


 }
 camera.EndMode();


 //HUD/State machine handler
 text.Draw("Ship Health: ", (30), (70), 40, raylib::Color::White());
 if(shipDamage <= 10){
  text.Draw("Healthy", (30), (110), 40, raylib::Color::Green());
 }
 else if(shipDamage > 10 && shipDamage <= 30){
  text.Draw("Damaged", (30), (110), 40, raylib::Color::Yellow());
 }
 else if(shipDamage > 30 && shipDamage <= 40){
  text.Draw("WARNING", (30), (110), 40, raylib::Color::Orange());
 }
 else{
  text.Draw("DESTROYED", (30), (110), 40, raylib::Color::Red());
  shipDestroyed = true;
 }

 if(victoryFlag){
  text.Draw("COMPLETE!", window.GetWidth() / 2, window.GetHeight() / 2, 40, raylib::Color::Green());
 }
}
   

window.EndDrawing();

}
}











    return 0;
}