//Author: Aaron Whitaker
//Assignment: AS7
//Date: 3-26-2024


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

//#define PI 3.142857
//#include "VolumeControl.h"

// #include "inputs.hpp"
#include "skybox.hpp"

template<typename T>
concept Transformer = requires(T t, raylib::Transform m) {
	{ t.operator()(m) } -> std::convertible_to<raylib::Transform>;
};


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

bool ProcessInput(raylib::Degree& planeTargetHeading, float& planeTargetSpeed, size_t& selectedPlane);


void DrawBoundedModel(raylib::Model& model, Transformer auto transformer);
void DrawModel(raylib::Model& model, Transformer auto transformer);


struct Component { 
    struct Entity* object; 

    Component(struct Entity& e) : object(&e) {}

    virtual void setup(){ };
    virtual void cleanup(){ };
    virtual void tick(float dt){ };


};

struct TransformComponent : public Component { 
    //Using components constructor instead of a new one. 
    using Component::Component; 
    raylib::Vector3 position = {0, 0, 0};
	raylib::Vector3 velocity = raylib::Vector3::Zero();
    raylib::Vector3 scale = {1, 1, 1};
	raylib::Degree heading = 0, targetHeading = 0;
	raylib::Degree turn = 0, targetTurn = 0, targetRevertTurn = 0;
    raylib::Degree roll = 0;

	//float maxTurn = 45;
	float speed = 0, targetSpeed = 0, dt_ = 0;
    float targetX = 0, targetY = 0, targetZ = 0;
    float targetLaserBar = 300.0f, targetHealth = 300;


    float shipHealth = 300;
	float maxSpeed = 50;
	float minSpeed = 0;

    //static memory allocation is the allocation of memory at compile time, before the associated program is executed, 
	//unlike dynamic memory allocation or automatic memory allocation where memory is allocated as required at run time.

	float acceleration = 70; 
	float angularAcceleration = 10;

    //Quaternion?????
    raylib::Quaternion rotation = raylib::Quaternion::Identity();

    void setup() {

    }

    void cleanup() {

    }

    void tick(float dt){	
		dt_ = dt;

		
    }
};

struct Entity { 
    std::vector<std::unique_ptr<Component>> components;

    //Add component of transform component through constructor
    Entity() { AddComponent<TransformComponent>(); }

    //Delete the copy constructor. We are not allowing the Entity to be copied.
    Entity(const Entity&) = delete; 

    //Move constructor. Loop through entire passed entity's components and change their reference to the new object.
    //Vector behind the scenes is an array.
    //Copying every element of an array IS WAY MORE EXPENSIVE 
    //Beter to move pointer.

	////Forwarding references are a special kind of references that preserve the value category of a function argument,
    //making it possible to forward it by means of std::forward. Forwarding references are either: 
    Entity(Entity&& other) : components(std::move(other.components)) { 
        for(auto& c: components){
            c->object = this;
        }

    }

    template<std::derived_from<Component> T, typename... Ts>
    size_t AddComponent(Ts... args) {
        auto c = std::make_unique<T>(*this, std::forward<Ts>(args)...);
        components.push_back(std::move(c));
        return components.size() - 1;
    }

    //Only going to take child classes of component
    //Same semantics as a reference. Can't going inside a vector, etc. std::reference_wrapper can.
    template<std::derived_from<Component> T>
    std::optional<std::reference_wrapper<T>> GetComponent(){

		//std::reference_wrapper is a class template that wraps a reference in a copyable, assignable object.
		//
        //Compile time if statement 
        if constexpr(std::is_same_v<T, TransformComponent>){
			
            //Optimization
            //We KNOW the first component will be the transform, thats why we use 0 here.
            T* cast = dynamic_cast<T*>(components[0].get());

            //If not nullptr, return value stored in cast.
            if(cast) return *cast;
        }

		/*if constexpr(std::is_same_v<T, RenderingComponent>){
            //Optimization
            //We KNOW the first component will be the transform, thats why we use 0 here.
            T* cast = dynamic_cast<T*>(components[0].get());

            //If not nullptr, return value stored in cast.
            if(cast) return *cast;
        }*/

        for(auto& c: components){

            //c.get() returns a pointer to the managed object.
            //dynamic cast : If the pointer can be validly converted into a T , then do it.
            //If not , give us a nullptr.
            T* cast = dynamic_cast<T*>(c.get());
			

            //If not nullptr, return value stored in cast.
            if(cast){
				return *cast;
			} 
        }

        //null pointer for optionals.

		//std::cout << "getting component" << std::endl;
        return std::nullopt;
    }


    void tick(float dt){
        for(auto& c: components)
            c->tick(dt);
    }
};

bool ProcessEntityInput(Entity& entity, raylib::BufferedInput& inputs, Camera& camera, raylib::Window& window);
bool ProcessEntityPhysics(Entity& entity);

struct RenderingComponent : public Component {

	using Component::Component; 

	RenderingComponent(Entity& e, raylib::Model&& model) : Component(e), model(std::move(model)) { };

	raylib::Model model; //Assuming set on construction
    raylib::Model* model2;
    //raylib::Model* ship;
	bool drawBox = false, drawParticles = false;
    bool isShip = false, drawLaser = false;
    bool isBackground = false;
    bool isBoss = false, startFight = false;
    bool modelUnloaded = false, invisible = false, gameOver = false;
    float targetBeamPos = 0;
    Vector2 impactPoint = {0, 0};
    raylib::Vector3 shipLaserPosition = {0, 0, 0};

    struct Entity* shipObject;
    raylib::Sound* ping;
    raylib::Sound* laser;
    std::vector<Particle> particles;

    raylib::Window* window;
    Camera* cam;

	//Transformer auto transformer;




	void setup() {

	}

	void cleanup() {

	}

	void tick(float dt) { 
		
		

		auto ref = object->GetComponent<TransformComponent>();
		

		
		//Because its optional reference, may or may not exist. 
		//If it doesn't exist, we return.
		 if(!ref){
			std::cout << "No reference found" << std::endl;
			return;
		 } 


		auto& transform = ref->get();

		auto pos = transform.position;
		auto head = transform.heading;
		auto turn = transform.turn;
        auto roll = transform.roll;
        auto scale = transform.scale;

        auto transformer = [pos, head, turn, roll, scale](raylib::Transform transform) {
			return transform.Translate(pos).RotateX(turn).RotateY(raylib::Degree(head)).RotateZ(roll).Scale(scale.x, scale.y, scale.z);
		};

        if(isShip == false && !invisible && !isBackground && !isBoss){

         if(GetWorldToScreen(pos, *cam).x > -40.0 && GetWorldToScreen(pos, *cam).x < (*window).GetWidth() + 50){
            //std::cout << "IN BOUNDS" << std::endl;
            auto shipRef = shipObject->GetComponent<TransformComponent>();

            if(!shipRef){
			 std::cout << "No reference found" << std::endl;
			 return;
		    } 

            auto& shipTransform = shipRef->get();

            auto shipPos = shipTransform.position;
		    auto shipHead = shipTransform.heading;
		    auto shipTurn = shipTransform.turn;
            auto shipRoll = shipTransform.roll;
            auto shipScale = shipTransform.scale;

            auto shipRef2 = shipObject->GetComponent<RenderingComponent>();

            if(!shipRef2){
			 std::cout << "No reference found" << std::endl;
			 return;
		    } 

            auto& shipRender = shipRef2->get();
            raylib::Model* shipModel = shipRender.model2;



            auto shipTransformer = [shipPos, shipHead, shipTurn, shipRoll, shipScale](raylib::Transform transform) {
			return transform.Translate(shipPos).RotateX(shipTurn).RotateY(raylib::Degree(shipHead)).RotateZ(shipRoll).Scale(shipScale.x, shipScale.y, shipScale.z);
		 };
         
         if(!gameOver){
         if(CheckCollisions(*model2, *shipModel, shipTransformer, transformer)){
            //std::cout << "Collision detected" << std::endl;
            
            //std::cout << shipTransform.shipHealth << std::endl;
            if(particles.empty()){
                shipTransform.targetHealth -= 50;
                impactPoint = Vector2{shipPos.x, shipPos.y};
                FillParticleVector(particles, impactPoint);
                
                drawParticles = DrawParticles(particles, dt, impactPoint);

                (*ping).Play();

            }
         }

        if(shipRender.drawLaser == true){
            //std::cout << "drawing laser from asteroid" << std::endl;
            raylib::Transform backupTransform = (*model2).transform;
            (*model2).transform = transformer((*model2).transform);

            if(CheckCollisionBoxSphere((*model2).GetTransformedBoundingBox(), shipRender.shipLaserPosition, 20.0f)){
                std::cout << "laser collided" << std::endl;
                //UnloadModel(model);
                invisible = true;
                impactPoint = Vector2{pos.x, pos.y};
                FillParticleVector(particles, impactPoint);
                
                drawParticles = DrawParticles(particles, dt, impactPoint);

                (*ping).Play();

            }

            (*model2).transform = backupTransform;

        }

        }

         else if(GetWorldToScreen(pos, *cam).x < -40.0){
            if(!modelUnloaded){
                //UnloadModel(model);
                modelUnloaded = true;
            }   
         }
     }
    }

    if(drawParticles == true){
        std::cout << "Drawing particles" << std::endl;
        drawParticles = DrawParticles(particles, dt, impactPoint);
    }


		//To axis angle returns a std::pair of a <vector3, and a radian>.
		//Axis to rotate around, and how far to rotate around that axis.

		auto [axis, angle] = transform.rotation.ToAxisAngle();
        
        if(isShip == true){
         if(drawLaser == true){
            targetBeamPos = targetBeamPos - 2500.0f*dt;
            std::cout << abs((*cam).position.x - transform.position.x) << std::endl;
            /*if(transform.position.x < (*cam).position.x){
                //std::cout << "true" << std::endl;
                targetBeamPos -= abs((*cam).position.x - transform.position.x);
                
            }*/
            DrawBeam(raylib::Vector3(transform.position.x - 20, transform.position.y, transform.position.z), raylib::Vector3((transform.position.x + targetBeamPos), transform.position.y, transform.position.z), SKYBLUE);
            //std::cout << (*window).GetWidth() + (*cam).position.x << std::endl;
            //std::cout << (*cam).position.x << std::endl;
            shipLaserPosition = raylib::Vector3((transform.position.x + targetBeamPos), transform.position.y, transform.position.z);
            

            //std::cout << dt << std::endl;
            if(targetBeamPos <= -400){
                drawLaser = false;
                targetBeamPos = 0;
            }
         }
        }

        else if(isBoss == true && startFight == true){
            if(drawLaser == true){
            targetBeamPos = targetBeamPos + 2500.0f*dt;
            //std::cout << abs((*cam).position.x - transform.position.x) << std::endl;
            /*if(transform.position.x < (*cam).position.x){
                //std::cout << "true" << std::endl;
                targetBeamPos -= abs((*cam).position.x - transform.position.x);
                
            }*/
            DrawBeam(raylib::Vector3(transform.position.x + 20, transform.position.y, transform.position.z), raylib::Vector3((transform.position.x + targetBeamPos), transform.position.y, transform.position.z), RED);
            //std::cout << (*window).GetWidth() + (*cam).position.x << std::endl;
            //std::cout << (*cam).position.x << std::endl;
            shipLaserPosition = raylib::Vector3((transform.position.x + targetBeamPos), transform.position.y, transform.position.z);
            

            //std::cout << dt << std::endl;
            if(targetBeamPos >= 400){
                drawLaser = false;
                targetBeamPos = 0;
            }


         }
        }

        if(startFight == true){
         auto shipRef2 = shipObject->GetComponent<RenderingComponent>();

            if(!shipRef2){
			 std::cout << "No reference found" << std::endl;
			 return;
		    } 

            auto& shipRender = shipRef2->get();
            if(shipRender.drawLaser == true){
             //std::cout << "drawing laser from asteroid" << std::endl;
             raylib::Transform backupTransform = (*model2).transform;
             (*model2).transform = transformer((*model2).transform);

             if(CheckCollisionBoxSphere((*model2).GetTransformedBoundingBox(), shipRender.shipLaserPosition, 10.0f)){
                std::cout << "laser collided" << std::endl;
                //UnloadModel(model);
                //invisible = true;
                if(particles.empty()){
                transform.targetHealth -= 50;
                impactPoint = Vector2{pos.x, pos.y};
                FillParticleVector(particles, impactPoint);
                
                
                drawParticles = DrawParticles(particles, dt, impactPoint);

                (*ping).Play();
                }

                /*if(particles.empty()){
                shipTransform.shipHealth -= 50;
                impactPoint = Vector2{shipPos.x, shipPos.y};
                FillParticleVector(particles, impactPoint);
                
                drawParticles = DrawParticles(particles, dt, impactPoint);

                (*ping).Play();*/

            

             }

             (*model2).transform = backupTransform;

            }
         }


		//model.Draw(transform.position, axis, angle);

    if(isBoss){
        if(!(GetWorldToScreen(pos, *cam).x > -40.0 && GetWorldToScreen(pos, *cam).x < (*window).GetWidth() + 50)){
                invisible = false;
        }
    }

       

        if(!modelUnloaded && !invisible) DrawModel(*model2, transformer);

       
		
	 
    }
	
};

struct PhysicsComponent : public Component { 

	using Component::Component; 



	void tick(float dt){
	auto transRef = object->GetComponent<TransformComponent>();
    auto rendRef = object->GetComponent<RenderingComponent>();
		

		
		//Because its optional reference, may or may not exist. 
		//If it doesn't exist, we return.
		 if(!transRef){
			std::cout << "No reference found" << std::endl;
			return;
		 } 

         if(!rendRef){
			std::cout << "No reference found" << std::endl;
			return;
		 } 


	auto& transform = transRef->get();
    auto& render = rendRef->get();

    if(!render.isShip && !render.isBoss){
        //std::cout << "Hello" << std::endl;
        transform.heading += transform.targetX * dt;
        transform.turn += transform.targetY * dt;
        transform.roll += transform.targetZ * dt;
        //std::cout << transform.heading << std::endl;
        if(transform.heading > 360){
            transform.heading = 0;
        }
        if(transform.turn > 360){
            transform.turn = 0;
        }
        if(transform.roll > 360){
            transform.roll = 0;
        }

    }

    else{
	ProcessEntityPhysics(*object);
    if(render.isShip) std::cout << transform.position.x << std::endl;
    if(transform.targetLaserBar < 300){
        
        transform.targetLaserBar += 100*dt;

        if(transform.targetLaserBar > 290 ){
            transform.targetLaserBar = 300;
        }
    }
    if(transform.targetHealth < transform.shipHealth){
        transform.shipHealth -= 200*dt;
        if(abs(transform.targetHealth - transform.shipHealth) <= 5){
            transform.shipHealth = transform.targetHealth;
        }
    }
    else if(transform.targetHealth > transform.shipHealth){
        transform.shipHealth += 200*dt;
        if(abs(transform.targetHealth - transform.shipHealth) <= 5){
            transform.shipHealth = transform.targetHealth;
        }
    }
    //std::cout << transform.targetLaserBar << std::endl;
	
    }

    
    transform.position = transform.position + transform.velocity*dt;
    
	

	


};



};


struct InputComponent : public Component {

	using Component::Component; 
	raylib::BufferedInput* input;



	bool processInput = true;



	void tick(float dt){
        //std::cout << "hello?" << std::endl;

        auto rendRef = object->GetComponent<RenderingComponent>();

        if(!rendRef){
			std::cout << "No reference found" << std::endl;
			return;
		} 

        auto& render = rendRef->get();

        auto cam = render.cam;
        auto window = render.window;


		if(processInput){
            
			ProcessEntityInput(*object, *input, *cam, *window);
			//input.PollEvents();
		}
	}

};



int main() {

raylib::Window window(1000, 700, "CS381 - Assignment 7");
raylib::Text text;
int windHeight = 0, windWidth = 0;
bool playSound = false;
bool gameOver = false;
bool victory = false;
bool startGame = false;

//bool startFight = false;

float wait = 0;


//InitAudioDevice();

raylib::AudioDevice audio;

raylib::Music music("../sounds/corneria.mp3");
raylib::Sound ping("../sounds/starfoxsfx.mp3");
raylib::Sound laser("../sounds/laser.mp3");
raylib::Sound down("../sounds/playerdown.mp3");
raylib::Music death("../sounds/continue.mp3");
raylib::Music boss("../sounds/boss.mp3");
raylib::Music vict("../sounds/victory.mp3");

raylib::Color color1 = raylib::Color::Red();
raylib::Color color2 = raylib::Color::Orange();
raylib::Color color3 = raylib::Color::Yellow();
raylib::Color color4 = raylib::Color::Green();
raylib::Color color5 = raylib::Color::Blue();
raylib::Color color6 = raylib::Color::Purple();

//raylib::Model modelSpaceShip = LoadModel("../meshes/spaceship.glb");
//modelSpaceShip.transform = raylib::Transform(modelSpaceShip.transform).Scale(0.15, 0.15, 0.15);
//modelSpaceShip.transform = raylib::Transform(modelSpaceShip.transform).RotateY(raylib::Degree(135));

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

raylib::BufferedInput inputs;
std::vector<Particle> particles;

raylib::Camera camera(
        raylib::Vector3(0, 0, -350),
        raylib::Vector3(0, 0, 0),
        raylib::Vector3::Up(),
        45,
        CAMERA_PERSPECTIVE
    );

cs381::SkyBox skybox("textures/deepbluespace.png");

SetTargetFPS(60);


//Load all of our models : 
std::vector<Entity> entities;
Entity& space_ship = entities.emplace_back();

raylib::Vector3 cameraVelocity = {-50, 0, 0};


entities[0].AddComponent<InputComponent>();
entities[0].AddComponent<PhysicsComponent>();

entities[0].AddComponent<RenderingComponent>(raylib::Model("../meshes/spaceship.glb"));
entities[0].GetComponent<RenderingComponent>()->get().model2 = &modelShip;
//entities[0].GetComponent<RenderingComponent>()->get().model.transform = raylib::Transform(entities[0].GetComponent<RenderingComponent>()->get().model.transform).Scale(0.02, 0.02, 0.02);
//entities[0].GetComponent<RenderingComponent>()->get().model.transform = raylib::Transform(entities[0].GetComponent<RenderingComponent>()->get().model.transform).RotateY(raylib::Degree(270));
entities[0].GetComponent<RenderingComponent>()->get().isShip = true;
//entities[0].GetComponent<RenderingComponent>()->get().particles = &particles;

entities[0].GetComponent<TransformComponent>()->get().velocity = raylib::Vector3{-50, 0, 0};

entities[0].GetComponent<InputComponent>()->get().input = &inputs;
entities[0].GetComponent<RenderingComponent>()->get().cam = &camera;
entities[0].GetComponent<RenderingComponent>()->get().window = &window;
entities[0].GetComponent<RenderingComponent>()->get().laser = &laser;
entities[0].GetComponent<RenderingComponent>()->get().ping = &ping;

Entity& planet = entities.emplace_back();
entities[1].AddComponent<RenderingComponent>();
entities[1].GetComponent<TransformComponent>()->get().position = raylib::Vector3{0, 0, 600};
entities[1].GetComponent<RenderingComponent>()->get().model2 = &modelPlanet;
(*entities[1].GetComponent<RenderingComponent>()->get().model2).transform = raylib::Transform((*entities[1].GetComponent<RenderingComponent>()->get().model2).transform).Scale(50, 50, 50);
entities[1].GetComponent<RenderingComponent>()->get().isBackground = true;


Entity& meteor1 = entities.emplace_back();
Entity& meteor2 = entities.emplace_back();
Entity& meteor3 = entities.emplace_back();
Entity& meteor4 = entities.emplace_back();
Entity& meteor5 = entities.emplace_back();
Entity& meteor6 = entities.emplace_back();
Entity& meteor7 = entities.emplace_back();
Entity& meteor8 = entities.emplace_back();
Entity& meteor9 = entities.emplace_back();
Entity& meteor10 = entities.emplace_back();
Entity& meteor11 = entities.emplace_back();
Entity& meteor12 = entities.emplace_back();
Entity& meteor13 = entities.emplace_back();
Entity& meteor14 = entities.emplace_back();
Entity& meteor15 = entities.emplace_back();
Entity& meteor16 = entities.emplace_back();
Entity& meteor17 = entities.emplace_back();
Entity& meteor18 = entities.emplace_back();
Entity& meteor19 = entities.emplace_back();
Entity& meteor20 = entities.emplace_back();
Entity& meteor21 = entities.emplace_back();
Entity& meteor22 = entities.emplace_back();
Entity& meteor23 = entities.emplace_back();
Entity& meteor24 = entities.emplace_back();
Entity& meteor25 = entities.emplace_back();
Entity& meteor26 = entities.emplace_back();
Entity& meteor27 = entities.emplace_back();
Entity& meteor28 = entities.emplace_back();

Entity& meteor29 = entities.emplace_back();
Entity& meteor30 = entities.emplace_back();
Entity& meteor31 = entities.emplace_back();
Entity& meteor32 = entities.emplace_back();
Entity& meteor33 = entities.emplace_back();
Entity& meteor34 = entities.emplace_back();
Entity& meteor35 = entities.emplace_back();
Entity& meteor36 = entities.emplace_back();
Entity& meteor37 = entities.emplace_back();
Entity& meteor38 = entities.emplace_back();
Entity& meteor39 = entities.emplace_back();
Entity& meteor40 = entities.emplace_back();
Entity& meteor41 = entities.emplace_back();
Entity& meteor42 = entities.emplace_back();
Entity& meteor43 = entities.emplace_back();


entities[2].GetComponent<TransformComponent>()->get().position = raylib::Vector3{-700, 0, 0};
entities[3].GetComponent<TransformComponent>()->get().position = raylib::Vector3(-900, 50, 0);
entities[4].GetComponent<TransformComponent>()->get().position = raylib::Vector3{-1100, -20, 0};
entities[5].GetComponent<TransformComponent>()->get().position = raylib::Vector3(-1300, -90, 0);
entities[6].GetComponent<TransformComponent>()->get().position = raylib::Vector3{-1500, -30, 0};
entities[7].GetComponent<TransformComponent>()->get().position = raylib::Vector3(-1600, 60, 0);
entities[8].GetComponent<TransformComponent>()->get().position = raylib::Vector3{-1800, -90, 0};
entities[9].GetComponent<TransformComponent>()->get().position = raylib::Vector3(-2000, 80, 0);
entities[10].GetComponent<TransformComponent>()->get().position = raylib::Vector3{-2250, -70, 0};
entities[11].GetComponent<TransformComponent>()->get().position = raylib::Vector3(-2570, 50, 0);
entities[12].GetComponent<TransformComponent>()->get().position = raylib::Vector3{-2850, 90, 0};
entities[13].GetComponent<TransformComponent>()->get().position = raylib::Vector3(-3050, 0, 0);
entities[14].GetComponent<TransformComponent>()->get().position = raylib::Vector3{-3150, -95, 0};
entities[15].GetComponent<TransformComponent>()->get().position = raylib::Vector3(-3380, -50, 0);

entities[16].GetComponent<TransformComponent>()->get().position = raylib::Vector3{-3500, 74, 0};
entities[17].GetComponent<TransformComponent>()->get().position = raylib::Vector3(-3750, -80, 0);
entities[18].GetComponent<TransformComponent>()->get().position = raylib::Vector3{-3900, -20, 0};
entities[19].GetComponent<TransformComponent>()->get().position = raylib::Vector3(-4150, 20, 0);
entities[20].GetComponent<TransformComponent>()->get().position = raylib::Vector3{-4275, -90, 0};
entities[21].GetComponent<TransformComponent>()->get().position = raylib::Vector3(-4325, 45, 0);
entities[22].GetComponent<TransformComponent>()->get().position = raylib::Vector3{-4550, -60, 0};
entities[23].GetComponent<TransformComponent>()->get().position = raylib::Vector3(-4600, 0, 0);
entities[24].GetComponent<TransformComponent>()->get().position = raylib::Vector3{-4830, 60, 0};
entities[25].GetComponent<TransformComponent>()->get().position = raylib::Vector3(-5000, -50, 0);
entities[26].GetComponent<TransformComponent>()->get().position = raylib::Vector3{-5270, 50, 0};
entities[27].GetComponent<TransformComponent>()->get().position = raylib::Vector3(-5410, 80, 0);
entities[28].GetComponent<TransformComponent>()->get().position = raylib::Vector3{-5650, -15, 0};
entities[29].GetComponent<TransformComponent>()->get().position = raylib::Vector3(-5800, 23, 0);

entities[30].GetComponent<TransformComponent>()->get().position = raylib::Vector3{-6000, 69, 0};
entities[31].GetComponent<TransformComponent>()->get().position = raylib::Vector3(-6250, -90, 0);
entities[32].GetComponent<TransformComponent>()->get().position = raylib::Vector3{-6500, 0, 0};
entities[33].GetComponent<TransformComponent>()->get().position = raylib::Vector3(-6850, 30, 0);
entities[34].GetComponent<TransformComponent>()->get().position = raylib::Vector3{-7075, -20, 0};
entities[35].GetComponent<TransformComponent>()->get().position = raylib::Vector3(-7225, 75, 0);
entities[36].GetComponent<TransformComponent>()->get().position = raylib::Vector3{-7450, 10, 0};
entities[37].GetComponent<TransformComponent>()->get().position = raylib::Vector3(-7680, 35, 0);
entities[38].GetComponent<TransformComponent>()->get().position = raylib::Vector3{-7930, -10, 0};
entities[39].GetComponent<TransformComponent>()->get().position = raylib::Vector3(-8120, -50, 0);
entities[40].GetComponent<TransformComponent>()->get().position = raylib::Vector3{-8370, 50, 0};
entities[41].GetComponent<TransformComponent>()->get().position = raylib::Vector3(-8560, 80, 0);
entities[42].GetComponent<TransformComponent>()->get().position = raylib::Vector3{-8780, -35, 0};
entities[43].GetComponent<TransformComponent>()->get().position = raylib::Vector3(-8900, 23, 0);
entities[44].GetComponent<TransformComponent>()->get().position = raylib::Vector3(-9100, 90, 0);




//entities[1].AddComponent<InputComponent>();
//entities[1].AddComponent<PhysicsComponent>();

//std::cout << entities.size() << std::endl;

//raylib::Model* spacePointer = &(entities[0].GetComponent<RenderingComponent>()->get().model);

int changeTurn = 1;

for(int i = 2; i < entities.size(); i++){
    

auto ref = entities[i].GetComponent<TransformComponent>();

        if(!ref){
			std::cout << "No reference found" << std::endl;
			
		} 

auto& transform = ref->get();
transform.velocity = raylib::Vector3{50, 0, 0};

switch(changeTurn) {
    case 1: 
    transform.targetX = 30;
    break;
    case 2: 
    transform.targetY = 30;
    break;
    case 3:
    transform.targetZ = 30;
    break;
    case 4:
    transform.targetX = 30;
    transform.targetY = 30;
    break; 
    case 5: 
    transform.targetX = 30;
    transform.targetZ = 30;
    break;
    case 6: 
    transform.targetY = 30;
    transform.targetZ = 30;
    break;
    case 7: 
    transform.targetX = 30;
    transform.targetY = 30; 
    transform.targetZ = 30;
    break;
}
changeTurn++;
if(changeTurn > 7){
    changeTurn = 1;
}


entities[i].AddComponent<PhysicsComponent>();
entities[i].AddComponent<RenderingComponent>(raylib::Model());
entities[i].GetComponent<RenderingComponent>()->get().model2 = &modelMeteor;
entities[i].GetComponent<RenderingComponent>()->get().cam = &camera;
entities[i].GetComponent<RenderingComponent>()->get().window = &window;
entities[i].GetComponent<RenderingComponent>()->get().ping = &ping;

entities[i].GetComponent<RenderingComponent>()->get().shipObject = &entities[0];


//entities[i].GetComponent<TransformComponent>()
}


//PERFORM REMAINING METEOR OPERATIONS 
entities[3].GetComponent<TransformComponent>()->get().scale = raylib::Vector3{1.3, 1.3, 1.3};
//entities[3].GetComponent<RenderingComponent>()->get().model.transfor = raylib::Transform(entities[3].GetComponent<RenderingComponent>()->get().model.transform).Scale(1.3, 1.3, 1.3);
entities[5].GetComponent<TransformComponent>()->get().scale = raylib::Vector3{1.6, 1.6, 1.6};
//entities[5].GetComponent<RenderingComponent>()->get().model.transform = raylib::Transform(entities[5].GetComponent<RenderingComponent>()->get().model.transform).Scale(1.6, 1.6, 1.6);
entities[6].GetComponent<TransformComponent>()->get().scale = raylib::Vector3{1.4, 1.4, 1.4};
//entities[6].GetComponent<RenderingComponent>()->get().model.transform = raylib::Transform(entities[6].GetComponent<RenderingComponent>()->get().model.transform).Scale(1.4, 1.4, 1.4);
entities[13].GetComponent<TransformComponent>()->get().scale = raylib::Vector3{1.2, 1.2, 1.2};
//entities[13].GetComponent<RenderingComponent>()->get().model.transform = raylib::Transform(entities[13].GetComponent<RenderingComponent>()->get().model.transform).Scale(1.2, 1.2, 1.2);
entities[17].GetComponent<TransformComponent>()->get().scale = raylib::Vector3{1.4, 1.4, 1.4};
//entities[17].GetComponent<RenderingComponent>()->get().model.transform = raylib::Transform(entities[17].GetComponent<RenderingComponent>()->get().model.transform).Scale(1.4, 1.4, 1.4);
entities[20].GetComponent<TransformComponent>()->get().scale = raylib::Vector3{1.5, 1.5, 1.5};
//entities[20].GetComponent<RenderingComponent>()->get().model.transform = raylib::Transform(entities[20].GetComponent<RenderingComponent>()->get().model.transform).Scale(1.5, 1.5, 1.5);
entities[24].GetComponent<TransformComponent>()->get().scale = raylib::Vector3{2, 2, 2};
//entities[24].GetComponent<RenderingComponent>()->get().model.transform = raylib::Transform(entities[24].GetComponent<RenderingComponent>()->get().model.transform).Scale(2, 2, 2);
entities[25].GetComponent<TransformComponent>()->get().scale = raylib::Vector3{1.5, 1.5, 1.5};
//entities[25].GetComponent<RenderingComponent>()->get().model.transform = raylib::Transform(entities[25].GetComponent<RenderingComponent>()->get().model.transform).Scale(1.5, 1.5, 1.5);
entities[27].GetComponent<TransformComponent>()->get().scale = raylib::Vector3{1.3, 1.3, 1.3};
//entities[27].GetComponent<RenderingComponent>()->get().model.transform = raylib::Transform(entities[27].GetComponent<RenderingComponent>()->get().model.transform).Scale(1.3, 1.3, 1.3);
entities[28].GetComponent<TransformComponent>()->get().scale = raylib::Vector3{1.4, 1.4, 1.4};
//entities[28].GetComponent<RenderingComponent>()->get().model.transform = raylib::Transform(entities[28].GetComponent<RenderingComponent>()->get().model.transform).Scale(1.4, 1.4, 1.4);

entities[30].GetComponent<TransformComponent>()->get().scale = raylib::Vector3{1.3, 1.3, 1.3};
//entities[30].GetComponent<RenderingComponent>()->get().model.transform = raylib::Transform(entities[30].GetComponent<RenderingComponent>()->get().model.transform).Scale(1.3, 1.3, 1.3);
//entities[30].GetComponent<RenderingComponent>()->get().position = raylib::Vector3(-6250, -90, 0);

entities[32].GetComponent<TransformComponent>()->get().scale = raylib::Vector3{1.6, 1.6, 1.6};
//entities[32].GetComponent<RenderingComponent>()->get().model.transform = raylib::Transform(entities[32].GetComponent<RenderingComponent>()->get().model.transform).Scale(1.6, 1.6, 1.6);
entities[33].GetComponent<TransformComponent>()->get().scale = raylib::Vector3{1.2, 1.2, 1.2};
//entities[33].GetComponent<RenderingComponent>()->get().model.transform = raylib::Transform(entities[33].GetComponent<RenderingComponent>()->get().model.transform).Scale(1.2, 1.2, 1.2);
//entities[33].GetComponent<RenderingComponent>()->get().position = raylib::Vector3{-7075, -20, 0};
//entities[34].GetComponent<RenderingComponent>()->get().position = raylib::Vector3(-7225, 75, 0);

entities[36].GetComponent<TransformComponent>()->get().scale = raylib::Vector3{1.4, 1.4, 1.4};
//entities[36].GetComponent<RenderingComponent>()->get().model.transform = raylib::Transform(entities[36].GetComponent<RenderingComponent>()->get().model.transform).Scale(1.4, 1.4, 1.4);
entities[37].GetComponent<TransformComponent>()->get().scale = raylib::Vector3{1.3, 1.3, 1.3};
//entities[37].GetComponent<RenderingComponent>()->get().model.transform = raylib::Transform(entities[37].GetComponent<RenderingComponent>()->get().model.transform).Scale(1.3, 1.3, 1.3);
//entities[37].GetComponent<RenderingComponent>()->get().position = raylib::Vector3{-7930, -10, 0};

entities[39].GetComponent<TransformComponent>()->get().scale = raylib::Vector3{1.5, 1.5, 1.5};
//entities[39].GetComponent<RenderingComponent>()->get().model.transform = raylib::Transform(entities[39].GetComponent<RenderingComponent>()->get().model.transform).Scale(1.5, 1.5, 1.5);

entities[40].GetComponent<TransformComponent>()->get().scale = raylib::Vector3{1.2, 1.2, 1.2};
//entities[40].GetComponent<RenderingComponent>()->get().model.transform = raylib::Transform(entities[40].GetComponent<RenderingComponent>()->get().model.transform).Scale(1.2, 1.2, 1.2);
//entities[40].GetComponent<RenderingComponent>()->get().position = raylib::Vector3(-8560, 80, 0);
entities[42].GetComponent<TransformComponent>()->get().scale = raylib::Vector3{1.3, 1.3, 1.3};
//entities[42].GetComponent<RenderingComponent>()->get().model.transform = raylib::Transform(entities[42].GetComponent<RenderingComponent>()->get().model.transform).Scale(1.3, 1.3, 1.3);
//entities[42].GetComponent<RenderingComponent>()->get().position = raylib::Vector3(-8900, 23, 0);
entities[44].GetComponent<TransformComponent>()->get().scale = raylib::Vector3{1.6, 1.6, 1.6};
//entities[44].GetComponent<RenderingComponent>()->get().model.transform = raylib::Transform(entities[44].GetComponent<RenderingComponent>()->get().model.transform).Scale(1.6, 1.6, 1.6);


entities[3].GetComponent<TransformComponent>()->get().velocity = raylib::Vector3(30, 0, 0);
entities[4].GetComponent<TransformComponent>()->get().velocity = raylib::Vector3(60, 0, 0);
entities[6].GetComponent<TransformComponent>()->get().velocity = raylib::Vector3(30, 0, 0);
entities[10].GetComponent<TransformComponent>()->get().velocity = raylib::Vector3(30, 0, 0);
entities[13].GetComponent<TransformComponent>()->get().velocity = raylib::Vector3(70, 0, 0);
entities[15].GetComponent<TransformComponent>()->get().velocity = raylib::Vector3(35, 0, 0);
entities[20].GetComponent<TransformComponent>()->get().velocity = raylib::Vector3(60, 0, 0);

Entity& enemy = entities.emplace_back();
entities[45].AddComponent<PhysicsComponent>();
entities[45].AddComponent<RenderingComponent>();
entities[45].GetComponent<TransformComponent>()->get().position = raylib::Vector3{-4600, 0, 50};
entities[45].GetComponent<TransformComponent>()->get().targetTurn = -30.0f;
entities[45].GetComponent<RenderingComponent>()->get().model2 = &modelBoss;
entities[45].GetComponent<TransformComponent>()->get().scale = raylib::Vector3{9, 9, 9};
entities[45].GetComponent<RenderingComponent>()->get().isBoss = true;

entities[45].GetComponent<RenderingComponent>()->get().cam = &camera;
entities[45].GetComponent<RenderingComponent>()->get().window = &window;
entities[45].GetComponent<RenderingComponent>()->get().laser = &laser;
entities[45].GetComponent<RenderingComponent>()->get().ping = &ping;
entities[45].GetComponent<RenderingComponent>()->get().shipObject = &entities[0];

entities[0].GetComponent<RenderingComponent>()->get().shipObject = &entities[45];
/*entities[1].AddComponent<RenderingComponent>(std::move(modelMeteor));
entities[2].AddComponent<RenderingComponent>(std::move(modelMeteor));*/



std::cout << "Test" << std::endl;
//entities[1].GetComponent<RenderingComponent>()->get().model

float musicVal = 0.5f;
int selectTrack = 1;


while(!window.ShouldClose()) { 

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


if(!gameOver){
 camera.position = (raylib::Vector3)camera.position + cameraVelocity*window.GetFrameTime();
 
 if(!victory){
 camera.target = (raylib::Vector3)camera.target + cameraVelocity*window.GetFrameTime();
 }
 else if(victory){
 camera.target = (raylib::Vector3)camera.target + (((raylib::Vector3)entities[0].GetComponent<TransformComponent>()->get().position - (raylib::Vector3)camera.target) * window.GetFrameTime());
 if(camera.fovy >= 30.0f){
    camera.fovy -= 20*window.GetFrameTime();
 }
 }
 music.Play();
 music.Update();
}

 window.BeginDrawing();
 {

  windWidth = window.GetWidth(); 
  windHeight = window.GetHeight();

  window.ClearBackground(BLACK);

  camera.BeginMode();
  {
  

  

  music.SetVolume(musicVal);
  ping.SetVolume(0.5f);
  laser.SetVolume(0.5f);
  down.SetVolume(0.5f);
  death.SetVolume(0.5f);
  boss.SetVolume(0.5f);

  skybox.Draw();
 

  if(!gameOver && !victory){
    if((entities[0].GetComponent<TransformComponent>()->get().position.x < -4300.0f) && selectTrack == 1){
        if(musicVal > 0.0f){
            musicVal -= 0.5*window.GetFrameTime();
        }
        if(entities[0].GetComponent<TransformComponent>()->get().position.x < -4600.0f){
            selectTrack = 2;
        }
    }

    switch(selectTrack){
    case 1: 
    music.Play();
    music.Update();
    break;

    case 2:
    boss.Play();
    boss.Update();
    break;
    }
    
  if(!(entities[45].GetComponent<RenderingComponent>()->get().startFight)){
   if(entities[0].GetComponent<TransformComponent>()->get().position.x <= -4600.0f && (!(entities[45].GetComponent<RenderingComponent>()->get().startFight))){
    entities[45].GetComponent<TransformComponent>()->get().velocity.x = -150;
    entities[45].GetComponent<TransformComponent>()->get().targetHealth = 1.0f;
   }

   if(entities[0].GetComponent<TransformComponent>()->get().position.x <= -5000.0f){
    entities[45].GetComponent<TransformComponent>()->get().position = raylib::Vector3{(entities[0].GetComponent<TransformComponent>()->get().position.x - 500.0f), 0, 0};
    entities[45].GetComponent<TransformComponent>()->get().velocity.x = 0.0f;
    (*entities[45].GetComponent<RenderingComponent>()->get().model2).transform = raylib::Transform((*entities[45].GetComponent<RenderingComponent>()->get().model2).transform).RotateY(raylib::Degree(180));
    entities[45].GetComponent<RenderingComponent>()->get().startFight = true;
    entities[0].GetComponent<RenderingComponent>()->get().startFight = true;
    entities[45].GetComponent<TransformComponent>()->get().targetHealth = 300.0f;
   }
  }

  if(entities[45].GetComponent<RenderingComponent>()->get().startFight){
  if(GetWorldToScreen(entities[45].GetComponent<TransformComponent>()->get().position, camera).x < window.GetWidth() - 100){
    entities[45].GetComponent<TransformComponent>()->get().velocity.x = -50.0f;
    float dif = (entities[0].GetComponent<TransformComponent>()->get().position.y) - (entities[45].GetComponent<TransformComponent>()->get().position.y);
    if(dif > 0 ){
        entities[45].GetComponent<TransformComponent>()->get().targetTurn = 30.0f;
    }
    else if(dif < 0){
        entities[45].GetComponent<TransformComponent>()->get().targetTurn = -30.0f;
    }
    entities[45].GetComponent<TransformComponent>()->get().position.y += dif * window.GetFrameTime();
    if(entities[45].GetComponent<TransformComponent>()->get().targetLaserBar == 300.0f){
        entities[45].GetComponent<RenderingComponent>()->get().drawLaser = true;
        entities[45].GetComponent<TransformComponent>()->get().targetLaserBar = 0.0f;
        laser.Play();
    }
    std::cout << "appearing" << std::endl;
  }
  }


  for(int i = 0; i < entities.size(); i++){
	entities[i].tick(window.GetFrameTime());
    if(entities[i].GetComponent<RenderingComponent>()->get().modelUnloaded == true){
        //entities.erase(entities.begin() + i);
    }
  }
  inputs.PollEvents();
  }
  else if(gameOver){
    
    wait += 5*window.GetFrameTime();
    if(wait > 25.0f){
    death.Play();
    death.Update();
    }
    
    for(int i = 0; i < entities.size(); i++){
        entities[i].GetComponent<RenderingComponent>()->get().gameOver = true;
        entities[i].GetComponent<RenderingComponent>()->get().tick(window.GetFrameTime());
    }

  }
  else if(victory){
    wait += 5*window.GetFrameTime();
    if(wait > 15.0f){
    vict.Play();
    vict.Update();
    }
    for(int i = 0; i < entities.size(); i++){
        entities[i].GetComponent<RenderingComponent>()->get().gameOver = true;
        entities[i].GetComponent<TransformComponent>()->get().tick(window.GetFrameTime());
        entities[i].GetComponent<PhysicsComponent>()->get().tick(window.GetFrameTime());
        entities[i].GetComponent<RenderingComponent>()->get().tick(window.GetFrameTime());
    }

  }








  }
  camera.EndMode();
  auto shipDamage = entities[0].GetComponent<TransformComponent>()->get().shipHealth;
  auto shipDamage2 = entities[45].GetComponent<TransformComponent>()->get().shipHealth;

if(!victory){
  //HUD/State machine handler
DrawRectangle(30 , window.GetHeight() / 4 - 10, 300, 20, raylib::Color(GRAY));
DrawRectangle(30 , window.GetHeight() / 4 - 10, entities[0].GetComponent<TransformComponent>()->get().targetLaserBar, 20, raylib::Color(GREEN));



 text.Draw("Ship Health: ", (30), (70), 40, raylib::Color::White());
 text.Draw("Laser: ", (30), (140), 20, raylib::Color::White());

 DrawRectangle(30 , window.GetHeight() / 6 - 10, 300, 20, raylib::Color(GRAY));

 
 if(shipDamage > 200 && shipDamage <= 300){
 DrawRectangle(30 , window.GetHeight() / 6 - 10, shipDamage, 20, raylib::Color(GREEN));
 }

 else if(shipDamage > 100 && shipDamage <= 200){
 DrawRectangle(30 , window.GetHeight() / 6 - 10, shipDamage, 20, raylib::Color(YELLOW));
 }

 else if(shipDamage > 50 && shipDamage <= 100){
  DrawRectangle(30 , window.GetHeight() / 6 - 10, shipDamage, 20, raylib::Color(ORANGE));
 }

 else if(shipDamage > 0 && shipDamage <= 50){
  DrawRectangle(30 , window.GetHeight() / 6 - 10, shipDamage, 20, raylib::Color(RED));
 }

 else{
  //text.Draw("DESTROYED", (30), (110), 40, raylib::Color::Red());
  //shipDestroyed = true;
  
  //down.Update();
  if(!gameOver){
  down.Play();
  gameOver = true;
  }

  text.Draw("DESTORYED ", (30), (100), 40, raylib::Color::Red());
 }


 if(entities[45].GetComponent<RenderingComponent>()->get().startFight == true){

    text.Draw("BOSS: ", (window.GetWidth() - 400), (70), 40, raylib::Color::White());
    //text.Draw("Laser: ", (30), (140), 20, raylib::Color::White());

    DrawRectangle(window.GetWidth() - 400, window.GetHeight() / 6 - 10, 300, 20, raylib::Color(GRAY));

 
    if(shipDamage2 > 200 && shipDamage2 <= 300){
     DrawRectangle(window.GetWidth() - 400, window.GetHeight() / 6 - 10, shipDamage2, 20, raylib::Color(GREEN));
    }

    else if(shipDamage2 > 100 && shipDamage2 <= 200){
     DrawRectangle(window.GetWidth() - 400, window.GetHeight() / 6 - 10, shipDamage2, 20, raylib::Color(YELLOW));
    }

    else if(shipDamage2 > 50 && shipDamage2 <= 100){
     DrawRectangle(window.GetWidth() - 400, window.GetHeight() / 6 - 10, shipDamage2, 20, raylib::Color(ORANGE));
    }

    else if(shipDamage2 > 0 && shipDamage2 <= 50){
     DrawRectangle(window.GetWidth() - 400, window.GetHeight() / 6 - 10, shipDamage2, 20, raylib::Color(RED));
    }

    else{
        victory = true;
        entities[45].GetComponent<RenderingComponent>()->get().invisible = true;
    }

 }

}
else{
    /*text.Draw("C", (window.GetWidth() / 2) + (30), (100), 40, color1);
    text.Draw("O", (window.GetWidth() / 2) + (60), (100), 40, color2);
    text.Draw("N", (window.GetWidth() / 2) + (90), (100), 40, color3);
    text.Draw("G", (window.GetWidth() / 2) + (120), (100), 40, color4);
    text.Draw("R", (window.GetWidth() / 2) + (150), (100), 40, color5);
    text.Draw("A", (window.GetWidth() / 2) + (180), (100), 40, color6);
    text.Draw("T", (window.GetWidth() / 2) + (210), (100), 40, color1);
    text.Draw("U", (window.GetWidth() / 2) + (240), (100), 40, color2);
    text.Draw("L", (window.GetWidth() / 2) + (270), (100), 40, color3);
    text.Draw("A", (window.GetWidth() / 2) + (300), (100), 40, color4);
    text.Draw("T", (window.GetWidth() / 2) + (330), (100), 40, color5);
    text.Draw("I", (window.GetWidth() / 2) + (370), (100), 40, color6);
    text.Draw("O", (window.GetWidth() / 2) + (395), (100), 40, color1);
    text.Draw("N", (window.GetWidth() / 2) + (425), (100), 40, color2);
    text.Draw("S", (window.GetWidth() / 2) + (455), (100), 40, color3);*/

    text.Draw("Returning to ",  (window.GetWidth() / 2) + 60, 100, 40, raylib::Color::Blue());
    text.Draw("base ",  (window.GetWidth() / 2) + 60, 130, 40, raylib::Color::Green());
}

 }


 window.EndDrawing();
 }





}

return 0;
}

void DrawBoundedModel(raylib::Model& model, Transformer auto transformer) {
	raylib::Transform backupTransform = model.transform;
	model.transform = transformer(backupTransform);
	model.Draw({});
	model.GetTransformedBoundingBox().Draw();
	model.transform = backupTransform;
}

void DrawModel(raylib::Model& model, Transformer auto transformer) {
	raylib::Transform backupTransform = model.transform;
	model.transform = transformer(backupTransform);
	model.Draw({});
	model.transform = backupTransform;
}

bool ProcessEntityInput(Entity& entity, raylib::BufferedInput& inputs, Camera& cam, raylib::Window& window){

  //std::cout << "hello?" << std::endl;
  static bool spacePressedLastFrame = false;

  auto ref = entity.GetComponent<TransformComponent>();
  

		//Because its optional reference, may or may not exist. 
		//If it doesn't exist, we return.
	if(!ref){
		std::cout << "No reference found" << std::endl;
		return false;
	} 

    
	
  auto& transform = ref->get();



    //std::cout << GetWorldToScreen(transform.position, cam).y << std::endl;
  

  
  inputs["moveUp"] = raylib::Action::key(KEY_W).SetPressedCallback([&transform, &cam]{
    if((GetWorldToScreen(transform.position, cam).y <= 0.0f)){

    }
	
    else{
        transform.position.y += 130 * transform.dt_;
        transform.targetTurn = 30;
    }
		
		
	}).move();
  

  inputs["moveDown"] = raylib::Action::key(KEY_S).SetPressedCallback([&transform, &cam, &window]{
		
    if(GetWorldToScreen(transform.position, cam).y > window.GetHeight()){
  

    }

    else{
        transform.position.y -= 130 * transform.dt_;
        transform.targetTurn = -30;
    }

	
		
	}).move();

  inputs["moveLeft"] = raylib::Action::key(KEY_A).SetPressedCallback([&transform, &cam, &window]{
		
    if(GetWorldToScreen(transform.position, cam).x < 0){
        

    }

    else{
        transform.position.x += 130 * transform.dt_;
    }

		
		
	}).move();
	
  bool boost = false;
  inputs["boost"] = raylib::Action::key(KEY_SPACE).SetPressedCallback([&boost]{
    boost = true;
  }).move();

  inputs["moveRight"] = raylib::Action::key(KEY_D).SetPressedCallback([&transform, &cam, &window, &boost]{
		
        if(GetWorldToScreen(transform.position, cam).x > window.GetWidth()){

        }

        else{
            
            transform.position.x -= 130 * transform.dt_;
            //if(boost) std::cout << "boost" << std::endl;
        }
		
		
	}).move();

  inputs["moveCameraUp"] = raylib::Action::key(KEY_UP).SetPressedCallback([&cam]{
		
		//cam.position.y += 2;
		
  }).move();

  inputs["moveCameraDown"] = raylib::Action::key(KEY_DOWN).SetPressedCallback([&cam]{
		
		//cam.position.y -= 2;
		
  }).move();

  
  //bool fire = (transform.targetLaserBar == 300.0f);


  //std::cout << transform.targetLaserBar << std::endl;
  

    inputs["fireLaser"] = raylib::Action::key(KEY_SPACE).SetPressedCallback([&entity, &transform]{
        auto rendRef = entity.GetComponent<RenderingComponent>();
        if(!rendRef){
		std::cout << "No reference found" << std::endl;
        }
        auto& render = rendRef->get();

        std::cout << transform.targetLaserBar << std::endl;
        if(!(transform.targetLaserBar < 300.0f)){
        render.drawLaser = true;
        (*render.laser).Play(); 
        transform.targetLaserBar = 0;
        }

    }).move();
  



  
  

  //spacePressedLastFrame = IsKeyDown(KEY_SPACE);



  /*if(IsKeyDown(KEY_W)){
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

  
  

 }*/










    return false;
}

bool ProcessEntityPhysics(Entity& entity){

    static constexpr auto AngleClamp = [](raylib::Degree angle) -> raylib::Degree {
		//Splits the heading into two values. Both whole and decimal.
		float decimal = float(angle) - int(angle);
		int whole = int(angle) % 360;
		whole += (whole < 0) * 360;
		return decimal + whole;
	};

    auto ref = entity.GetComponent<TransformComponent>();

		//Because its optional reference, may or may not exist. 
		//If it doesn't exist, we return.
	if(!ref){
		std::cout << "No reference found" << std::endl;
		return false;
	} 
	
    auto& transform = ref->get();

    float target = AngleClamp(transform.targetTurn);
	float difference = abs(target - transform.turn);

    if(transform.targetTurn > transform.turn){
        transform.turn += transform.acceleration * transform.dt_;
    } 

    else if(transform.targetTurn < transform.turn){
        transform.turn -= transform.acceleration * transform.dt_;
    }

    if(difference <= .5) transform.turn = target;



    return false;
}