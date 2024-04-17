//Author: Aaron Whitaker
//Assignment: AS9
//Date: 4-9-2024


#include <iostream>
#include "raylib-cpp.hpp"
#include "rlgl.h"
#include <vector>
#include <memory>
#include <optional>
#include <concepts>
//#include "delegate.hpp"
#include <BufferedInput.hpp>
#include <ranges>
#include <random>
#include "ECS.hpp"
using scene = cs381::Scene<cs381::SkiplistComponentStorage>;

//#define PI 3.142857
//#include "VolumeControl.h"

// #include "inputs.hpp"
#include "skybox.hpp"

//void Draw(cs381::Scene& scene);


template<typename T> 
concept Transformer = requires(T t, raylib::Transform m) {
    { t.operator()(m) } -> std::convertible_to<raylib::Transform>;
};

float DegreeToRadian(raylib::Degree angle){
    return ((float)angle)*(std::numbers::pi / 180.0f);
}

struct Particle {
    Vector2 velocity;
    Vector2 position;
};

void FillParticleVector(std::vector<Particle>& particle, Vector2 shipPos){
    for(int i = 0; i < 100; i++){
        float speed = (float)GetRandomValue(50, 300);
        std::mt19937 rng;
        rng.seed(std::random_device()());
        std::uniform_real_distribution<float> dist(0.0f, 2.0f * PI);
        float direction = dist(rng);

        particle.push_back(
            Particle{
                Vector2{speed * cos(direction), speed* sin(direction)},
                Vector2{(float)shipPos.x , (float)shipPos.y}
            });
    }
}

float Distance(float x, float y){
    return (float) sqrt((x*x) + (y*y));
}

void DrawBeam(raylib::Vector3 shipPos, raylib::Vector3 endPos, raylib::Color color){
    DrawCapsule(shipPos, endPos, 6.0f, 2, 2, Fade(color, 0.6f));
}

bool DrawParticles(std::vector<Particle>& particles, float dt, Vector2 shipPos){

    

    for(int i = 0; i < particles.size(); i++){
        //Particle particle = particles[i];
        DrawCircleGradient(particles[i].position.x, particles[i].position.y, 8.0f, Fade(RED, 0.6f), Fade(RED, 0.0f));
        DrawCircleV(particles[i].position, 2.0f, RED);
        particles[i].position.x += particles[i].velocity.x * dt;
        particles[i].position.y += particles[i].velocity.y * dt;

        if(Distance(particles[i].position.x - shipPos.x, particles[i].position.y - shipPos.y) > 200.0f){
            particles.erase(particles.begin() + i);
        } 
    }

    if(particles.empty()){
        return false;
    }

    return true;
}

// bool CheckCollisions(raylib::Model& model, raylib::Model& model2, Transformer auto transformer, Transformer auto transformer2){
//     raylib::Transform backupTransform = model.transform;
//     raylib::Transform backupTransform2 = model2.transform;

//     model.transform = transformer(model.transform);
//     model2.transform = transformer2(model2.transform); 
//     //model.Draw({});
//     //if(drawBox) model.GetTransformedBoundingBox().Draw();

//     bool collision = false;

//     if(CheckCollisionBoxes(model.GetTransformedBoundingBox(), model2.GetTransformedBoundingBox())){
//       std::cout << "Collision Detected" << std::endl;
//       collision = true;
//     }

//     model.transform = backupTransform;
//     model2.transform = backupTransform2;

    

//     return collision;
// }





struct Rendering { 

    bool invisible = false;
    bool drawParticles = false;
    raylib::Model* model;
    raylib::Camera* camera;
    raylib::Window* window;
    raylib::Sound* ping;
    raylib::Sound* laser;

    std::vector<Particle> particles;
    Vector2 impactPoint = {0, 0};
};

struct Transforming {
    
    raylib::Vector3 velocity = raylib::Vector3::Zero();
    raylib::Vector3 position = {0, 0, 0};
    raylib::Vector3 targetPos = {0, 0, 0};
    raylib::Vector3 scale = {1, 1, 1};
    //float dt;
    

};

struct Physics{

    float speed = 0, targetSpeed = 0;
    float acceleration = 5; 
    bool isMeteor = false;
    raylib::Vector3 shipPos = {0,0,0};

};

struct Physics3D : public Physics{

    raylib::Quaternion quat; 
    raylib::Degree pitch = 0; 
    raylib::Degree yaw = 0; 
    raylib::Degree roll = 0;

    float targetPitch = 0;
    float targetYaw = 0; 
    float targetRoll = 0;

};

struct Input {
    raylib::BufferedInput* input;
    bool selected;
};

struct Player {
    raylib::Sound* death;
};



struct Ship {

    float targetBeamPos;
    raylib::Vector3 shipLaserPosition;
    float targetLaserBar;
    float targetHealth;
    float shipHealth;
    bool drawLaser = false;
 
    

};

enum StateMachine {

titleScreen,
mainLevel,
bossFight,
gameOver

};

enum BossPhase {

idle, 
speeding, 
offscreen,
ready

};

void UpdateShipReference(scene& scene){
    raylib::Vector3 shipPos = {0,0,0};
    for(cs381::Entity e = 0; e < scene.entityMasks.size(); e++){
     if(!scene.HasComponent<Physics3D>(e)) continue;
     if(scene.HasComponent<Player>(e)) shipPos = scene.GetComponent<Transforming>(e).position;

     else if(!(scene.HasComponent<Player>(e))) scene.GetComponent<Physics3D>(e).shipPos = shipPos;
    
    }
}

void UpdateHealth(scene& scene, cs381::Entity e){
    if(!(scene.HasComponent<Ship>(e))) return;

    scene.GetComponent<Ship>(e).targetHealth -= 50.0f;
}

// void CheckCollision(Rendering& render){
    
//    

    
// };

void Draw(scene& scene, StateMachine& state, float dt){

 cs381::Entity player = 0;
 for(cs381::Entity e = 0; e < scene.entityMasks.size(); e++){
  if(!scene.HasComponent<Rendering>(e)) continue;
  if(!scene.HasComponent<Transforming>(e)) continue;

  if(scene.HasComponent<Player>(e)) player = e;

  auto& transforming = scene.GetComponent<Transforming>(e);
  auto& rendering = scene.GetComponent<Rendering>(e);

  if(GetWorldToScreen(transforming.position, *(rendering.camera)).x > -40.0 && GetWorldToScreen(transforming.position, (*rendering.camera)).x < (*rendering.window).GetWidth() + 50 && !(rendering.invisible)){

    auto pos = transforming.position; 
    auto scale = transforming.scale;

    

    auto [axis, angle] = scene.HasComponent<Physics3D>(e) ? scene.GetComponent<Physics3D>(e).quat.ToAxisAngle() : std::pair<raylib::Vector3, raylib::Radian>(raylib::Vector3{0,0,0}, 0);

    

    auto transformer3D = [pos, scale, axis, angle](raylib::Transform transform){
        return transform.Translate(pos).Rotate(axis, angle).Scale(scale.x, scale.y, scale.z);
    };
  

    raylib::Transform backupTransform = rendering.model->transform;
    rendering.model->transform = transformer3D(backupTransform);

    //CHECK TO SEE IF METEOR COLLIDED OR DESTROYED
    
    if(!(scene.HasComponent<Ship>(e)) && (scene.HasComponent<Physics3D>(e)) && state != StateMachine::gameOver){
        auto& physics = scene.GetComponent<Physics3D>(e);
        
         if(CheckCollisionBoxSphere(rendering.model->GetTransformedBoundingBox(), physics.shipPos, 20.0f)){
             if(rendering.particles.empty()){
                UpdateHealth(scene, player);
                rendering.impactPoint = Vector2{physics.shipPos.x, physics.shipPos.y};
                FillParticleVector(rendering.particles, rendering.impactPoint);
                rendering.drawParticles = DrawParticles(rendering.particles, dt, rendering.impactPoint);
                (*(rendering.ping)).Play();
             }
         }
         //ONLY WORKS IF PLAYER IS INITIALIZED AS FIRST ENTITY
        
        if(scene.HasComponent<Ship>(player)){
          auto& ship = scene.GetComponent<Ship>(player);
          if(CheckCollisionBoxSphere(rendering.model->GetTransformedBoundingBox(), ship.shipLaserPosition, 20.0f) && ship.drawLaser){
             //std::cout << "laser collided" << std::endl;
             rendering.invisible = true;
             rendering.impactPoint = Vector2{pos.x, pos.y};
             FillParticleVector(rendering.particles, rendering.impactPoint);
                
             rendering.drawParticles = DrawParticles(rendering.particles, dt, rendering.impactPoint);

             (*(rendering.ping)).Play();
          }
        }
    }
      

    //if(!(rendering.invisible)) rendering.model->Draw({});
     rendering.model->Draw({});
    //if(scene.HasComponent<Player>(e)) DrawCircle(pos.x, pos.y, 20.0f, raylib::WHITE);
  
    rendering.model->transform = backupTransform;
 
 }

    if(rendering.drawParticles) rendering.drawParticles = DrawParticles(rendering.particles, dt, rendering.impactPoint);
 
 }
}

void ApplyPhysics(scene& scene, float dt){

for(cs381::Entity e = 0; e < scene.entityMasks.size(); e++){
  if(!scene.HasComponent<Physics3D>(e)) continue;
  if(!scene.HasComponent<Transforming>(e)) continue;

  
  auto& transforming = scene.GetComponent<Transforming>(e);
  auto& physics = scene.GetComponent<Physics3D>(e);
  //transforming.dt = dt;

 if(physics.isMeteor == true){
    //auto& meteor = scene.GetComponent<Meteor>(e);

    physics.pitch += physics.targetPitch * dt;
    physics.yaw += physics.targetYaw * dt;
    physics.roll += physics.targetRoll * dt;

    if(physics.pitch > 360) physics.pitch = 0;
    if(physics.yaw > 360) physics.yaw = 0;
    if(physics.roll > 360) physics.roll = 0;

 }

  raylib::Quaternion target = raylib::Quaternion::FromEuler(DegreeToRadian(physics.roll), DegreeToRadian(physics.yaw), DegreeToRadian(physics.pitch));
  //std::cout << target.x << std::endl;
  physics.quat = physics.quat.Slerp(target, dt*2);


  transforming.position = transforming.position + transforming.velocity*dt;

}

}

void UpdatePlayer(scene& scene, cs381::Entity e, float dt){
    
        if(!scene.HasComponent<Transforming>(e)) return;
        if(!scene.HasComponent<Input>(e)) return;
        if(!scene.HasComponent<Player>(e)) return;
        if(!scene.HasComponent<Rendering>(e)) return;

        auto& transforming = scene.GetComponent<Transforming>(e);
        auto& render = scene.GetComponent<Rendering>(e);

        
        if(IsKeyDown(KEY_W)){
            if(GetWorldToScreen(transforming.position, (*(render.camera))).y <= 0.0f){

            }
            else{
                transforming.position.y += 130 * dt;
            }
        }

        if(IsKeyDown(KEY_A)){
    
            if(GetWorldToScreen(transforming.position, (*(render.camera))).x < 0){

            }
            else{
                transforming.position.x += 130 * dt;
            }

        }

        if(IsKeyDown(KEY_S)){
            if(GetWorldToScreen(transforming.position, (*(render.camera))).y > (*(render.window)).GetHeight()){

            }
            else{
                transforming.position.y -= 130 * dt;
            }
        }

        if(IsKeyDown(KEY_D)){
            if(GetWorldToScreen(transforming.position, (*(render.camera))).x > (*(render.window)).GetWidth()){

            

            }
            else{
                transforming.position.x -= 130 * dt;
            }
        }    
    
}

void UpdateShipProperties(scene& scene, float dt){
    
 for(cs381::Entity e = 0; e < scene.entityMasks.size(); e++){
  if(!scene.HasComponent<Physics3D>(e)) continue;
  if(!scene.HasComponent<Transforming>(e)) continue;
  if(!scene.HasComponent<Rendering>(e)) continue;
  if(!scene.HasComponent<Ship>(e)) continue;

  auto& ship = scene.GetComponent<Ship>(e);
  auto& transforming = scene.GetComponent<Transforming>(e);

    if(ship.drawLaser == true){
     if(scene.HasComponent<Player>(e)){
     ship.targetBeamPos = ship.targetBeamPos - 2500.0f*dt;
     ship.shipLaserPosition = raylib::Vector3((transforming.position.x + ship.targetBeamPos), transforming.position.y, transforming.position.z);

     DrawBeam(raylib::Vector3(transforming.position.x - 20, transforming.position.y, transforming.position.z), ship.shipLaserPosition, SKYBLUE);


     if(ship.targetBeamPos <= -400){
        ship.drawLaser = false;
        ship.targetBeamPos = 0;
     }

      
     }
    }
    if(ship.targetLaserBar < 300){
        ship.targetLaserBar += 100*dt;

        if(ship.targetLaserBar > 290 ){
            ship.targetLaserBar = 300;
        }
     }

    if(ship.targetHealth < ship.shipHealth){
        ship.shipHealth -= 200*dt;
        if(abs(ship.targetHealth - ship.shipHealth) <= 5){
            ship.shipHealth = ship.targetHealth;
        }
    }
    else if(ship.targetHealth > ship.shipHealth){
        ship.shipHealth += 200*dt;
        if(abs(ship.targetHealth - ship.shipHealth) <= 5){
            ship.shipHealth = ship.targetHealth;
        }
    }

 }
}




void AddBuffer(int e, Input& input, Transforming& transforming, Rendering& render, Physics3D& physics, Ship& ship){

     (*(input.input))["moveUp"].SetPressedCallback([&transforming, &render, &physics]{
		
         //transforming.targetPos.y += 30;
         //physics.targetPos += 50;
         if(physics.roll < 45){
         physics.roll += 45;
         }
    
      
     }).move();

     (*(input.input))["moveDown"].SetPressedCallback([&transforming, &render, &physics]{
		
      if(physics.roll > -45){
         physics.roll -= 45;
         }
 		
         //transforming.targetPos.y -= 30;
    //     //physics.targetPos += 50;
    
      
     }).move();

     (*(input.input))["fireLaser"].SetPressedCallback([&ship, &render]{
		

        
        std::cout << ship.targetLaserBar << std::endl;
        if(!(ship.targetLaserBar < 300.0f)){
        ship.drawLaser = true;
        (*(render.laser)).Play(); 
        ship.targetLaserBar = 0;
        }

    }).move();

      
     

    //  (*(input.input))["moveLeft"].SetPressedCallback([&transforming, &render, &physics]{
		
    //      transforming.targetPos.x += 30.0f;
    
      
    //  }).move();

    //  (*(input.input))["moveRight"].SetPressedCallback([&transforming, &render, &physics]{
		
    //         transforming.targetPos.x -= 30.0f;
    
      
    //  }).move();
}

void InitializeBuffer(scene& scene){
    for(cs381::Entity e = 0; e < scene.entityMasks.size(); e++){
        if(!scene.HasComponent<Input>(e)) continue;
        if(!scene.HasComponent<Transforming>(e)) continue;
        if(!scene.HasComponent<Rendering>(e)) continue;
        if(!scene.HasComponent<Physics3D>(e)) continue;
        if(!scene.HasComponent<Ship>(e)) continue;

        // auto& transforming = scene.GetComponent<Transforming>(e);
        // auto& render = scene.GetComponent<Rendering>(e);
        
        
        auto& input = scene.GetComponent<Input>(e);
        auto& render = scene.GetComponent<Rendering>(e);
        auto& transforming = scene.GetComponent<Transforming>(e);
        auto& physics = scene.GetComponent<Physics3D>(e);
        auto& ship = scene.GetComponent<Ship>(e);

        AddBuffer(e, input, transforming, render, physics, ship);
        
    }
}

void DisplayShipHUD(scene& scene, cs381::Entity e, raylib::Text* text, raylib::Window* window){
    if(!scene.HasComponent<Ship>(e)) return;

    auto& ship = scene.GetComponent<Ship>(e);
    //auto.shipHealth = ship.shipHealth;

    DrawRectangle(30 , (*window).GetHeight() / 4 - 10, 300, 20, raylib::Color(GRAY));
    DrawRectangle(30 , (*window).GetHeight() / 4 - 10, ship.targetLaserBar, 20, raylib::Color(GREEN));

    DrawRectangle(30 , (*window).GetHeight() / 6 - 10, 300, 20, raylib::Color(GRAY));

    if(ship.shipHealth > 200 && ship.shipHealth <= 300){
     DrawRectangle(30 , (*window).GetHeight() / 6 - 10, ship.shipHealth, 20, raylib::Color(GREEN));
    }

    else if(ship.shipHealth > 100 && ship.shipHealth <= 200){
     DrawRectangle(30 , (*window).GetHeight() / 6 - 10, ship.shipHealth, 20, raylib::Color(YELLOW));
    }

    else if(ship.shipHealth > 50 && ship.shipHealth <= 100){
     DrawRectangle(30 , (*window).GetHeight() / 6 - 10, ship.shipHealth, 20, raylib::Color(ORANGE));
    }

    else if(ship.shipHealth > 0 && ship.shipHealth <= 50){
     DrawRectangle(30 , (*window).GetHeight() / 6 - 10, ship.shipHealth, 20, raylib::Color(RED));
    }

    if(scene.HasComponent<Player>(e)){
     (*text).Draw("Ship Health: ", (30), (70), 40, raylib::Color::White());
     (*text).Draw("Laser: ", (30), (140), 20, raylib::Color::White());
     if(ship.shipHealth <= 0) (*text).Draw("DESTORYED ", (30), (100), 40, raylib::Color::Red());
    }

}


void TrackGameState(scene& scene, cs381::Entity e, StateMachine& state){
    
    if(state == StateMachine::titleScreen){
        if(IsKeyPressed(KEY_ENTER)) state = StateMachine::mainLevel;
        return;
    }

    if(!(scene.HasComponent<Transforming>(e))) return;
    if(!(scene.HasComponent<Ship>(e))) return;
    if(!(scene.HasComponent<Player>(e))) return;

    auto& transforming = scene.GetComponent<Transforming>(e);
    auto& ship = scene.GetComponent<Ship>(e);

    if(ship.shipHealth <= 0.0f){
         (*(scene.GetComponent<Player>(e).death)).Play();
         state = StateMachine::gameOver;
         return;
    }

    if(state == StateMachine::mainLevel && transforming.position.x <= -4600) state = StateMachine::bossFight;

}

void UpdateBossAI(scene& scene, cs381::Entity e){

}


int main(){ 

raylib::Window window(1000, 700, "CS381 - Assignment 9");
raylib::Text text;
int windHeight = 0, windWidth = 0;
// bool playSound = false;
// bool gameOver = false;
// bool victory = false;
// bool startGame = false;

raylib::AudioDevice audio;

raylib::Music music("../sounds/corneria.mp3");
raylib::Sound ping("../sounds/starfoxsfx.mp3");
raylib::Sound laser("../sounds/laser.mp3");
raylib::Sound down("../sounds/playerdown.mp3");
raylib::Music death("../sounds/continue.mp3");
raylib::Music boss("../sounds/boss.mp3");
raylib::Music vict("../sounds/victory.mp3");

raylib::Model modelMeteor = LoadModel("../meshes/meteorite.glb");
raylib::Model* meteorPointer = &modelMeteor;
modelMeteor.transform = raylib::Transform(modelMeteor.transform).Scale(8, 8, 8);

raylib::Model modelShip = LoadModel("../meshes/spaceship.glb");
modelShip.transform = raylib::Transform(modelShip.transform).Scale(0.02, 0.02, 0.02);
modelShip.transform = raylib::Transform(modelShip.transform).RotateY(raylib::Degree(270));

raylib::Model modelBoss = LoadModel("../meshes/boss2.glb");
modelBoss.transform = raylib::Transform(modelBoss.transform).RotateX(raylib::Degree(270));
modelBoss.transform = raylib::Transform(modelBoss.transform).RotateY(raylib::Degree(270));

raylib::Model modelPlanet = LoadModel("../meshes/planet.glb");


//Initialize Callbacks
raylib::BufferedInput inputs;
inputs["moveUp"] = raylib::Action::key(KEY_W);
inputs["moveDown"] = raylib::Action::key(KEY_S);
inputs["moveLeft"] = raylib::Action::key(KEY_A);
inputs["moveRight"] = raylib::Action::key(KEY_D);
inputs["fireLaser"] = raylib::Action::key(KEY_SPACE);
//IsKeyDown(KEY_W);
//inputs["moveUp"] = IsKeyDown(KEY_W);

std::vector<Particle> particles;

raylib::Camera camera(
        raylib::Vector3(0, 0, -350),
        raylib::Vector3(0, 0, 0),
        raylib::Vector3::Up(),
        45,
        CAMERA_PERSPECTIVE
    );

raylib::Vector3 cameraVelocity = {-50, 0, 0};



cs381::SkyBox skybox("textures/deepbluespace.png");

SetTargetFPS(60);

cs381::Scene<cs381::SkiplistComponentStorage> mainScene;

auto player = mainScene.CreateEntity();
mainScene.AddComponent<Transforming>(player);
mainScene.AddComponent<Rendering>(player);
mainScene.AddComponent<Physics3D>(player);
mainScene.AddComponent<Input>(player);
mainScene.AddComponent<Player>(player);
mainScene.AddComponent<Ship>(player);

mainScene.GetComponent<Physics3D>(player).acceleration = 5;
mainScene.GetComponent<Physics3D>(player).quat = raylib::Quaternion::FromEuler(0.0f, 0.0f, 0.0f);
mainScene.GetComponent<Physics3D>(player).isMeteor = false;

mainScene.GetComponent<Transforming>(player).velocity = raylib::Vector3{-50, 0, 0};
mainScene.GetComponent<Transforming>(player).scale = raylib::Vector3{1, 1, 1};

mainScene.GetComponent<Input>(player).input = &inputs;

mainScene.GetComponent<Rendering>(player).model = &modelShip;
mainScene.GetComponent<Rendering>(player).camera = &camera;
mainScene.GetComponent<Rendering>(player).window = &window;
mainScene.GetComponent<Rendering>(player).ping = &ping;
mainScene.GetComponent<Rendering>(player).laser = &laser;
mainScene.GetComponent<Rendering>(player).impactPoint = {0, 0};
mainScene.GetComponent<Rendering>(player).invisible = false;


mainScene.GetComponent<Ship>(player).targetLaserBar = 300.0f;
mainScene.GetComponent<Ship>(player).targetHealth = 300.0f;
mainScene.GetComponent<Ship>(player).shipHealth = 300.0f;
mainScene.GetComponent<Ship>(player).shipLaserPosition = raylib::Vector3{0, 0, 0};
mainScene.GetComponent<Ship>(player).targetBeamPos = 0.0f; 
mainScene.GetComponent<Ship>(player).drawLaser = false;

mainScene.GetComponent<Player>(player).death = &down;

auto meteor1 = mainScene.CreateEntity();
auto meteor2 = mainScene.CreateEntity();
auto meteor3 = mainScene.CreateEntity();
auto meteor4 = mainScene.CreateEntity();
auto meteor5 = mainScene.CreateEntity();

auto meteor6 = mainScene.CreateEntity();
auto meteor7 = mainScene.CreateEntity();
auto meteor8 = mainScene.CreateEntity();
auto meteor9 = mainScene.CreateEntity();
auto meteor10 = mainScene.CreateEntity();

auto meteor11 = mainScene.CreateEntity();
auto meteor12 = mainScene.CreateEntity();
auto meteor13 = mainScene.CreateEntity();
auto meteor14 = mainScene.CreateEntity();
auto meteor15 = mainScene.CreateEntity();

auto meteor16 = mainScene.CreateEntity();
auto meteor17 = mainScene.CreateEntity();
auto meteor18 = mainScene.CreateEntity();
auto meteor19 = mainScene.CreateEntity();
auto meteor20 = mainScene.CreateEntity();

auto meteor21 = mainScene.CreateEntity();
auto meteor22 = mainScene.CreateEntity();
auto meteor23 = mainScene.CreateEntity();
auto meteor24 = mainScene.CreateEntity();
auto meteor25 = mainScene.CreateEntity();

auto meteor26 = mainScene.CreateEntity();
auto meteor27 = mainScene.CreateEntity();
auto meteor28 = mainScene.CreateEntity();
auto meteor29 = mainScene.CreateEntity();
auto meteor30 = mainScene.CreateEntity();

auto meteor31 = mainScene.CreateEntity();
auto meteor32 = mainScene.CreateEntity();
auto meteor33 = mainScene.CreateEntity();
auto meteor34 = mainScene.CreateEntity();
auto meteor35 = mainScene.CreateEntity();

auto meteor36 = mainScene.CreateEntity();
auto meteor37 = mainScene.CreateEntity();
auto meteor38 = mainScene.CreateEntity();
auto meteor39 = mainScene.CreateEntity();
auto meteor40 = mainScene.CreateEntity();

auto meteor41 = mainScene.CreateEntity();
auto meteor42 = mainScene.CreateEntity();
auto meteor43 = mainScene.CreateEntity();

//INITIALIZE METEOR VALUES

int changeTurn = 1;

for(cs381::Entity e = 1; e < mainScene.entityMasks.size(); e++){
//std::cout << "test" << std::endl;
mainScene.AddComponent<Transforming>(e);
mainScene.AddComponent<Rendering>(e);
mainScene.AddComponent<Physics3D>(e);
//mainScene.AddComponent<Meteor>(e);

mainScene.GetComponent<Transforming>(e).velocity = raylib::Vector3{50, 0, 0};
mainScene.GetComponent<Transforming>(e).scale = raylib::Vector3{1, 1, 1};

mainScene.GetComponent<Rendering>(e).model = &modelMeteor;
mainScene.GetComponent<Rendering>(e).camera = &camera;
mainScene.GetComponent<Rendering>(e).window = &window;
mainScene.GetComponent<Rendering>(e).ping = &ping;
mainScene.GetComponent<Rendering>(e).impactPoint = {0, 0};
mainScene.GetComponent<Rendering>(e).invisible = false;

mainScene.GetComponent<Physics3D>(e).acceleration = 5;
mainScene.GetComponent<Physics3D>(e).quat = raylib::Quaternion::FromEuler(0.0f, 0.0f, 0.0f);
mainScene.GetComponent<Physics3D>(e).isMeteor = true;

mainScene.GetComponent<Physics3D>(e).targetRoll = 0.0f;
mainScene.GetComponent<Physics3D>(e).targetPitch = 0.0f;
mainScene.GetComponent<Physics3D>(e).targetYaw = 0.0f;

switch(changeTurn) {
    case 1: 
    mainScene.GetComponent<Physics3D>(e).targetRoll = 30;
    break;
    case 2: 
    mainScene.GetComponent<Physics3D>(e).targetYaw = 30;
    break;
    case 3:
    mainScene.GetComponent<Physics3D>(e).targetPitch = 30;
    break;
    case 4:
    mainScene.GetComponent<Physics3D>(e).targetRoll = 30;
    mainScene.GetComponent<Physics3D>(e).targetYaw = 30;
    break; 
    case 5: 
    mainScene.GetComponent<Physics3D>(e).targetRoll = 30;
    mainScene.GetComponent<Physics3D>(e).targetPitch = 30;
    break;
    case 6: 
    mainScene.GetComponent<Physics3D>(e).targetYaw = 30;
    mainScene.GetComponent<Physics3D>(e).targetPitch = 30;
    break;
    case 7: 
    mainScene.GetComponent<Physics3D>(e).targetRoll = 30;
    mainScene.GetComponent<Physics3D>(e).targetYaw = 30; 
    mainScene.GetComponent<Physics3D>(e).targetPitch = 30;
    break;
}

changeTurn++;

if(changeTurn > 7){
    changeTurn = 1;
}

}

//DEFINE METEOR POSITIONS
mainScene.GetComponent<Transforming>(meteor1).position = raylib::Vector3{-700, 0, 0};
mainScene.GetComponent<Transforming>(meteor2).position = raylib::Vector3{-900, 50, 0};
mainScene.GetComponent<Transforming>(meteor3).position = raylib::Vector3{-1100, -20, 0};
mainScene.GetComponent<Transforming>(meteor4).position = raylib::Vector3{-1300, -90, 0};
mainScene.GetComponent<Transforming>(meteor5).position = raylib::Vector3{-1500, -30, 0};

mainScene.GetComponent<Transforming>(meteor6).position = raylib::Vector3{-1600, 60, 0};
mainScene.GetComponent<Transforming>(meteor7).position = raylib::Vector3{-1800, -90, 0};
mainScene.GetComponent<Transforming>(meteor8).position = raylib::Vector3{-2000, 80, 0};
mainScene.GetComponent<Transforming>(meteor9).position = raylib::Vector3{-2250, -70, 0};
mainScene.GetComponent<Transforming>(meteor10).position = raylib::Vector3{-2570, 50, 0};

mainScene.GetComponent<Transforming>(meteor11).position = raylib::Vector3{-2850, 90, 0};
mainScene.GetComponent<Transforming>(meteor12).position = raylib::Vector3{-3050, 0, 0};
mainScene.GetComponent<Transforming>(meteor13).position = raylib::Vector3{-3150, -95, 0};
mainScene.GetComponent<Transforming>(meteor14).position = raylib::Vector3{-3380, -50, 0};
mainScene.GetComponent<Transforming>(meteor15).position = raylib::Vector3{-3500, 74, 0};

mainScene.GetComponent<Transforming>(meteor16).position = raylib::Vector3{-3750, -80, 0};
mainScene.GetComponent<Transforming>(meteor17).position = raylib::Vector3{-3900, -20, 0};
mainScene.GetComponent<Transforming>(meteor18).position = raylib::Vector3{-4150, 20, 0};
mainScene.GetComponent<Transforming>(meteor19).position = raylib::Vector3{-4275, -90, 0};
mainScene.GetComponent<Transforming>(meteor20).position = raylib::Vector3{-4325, 45, 0};

mainScene.GetComponent<Transforming>(meteor21).position = raylib::Vector3{-4550, -60, 0};
mainScene.GetComponent<Transforming>(meteor22).position = raylib::Vector3{-4600, 0, 0};
mainScene.GetComponent<Transforming>(meteor23).position = raylib::Vector3{-4830, 60, 0};
mainScene.GetComponent<Transforming>(meteor24).position = raylib::Vector3{-5000, -50, 0};
mainScene.GetComponent<Transforming>(meteor25).position = raylib::Vector3{-5270, 50, 0};

mainScene.GetComponent<Transforming>(meteor26).position = raylib::Vector3{-5410, 80, 0};
mainScene.GetComponent<Transforming>(meteor27).position = raylib::Vector3{-5650, -15, 0};
mainScene.GetComponent<Transforming>(meteor28).position = raylib::Vector3{-5800, 23, 0};
mainScene.GetComponent<Transforming>(meteor29).position = raylib::Vector3{-6000, 69, 0};
mainScene.GetComponent<Transforming>(meteor30).position = raylib::Vector3{-6250, -90, 0};

mainScene.GetComponent<Transforming>(meteor31).position = raylib::Vector3{-6500, 0, 0};
mainScene.GetComponent<Transforming>(meteor32).position = raylib::Vector3{-6850, 30, 0};
mainScene.GetComponent<Transforming>(meteor33).position = raylib::Vector3{-7075, -20, 0};
mainScene.GetComponent<Transforming>(meteor34).position = raylib::Vector3{-7225, 75, 0};
mainScene.GetComponent<Transforming>(meteor35).position = raylib::Vector3{-7450, 10, 0};

mainScene.GetComponent<Transforming>(meteor36).position = raylib::Vector3{-7680, 35, 0};
mainScene.GetComponent<Transforming>(meteor37).position = raylib::Vector3{-7930, -10, 0};
mainScene.GetComponent<Transforming>(meteor38).position = raylib::Vector3{-8120, -50, 0};
mainScene.GetComponent<Transforming>(meteor39).position = raylib::Vector3{-8370, 50, 0};
mainScene.GetComponent<Transforming>(meteor40).position = raylib::Vector3{-8560, 80, 0};

mainScene.GetComponent<Transforming>(meteor41).position = raylib::Vector3{-8780, -35, 0};
mainScene.GetComponent<Transforming>(meteor42).position = raylib::Vector3{-8900, 23, 0};
mainScene.GetComponent<Transforming>(meteor43).position = raylib::Vector3{-9100, 90, 0};

//DEFINE SCALE PROPERTIES
mainScene.GetComponent<Transforming>(meteor2).scale = raylib::Vector3{1.3, 1.3, 1.3};
mainScene.GetComponent<Transforming>(meteor4).scale = raylib::Vector3{1.6, 1.6, 1.6};
mainScene.GetComponent<Transforming>(meteor5).scale = raylib::Vector3{1.4, 1.4, 1.4};
mainScene.GetComponent<Transforming>(meteor12).scale = raylib::Vector3{1.2, 1.2, 1.2};
mainScene.GetComponent<Transforming>(meteor16).scale = raylib::Vector3{1.4, 1.4, 1.4};
mainScene.GetComponent<Transforming>(meteor19).scale = raylib::Vector3{1.5, 1.5, 1.5};
mainScene.GetComponent<Transforming>(meteor23).scale = raylib::Vector3{2, 2, 2};
mainScene.GetComponent<Transforming>(meteor24).scale = raylib::Vector3{1.5, 1.5, 1.5};
mainScene.GetComponent<Transforming>(meteor26).scale = raylib::Vector3{1.3, 1.3, 1.3};
mainScene.GetComponent<Transforming>(meteor27).scale = raylib::Vector3{1.4, 1.4, 1.4};
mainScene.GetComponent<Transforming>(meteor29).scale = raylib::Vector3{1.3, 1.3, 1.3};
mainScene.GetComponent<Transforming>(meteor31).scale = raylib::Vector3{1.6, 1.6, 1.6};
mainScene.GetComponent<Transforming>(meteor32).scale = raylib::Vector3{1.2, 1.2, 1.2};
mainScene.GetComponent<Transforming>(meteor35).scale = raylib::Vector3{1.4, 1.4, 1.4};
mainScene.GetComponent<Transforming>(meteor36).scale = raylib::Vector3{1.3, 1.3, 1.3};
mainScene.GetComponent<Transforming>(meteor38).scale = raylib::Vector3{1.5, 1.5, 1.5};
mainScene.GetComponent<Transforming>(meteor39).scale = raylib::Vector3{1.2, 1.2, 1.2};
mainScene.GetComponent<Transforming>(meteor41).scale = raylib::Vector3{1.3, 1.3, 1.3};
mainScene.GetComponent<Transforming>(meteor43).scale = raylib::Vector3{1.6, 1.6, 1.6};

auto bossEntity = mainScene.CreateEntity();

mainScene.AddComponent<Transforming>(bossEntity);
mainScene.AddComponent<Rendering>(bossEntity);
mainScene.AddComponent<Physics3D>(bossEntity);
mainScene.AddComponent<Ship>(bossEntity);


mainScene.GetComponent<Transforming>(bossEntity).position = raylib::Vector3{-4600, 0, 50};
mainScene.GetComponent<Transforming>(bossEntity).scale = raylib::Vector3{9, 9, 9};

mainScene.GetComponent<Rendering>(bossEntity).model = &modelBoss;
mainScene.GetComponent<Rendering>(bossEntity).camera = &camera;
mainScene.GetComponent<Rendering>(bossEntity).window = &window;
mainScene.GetComponent<Rendering>(bossEntity).laser = &laser;
mainScene.GetComponent<Rendering>(bossEntity).ping = &ping;
mainScene.GetComponent<Physics3D>(bossEntity).roll = 30.0f;

// mainScene.GetComponent<Ship>(bossEntity).targetLaserBar = 300.0f;
// mainScene.GetComponent<Ship>(bossEntity).targetHealth = 300.0f;
// mainScene.GetComponent<Ship>(bossEntity).shipHealth = 300.0f;
// mainScene.GetComponent<Ship>(bossEntity).targetBeamPos = 0.0f; 
// mainScene.GetComponent<Ship>(bossEntity).drawLaser = false;
//mainScene.GetComponent<Ship>(bossEntity).isBoss = true;


StateMachine state;
state = titleScreen;
std::cout << state << std::endl;

float wait = 0.0f;

InitializeBuffer(mainScene);

while(!window.ShouldClose()) {
 

window.BeginDrawing();
{
// Clear screen
window.ClearBackground(BLACK);


switch(state){
    case StateMachine::titleScreen : 
    
    //window.BeginDrawing();
    //{

    text.Draw("To initiate liftoff, press enter", (window.GetWidth()/2), (window.GetHeight()/2), 20, raylib::Color::White());

    //window.EndDrawing();
    //}

    TrackGameState(mainScene, player, state);
    break;

    case StateMachine::mainLevel : 

    camera.position = (raylib::Vector3)camera.position + cameraVelocity*window.GetFrameTime();
    camera.target = (raylib::Vector3)camera.target + cameraVelocity*window.GetFrameTime();

    camera.BeginMode();
    {
     
     music.Play();
     music.Update();
     TrackGameState(mainScene, player, state);
     UpdatePlayer(mainScene, player, window.GetFrameTime());
     UpdateShipProperties(mainScene, window.GetFrameTime());
     UpdateShipReference(mainScene);
     ApplyPhysics(mainScene, window.GetFrameTime());
     skybox.Draw();
     Draw(mainScene, state, window.GetFrameTime());
     
    }
	camera.EndMode();

    //DRAW HUD 
    DisplayShipHUD(mainScene, player, &text, &window);

    DrawFPS(10, 10);
    inputs.PollEvents();
    
    break;

    case StateMachine::gameOver :

    camera.BeginMode();
    {

    if(wait < 20.0f) wait += 5*window.GetFrameTime();
    
    if(wait >= 20.0f){
    death.Play();
    death.Update();
    }

    skybox.Draw();
    Draw(mainScene, state, window.GetFrameTime());

    }
    camera.EndMode();

    DisplayShipHUD(mainScene, player, &text, &window);
    DrawFPS(10, 10);

    break;

    case StateMachine::bossFight :


    camera.position = (raylib::Vector3)camera.position + cameraVelocity*window.GetFrameTime();
    camera.target = (raylib::Vector3)camera.target + cameraVelocity*window.GetFrameTime();

    camera.BeginMode();
    {
     
     music.Play();
     music.Update();
     TrackGameState(mainScene, player, state);
     UpdatePlayer(mainScene, player, window.GetFrameTime());
     UpdateShipProperties(mainScene, window.GetFrameTime());
     UpdateShipReference(mainScene);
     ApplyPhysics(mainScene, window.GetFrameTime());
     skybox.Draw();
     Draw(mainScene, state, window.GetFrameTime());
     
    }
	camera.EndMode();

    //DRAW HUD 
    DisplayShipHUD(mainScene, player, &text, &window);

    DrawFPS(10, 10);
    inputs.PollEvents();
    
    break;

}

}
window.EndDrawing();



}

return 0;
}