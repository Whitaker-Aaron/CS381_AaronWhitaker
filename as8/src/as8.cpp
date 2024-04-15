#include <iostream>
#include "raylib-cpp.hpp"
#include "rlgl.h"
#include "raymath.h"
#include <vector>
#include <memory>
#include <optional>
#include <concepts>
//#include "delegate.hpp"
#include <BufferedInput.hpp>
#include <ranges>
#include <string>
#include <cassert>
//#include "counter.cpp"

// #include "inputs.hpp"
#include "skybox.hpp"

template<typename T> 
concept Transformer = requires(T t, raylib::Transform m) {
    { t.operator()(m) } -> std::convertible_to<raylib::Transform>;
};



float DegreeToRadian(raylib::Degree angle){
    return ((float)angle)*(std::numbers::pi / 180.0f);
}



//Structure of arrays
//This way all of our data is right next to each other, instead of being a whole struct away. 
//Helps our cache access.
struct CalculateVelocityParams {
    static constexpr float acceleration = 5; 
    static constexpr float angularAcceleration = 15; 

    std::array<raylib::Vector3, 10> position;
    std::array<float, 10> targetSpeed; 
    std::array<raylib::Degree, 10> targetHeading;
};

//256 entities
using Entity = uint8_t; 

//Can only store one type of thing 

//Can't have different template types. Need to template the functions inside.
struct ComponentStorage {
    size_t elementSize = -1; 
    //Can't use auto so we use std::byte
    std::vector<std::byte> data; 

    //Data is invalid. Fill data will all zeroes.
    ComponentStorage() : elementSize(-1), data(1, std::byte{0}) {}


    //ElementSize is the element size we were given. 
    //Lets us store 5 of the data type initially.
    ComponentStorage(size_t elementSize) : elementSize(elementSize) {data.resize(5 * elementSize); }

    //OR


    //We can ask for a Tcomponent 
    //Default argument will be whatever its default is.
    template<typename Tcomponent> 
    ComponentStorage(Tcomponent reference = {}) : ComponentStorage(sizeof(Tcomponent)) {}


    //ComponentStorage(5);
    //ComponentStorage<int>();



    template<typename Tcomponent> 
    Tcomponent& Get(Entity e) {
        //Assert takes a bool and if bool false, compile time error message.
        //Size of component is the same size that we're tracking.
        assert(sizeof(Tcomponent) == elementSize);
        assert(e < (data.size() / elementSize));
        //Data.data() returns a pointer to the vectors interal array.
        //So a vector of arrays. Then we offset by e, with an index of elementsize
        //Incrementing through the actual bytes of the array.
        return *(Tcomponent*)(data.data() + e * elementSize);
    }


    //When we allocate, we can ask for more than one thing at a time to be created.
    //Default to 1.
    //
    template<typename Tcomponent> 
    std::pair<Tcomponent&, Entity> Allocate(size_t count = 1){
        //Component is same size
        assert(sizeof(Tcomponent) == elementSize);
        //8 bit integer as our entity index. Max number we can store in 8 bit is 256.
        assert(count < 255);
        //We're saving how many bytes were in the original vector.
        //Checkpointing where the vector ended so we can use this as a start point.
        auto originalEnd = data.size();
        //Insert enough bytes to hold all the new things we are allocating.
        //0 everything.
        data.insert(data.end(), elementSize * count, std::byte{0});
        //Skip last one
        for(size_t i = 0; i < count - 1; i++){
            //Create something called a placement.
            //If you call new as a function, we can pass it a pointer to an array.
            //Will allocate the thing in the array.
            //Create a new Tcomponent here.
            //Calls the default constructor.
            new(data.data() + originalEnd + i * elementSize) Tcomponent(); 
        }
        return { 
            //Save the pointer of the last element and return it to the user.
            *new(data.data() + data.size() - elementSize) Tcomponent(), 
            data.size() / elementSize
        };
    }

    template<typename Tcomponent> 
    Tcomponent& GetOrAllocate(Entity e) { 
        assert(sizeof(Tcomponent) == elementSize);
        //Size of our array in tcomponents, not in bytes.
        size_t size = data.size() / elementSize; 
        //If size is smaller than what we are asking for, then we need to allocate more elements.
        if(size <= e){
            Allocate<Tcomponent>(std::max<int64_t>(int64_t(e) - size, 1));
        }
        return Get<Tcomponent>(e);
    }
};

//The entity in this architecture is the index into the arrays.
//Component becomes the type of one of those arrays.
//Idiomatically (lol) components are also just data. No implementation.
//Systems that bulk act on components, and implement behavior.
//Need to build; therefore Scene 

//extern means this variable is declared in another .o file.
//When our program starts, we make a variable, we set its value to 0.
extern size_t globalComponentCounter;

template<typename T>
size_t GetComponentID(T reference = {}) {
    //Because we are templating, this will always return the same ID because are creating new instances.
    //We increment the global variable, so that when we call the ID again, we get the same ID, and are 
    //able to generate new IDs for other components.
    //This code only gets set during first run.
    static size_t id = globalComponentCounter++;
    return id;
}


struct Scene { 
    std::vector<std::vector<bool>> entityMasks;

    //Want storage for transforms. What index is it?
    std::vector<ComponentStorage> storages = {ComponentStorage()};

    template<typename Tcomponent>
    ComponentStorage& GetStorage() {
        size_t id = GetComponentID<Tcomponent>();
        if(storages.size() <= id){
            //Add to storage 
            storages.insert(storages.cend(), std::max<int64_t>(id - storages.size(), 1), ComponentStorage());
        }
        if(storages[id].elementSize == std::numeric_limits<size_t>::max()){
            //Means our storage was default constructed.
            //Construct using type.
            //If we made one for a number that might be higher, then we would have the space allocated, but not 
            //initialized. This checks that.
            storages[id] = ComponentStorage(Tcomponent{});
        }
        return storages[id];

       


    }

     Entity CreateEntity() {
            //A mask is a bunch of rules. 
            //Represents what we have data for. Aka which components they have.
            Entity e = entityMasks.size();
            //Initialize entire vector to false.
            entityMasks.emplace_back(std::vector<bool>{false});
            return e;
        }

     template<typename Tcomponent>
     Tcomponent& AddComponent(Entity e) {
        size_t id = GetComponentID<Tcomponent>();
        auto& eMask = entityMasks[e];
        //Keep following pattern where if we don't have the index for the appropriate action, 
        //we make it large enough to fit the index. Which is why resize in this case.
        if(eMask.size() <= id){
            eMask.resize(id + 1, false);
        }
        eMask[id] = true;
        //Allocate space for component if not already existant.
        //Return reference so someone can manipulate.
        return GetStorage<Tcomponent>().template GetOrAllocate<Tcomponent>(e);
     }

     template<typename Tcomponent> 
     Tcomponent& GetComponent(Entity e){
        size_t id = GetComponentID<Tcomponent>();
        assert(entityMasks[e][id]);
        return GetStorage<Tcomponent>().template Get<Tcomponent>(e);
     }

     template<typename Tcomponent>
     bool HasComponent(Entity e){
        size_t id = GetComponentID<Tcomponent>();
        return entityMasks.size() > e && entityMasks[e].size() > id && entityMasks[e][id];
     }

};

void DrawBoundedModel(raylib::Model& model, Transformer auto transformer);
void DrawModel(raylib::Model& model, Transformer auto transformer);

struct Rendering { 
    raylib::Model* model;
    bool drawBoundingBox = false;
};

struct Transforming {

    raylib::Vector3 position = {0, 0, 0};
 
};

struct Velocity { 
    raylib::Vector3 velocity = raylib::Vector3::Zero();

       float speed = 0, targetSpeed = 0;

       float maxSpeed = 50;
	   float minSpeed = 0;

       float acceleration = 5; 
	   float angularAcceleration = 10;
};

struct Physics2D {
    raylib::Degree heading = 0, targetHeading = 0;
};

struct Physics3D {
    raylib::Quaternion quat; 
    raylib::Degree pitch = 0; 
    raylib::Degree yaw = 0; 
    raylib::Degree roll = 0;

};

struct Input {
    raylib::BufferedInput* input;
};



raylib::Vector3 CalculateEntityVelocity(Velocity& velocity){
    return velocity.velocity;
}


//Problem with input handling
//Component/System --> if if if if if if if if
//Input maybe 1/2 a second. We're doing frames 60 times a second. Input happens less than 10% of the time. 
//We have these if statements happening every frame checking if something is happening 10% of the time. 
//CPU: Fetch Decode Execute
//While we decode we start to fetch the next instruction. 
//Then while decoding it executes. PIPELINING 
//When we jump, the entire pipeline gets invalidated. We need to flush the pipeline and start it over upon return.
//This is why we parallelize the CPU pipeline. Couple hundred cpu cycles for a cache miss alone.
//When we jump to a segment that hasn't been loaded yet, then now we have a cache miss almost guaranteed kinda.
//
//Polling is checking if true/false on input press.
//Event Based is almost the same, except now we have an input manager.
//input.PollEvents()
//Behind the secenes still has a giant block of if statements.
//An example other than input.
//Going to be checking every other frame.
//Polling for the information.
//Function calls when it does happen.
//How do we implement this?
//
//We can use delegates. Delegate is a list of function pointers.
//We want to avoid invalidating our cpu pipeline and repeating logic checks. DRY.
//
//What are the functions called when an event happens occurs?
//
//Callback
//
//Some function that we call and we get a response back.
//Name works less with modern event driven program. 
//More like event listener kinda.
//
//Set Callback means we're setting the one callback
//Add callback we don't care how many other functions are getting called, we want this one called as well.
//
//Action. What is an action?
//Any time user does something.
//Release key. Action baby.
//When we're polling, there are places all over the code where we check.
//If user rebinds their keys, less room to messup.
//
//We setup actions --> and then components/systems register(add)  callbacks.
//Server code written in main.
//
//Control flow is more confusing in an event baesd system.
//
//3D physics
//

void AddBuffer(int e, Input& input, Velocity& velocity, Rendering& render, Physics2D* physics2D, Physics3D* physics3D){


    (*(input.input))["increaseVelocity"].AddPressedCallback([&velocity, &render]{
		
      if(render.drawBoundingBox){
 		
        velocity.targetSpeed += 1;

      }
    }).move();

    
    (*(input.input))["decreaseVelocity"].AddPressedCallback([&velocity, &render]{
		
      if(render.drawBoundingBox){
 		
        velocity.targetSpeed -= 1;
        
      }
    }).move();

   (*(input.input))["stop"].AddPressedCallback([&velocity, &render]{
		if(render.drawBoundingBox){
 		
        velocity.targetSpeed = 0;
        
      }
	}).move();

    if(physics3D == nullptr && physics2D != nullptr){
    (*(input.input))["increaseHeading"].AddPressedCallback([&render, physics2D]{
		if(render.drawBoundingBox){
 		
        (*physics2D).targetHeading += 5;

      }
	}).move();
    }

    if(physics2D == nullptr && physics3D != nullptr){
    (*(input.input))["decreaseHeading"].AddPressedCallback([&render, physics3D]{
		if(render.drawBoundingBox){
 		
        (*physics3D).yaw -= 5;

      }
	}).move();
    }

    if(physics2D == nullptr && physics3D != nullptr){
    (*(input.input))["increaseHeading"].AddPressedCallback([&render, physics3D]{
		if(render.drawBoundingBox){
 		
        (*physics3D).yaw += 5;

        }
	}).move();
    }

    if(physics3D == nullptr && physics2D != nullptr){
    (*(input.input))["decreaseHeading"].AddPressedCallback([&render, physics2D]{
		if(render.drawBoundingBox){
 		
        (*physics2D).targetHeading -= 5;
     
        }
	}).move();
    }

    if(physics2D == nullptr && physics3D != nullptr){
    (*(input.input))["increasePitch"].AddPressedCallback([&render, physics3D]{
		if(render.drawBoundingBox){
 		
        (*physics3D).pitch += 5;

      }
	}).move();
    }

    if(physics2D == nullptr && physics3D != nullptr){
    (*(input.input))["decreasePitch"].AddPressedCallback([&render, physics3D]{
		if(render.drawBoundingBox){

        (*physics3D).pitch -= 5;

      }
	}).move();
    }

    if(physics2D == nullptr && physics3D != nullptr){
    (*(input.input))["increaseRoll"].AddPressedCallback([&render, physics3D]{
		if(render.drawBoundingBox){
 		

        (*physics3D).roll += 5;

      }
	}).move();
    }

    if(physics2D == nullptr && physics3D != nullptr){
    (*(input.input))["decreaseRoll"].AddPressedCallback([&render, physics3D]{
		if(render.drawBoundingBox){
 		

        (*physics3D).roll -= 5;

      }
	}).move();
    }
}

void InitializeBuffer(Scene& scene){
    for(Entity e = 0; e < scene.entityMasks.size(); e++){
        if(!scene.HasComponent<Input>(e)) continue;
        if(!scene.HasComponent<Transforming>(e)) continue;
        if(!scene.HasComponent<Rendering>(e)) continue;
        if(!scene.HasComponent<Velocity>(e)) continue;
        
        
        auto& input = scene.GetComponent<Input>(e);
        auto& render = scene.GetComponent<Rendering>(e);
        auto& transforming = scene.GetComponent<Transforming>(e);
        auto& velocity = scene.GetComponent<Velocity>(e);

        Physics3D* null3D = nullptr;
        Physics2D* null2D = nullptr;
        
        if(scene.HasComponent<Physics2D>(e) && !scene.HasComponent<Physics3D>(e)){
            auto& physics2D = scene.GetComponent<Physics2D>(e);
            AddBuffer(e, input, velocity, render, &physics2D, null3D );

        }
        else if(scene.HasComponent<Physics3D>(e) && !scene.HasComponent<Physics2D>(e)){
            auto& physics3D = scene.GetComponent<Physics3D>(e);
            AddBuffer(e, input, velocity, render, null2D, &physics3D);

        }        
        
    }
}


void Draw(Scene& scene, float selected, raylib::Vector3& camPos){
    //Iterate through every entity in the scene.
    //entityMasks.size() will deliver use the size of the entities.
    for(Entity e = 0; e < scene.entityMasks.size(); e++){
        if(!scene.HasComponent<Rendering>(e)) continue;
        if(!scene.HasComponent<Transforming>(e)) continue;


        auto& transforming = scene.GetComponent<Transforming>(e);
        auto pos = transforming.position;
        raylib::Degree head;

        if(scene.HasComponent<Physics2D>(e)){
            head = scene.GetComponent<Physics2D>(e).heading;
        }
        else{
            head = raylib::Degree(0);
        }

        auto transformer2D = [pos, head](raylib::Transform transform) {
			return transform.Translate(pos).RotateY(raylib::Degree(head));
		};

        //auto [axis, angle] = {raylib::Vector3{0,0,0}, 0};

        if(scene.HasComponent<Physics3D>(e)){
          auto [axis, angle] = scene.GetComponent<Physics3D>(e).quat.ToAxisAngle();
        }
       

        //Inline if statement to receive values for quaternion. Will fill with 0 otherwise.
        auto [axis, angle] = scene.HasComponent<Physics3D>(e) ? scene.GetComponent<Physics3D>(e).quat.ToAxisAngle() : std::pair<raylib::Vector3, raylib::Radian>(raylib::Vector3{0,0,0}, 0);

        auto transformer3D = [pos, axis, angle](raylib::Transform transform){
            return transform.Translate(pos).Rotate(axis, angle);
        };

        //auto transformer3D = [pos, ]

        
        auto& rendering = scene.GetComponent<Rendering>(e);
        raylib::Transform backupTransform = rendering.model->transform;
        if(scene.HasComponent<Physics3D>(e)){
            rendering.model->transform = transformer3D(backupTransform);
        }
        else{
            rendering.model->transform = transformer2D(backupTransform);
        }
        rendering.model->Draw({});
        if(rendering.drawBoundingBox){
        rendering.model->GetTransformedBoundingBox().Draw();
        }
        rendering.model->transform = backupTransform;

        if(e == selected){
            //std::cout << selected << std::endl;
            rendering.drawBoundingBox = true;
            camPos = pos;
            
        }
        else{
            rendering.drawBoundingBox = false;
        }
        
    }
}



raylib::Vector3 UpdatePhysics2D(Velocity& velocity, Physics2D& physics, float dt){

    static constexpr auto AngleClamp = [](raylib::Degree angle) -> raylib::Degree {
		float decimal = float(angle) - int(angle);
		int whole = int(angle) % 360;
		whole += (whole < 0) * 360;
		return decimal + whole;
	};

    

    //std::cout << physics.targetHeading << std::endl;
    float target = AngleClamp(physics.targetHeading);
    //std::cout << transform.angularAcceleration << std::endl;
	float difference = abs(target - physics.heading);
	if(target > physics.heading) {
		if(difference < 180) physics.heading += velocity.angularAcceleration * dt;
		else if(difference > 180) physics.heading -= velocity.angularAcceleration * dt;
	} else if(target < physics.heading) {
		if(difference < 180) physics.heading -= velocity.angularAcceleration * dt;
		else if(difference > 180) physics.heading += velocity.angularAcceleration * dt;
	} 
	if(difference < .5) physics.heading = target; // If the heading is really close to correct 
	physics.heading = AngleClamp(physics.heading);
	raylib::Radian angle = raylib::Degree(physics.heading);
	

	return {cos(angle) * velocity.speed, 0, -sin(angle) * velocity.speed};

}

raylib::Vector3 UpdatePhysics3D(Velocity& velocity, Physics3D& physics, float dt){

    //std::cout << DegreeToRadian(raylib::Degree(90)) << std::endl;
    //CONVERTS IN ORDER OF Z --> Y --> X
    //Quaternion is stored backwards in terms of (K, J, I, Real)
    //raylib::Quaternion q = raylib::Quaternion::FromEuler(0.0f, 0.0f, 0.0f);
    //q.FromEuler();
    //std::cout << q.w << std::endl;
    //transform.speed = 1;
    //physics.quat = raylib::Quaternion::FromEuler(0.0f, 0.0f, DegreeToRadian(raylib::Degree(90)));
    //physics.quat = raylib::Quaternion::FromEuler(0.0f, 0.0f, 0.0f);
    //physics.quat = raylib::Quaternion::FromEuler(0.0f, 0.0f, 0.0f);
    //std::cout << (raylib::Vector3::Left().RotateByQuaternion(q) * transform.speed).x << std::endl;
    //std::cout << physics.roll << std::endl;
    //physics.quat = raylib::Quaternion::FromEuler(DegreeToRadian(physics.roll), 0.0f, 0.0f);
    raylib::Quaternion target = raylib::Quaternion::FromEuler(DegreeToRadian(physics.roll), DegreeToRadian(physics.yaw), DegreeToRadian(physics.pitch));
    //physics.quat = target;
    //physics.quat = raylib::Quaternion::Slerp(physics.quat, target, 0.2f);
    if(physics.quat != target){
    physics.quat = physics.quat.Slerp(target, dt);
    }

    return raylib::Vector3::Left().RotateByQuaternion(physics.quat)*velocity.speed; 
    
    //return raylib::Vector3{0, 0, 0};

}



void UpdatePhysics(Scene& scene, float dt){
    for(Entity e = 0; e < scene.entityMasks.size(); e++){
     if(!scene.HasComponent<Velocity>(e) || !scene.HasComponent<Transforming>(e) || ((scene.HasComponent<Physics2D>(e)) == (scene.HasComponent<Physics3D>(e)))) continue;
        auto& transforming = scene.GetComponent<Transforming>(e);
        auto& velocity = scene.GetComponent<Velocity>(e);
       // std::cout << int(e) << std::endl;
       
        
        float target = Clamp(velocity.targetSpeed, velocity.minSpeed, velocity.maxSpeed);
        if(e == 0){
            //std::cout << transforming.maxSpeed << std::endl;
        }

	    if(velocity.speed < target){ 
            //std::cout << "performing speedup" << std::endl;
            velocity.speed += velocity.acceleration * dt;
            //std::cout << transforming.acceleration << std::endl;
        }

	    else if(velocity.speed > target){ 
            
            velocity.speed -= velocity.acceleration * dt;
        }

	    velocity.speed = Clamp(velocity.speed, velocity.minSpeed, velocity.maxSpeed);


        if(scene.HasComponent<Physics2D>(e)){
            auto& physics = scene.GetComponent<Physics2D>(e);
            velocity.velocity = UpdatePhysics2D(velocity, physics, dt);
        }
        else if(scene.HasComponent<Physics3D>(e)){
            auto& physics = scene.GetComponent<Physics3D>(e);
            velocity.velocity = UpdatePhysics3D(velocity, physics, dt);
        }

        transforming.position = transforming.position + velocity.velocity*dt;


     //std::cout << CalculateEntityVelocity(transforming).x << '\n';
    }
}


void ProcessInput(Scene& scene){
    for(Entity e = 0; e < scene.entityMasks.size(); e++){
     if(!scene.HasComponent<Input>(e)) continue;
     auto& inputs = scene.GetComponent<Input>(e);
     //(*(inputs.input)).PollEvents();
    }
}

//A component in DOECS is just Data
//We need a system to interact with it.
//System batch works on a set of components.

int main() {

//raylib::Quaternion quat; 
//raylib::Quaternion::FromAxisAngle()
//Lerp: AKA linear interpolation.
//In mathematics, linear interpolation is a method of curve fitting using linear polynomials
//to construct new data points within the range of a discrete set of known data points
//Takes two things in a value. When the value is 0, you use all of the first thing, when the value is 
//1, we use all of the second thing. When the value is between, we mix them by about some percentage.

//Slerp: Spherical linear interpolation
//Euler axes: Vector 3 that represents a rotation around X, Y and Z.
//Yaw: Y axis
//Pitch: Z axis 
//Roll: X axis
//We need to blend quaternions.
//Do we need to multiply it by the inverse?

const int screenWidth = 1200;
const int screenHeight = 600;
raylib::Window window(screenWidth, screenHeight, "CS381 - Assignment 8");



	// Create camera
auto camera = raylib::Camera(
	raylib::Vector3(0, 100, -200), // Position
	raylib::Vector3(0, 0, 200), // Target
	raylib::Vector3::Up(), // Up direction2
	60.0f,
	CAMERA_PERSPECTIVE
);

cs381::SkyBox skybox("textures/skybox.png");

raylib::BufferedInput inputs;




auto mesh = raylib::Mesh::Plane(10000, 10000, 50, 50, 25);
	raylib::Model ground = ((raylib::Mesh*)&mesh)->LoadModelFrom();
	raylib::Texture grass("textures/water.jpg");
	grass.SetFilter(TEXTURE_FILTER_BILINEAR);
	grass.SetWrap(TEXTURE_WRAP_REPEAT);
	ground.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = grass;


raylib::Model planeModel("../meshes/PolyPlane.glb");

raylib::Model boat1Model("../meshes/ddg51.glb");
boat1Model.transform = raylib::Transform(boat1Model.transform).RotateX(raylib::Degree(90)).RotateY(raylib::Degree(90));


raylib::Model boat2Model("../meshes/OilTanker.glb");
boat2Model.transform = raylib::Transform(boat2Model.transform).Scale(0.007, 0.007, 0.007);
boat2Model.transform = raylib::Transform(boat2Model.transform).RotateX(raylib::Degree(90)).RotateY(raylib::Degree(270));


raylib::Model boat3Model("../meshes/OrientExplorer.glb");
boat3Model.transform = raylib::Transform(boat3Model.transform).Scale(0.013, 0.013, 0.013);
boat3Model.transform = raylib::Transform(boat3Model.transform).RotateX(raylib::Degree(90)).RotateY(raylib::Degree(270));



raylib::Model boat4Model("../meshes/SmitHouston_Tug.glb");
boat4Model.transform = raylib::Transform(boat4Model.transform).Scale(1, 1, 1);
boat4Model.transform = raylib::Transform(boat4Model.transform).RotateX(raylib::Degree(0)).RotateY(raylib::Degree(90));

raylib::Model boat5Model("../meshes/CargoG_HOSBrigadoon.glb");
boat5Model.transform = raylib::Transform(boat5Model.transform).Scale(0.013, 0.013, 0.013);
boat5Model.transform = raylib::Transform(boat5Model.transform).RotateX(raylib::Degree(90)).RotateY(raylib::Degree(270));


Scene mainScene;
auto plane1 = mainScene.CreateEntity();
mainScene.AddComponent<Transforming>(plane1);
mainScene.AddComponent<Input>(plane1);
mainScene.GetComponent<Input>(plane1).input = &inputs;
mainScene.AddComponent<Physics3D>(plane1);
mainScene.GetComponent<Physics3D>(plane1).quat = raylib::Quaternion::FromEuler(0.0f, 0.0f, 0.0f);
mainScene.AddComponent<Rendering>(plane1);
mainScene.GetComponent<Rendering>(plane1).model = &planeModel;
mainScene.GetComponent<Transforming>(plane1).position = raylib::Vector3{0, 50, 0};
mainScene.AddComponent<Velocity>(plane1);
mainScene.GetComponent<Velocity>(plane1).maxSpeed = 50.0f;
mainScene.GetComponent<Velocity>(plane1).acceleration = 5.0f;
mainScene.GetComponent<Velocity>(plane1).angularAcceleration = 10.0f;

auto plane2 = mainScene.CreateEntity();
mainScene.AddComponent<Transforming>(plane2);
mainScene.AddComponent<Input>(plane2);
mainScene.GetComponent<Input>(plane2).input = &inputs;
mainScene.AddComponent<Physics3D>(plane2);
mainScene.GetComponent<Physics3D>(plane2).quat = raylib::Quaternion::FromEuler(0.0f, 0.0f, 0.0f);
mainScene.AddComponent<Rendering>(plane2);
mainScene.GetComponent<Rendering>(plane2).model = &planeModel;
mainScene.GetComponent<Transforming>(plane2).position = raylib::Vector3{-100, 50, 0};
mainScene.AddComponent<Velocity>(plane2);
mainScene.GetComponent<Velocity>(plane2).maxSpeed = 50.0f;
mainScene.GetComponent<Velocity>(plane2).acceleration = 5.0f;
mainScene.GetComponent<Velocity>(plane2).angularAcceleration = 10.0f;

auto plane3 = mainScene.CreateEntity();
mainScene.AddComponent<Transforming>(plane3);
mainScene.AddComponent<Input>(plane3);
mainScene.GetComponent<Input>(plane3).input = &inputs;
mainScene.AddComponent<Physics3D>(plane3);
mainScene.GetComponent<Physics3D>(plane3).quat = raylib::Quaternion::FromEuler(0.0f, 0.0f, 0.0f);
mainScene.AddComponent<Rendering>(plane3);
mainScene.GetComponent<Rendering>(plane3).model = &planeModel;
mainScene.GetComponent<Transforming>(plane3).position = raylib::Vector3{100, 50, 0};
mainScene.AddComponent<Velocity>(plane3);
mainScene.GetComponent<Velocity>(plane3).maxSpeed = 50.0f;
mainScene.GetComponent<Velocity>(plane3).acceleration = 5.0f;
mainScene.GetComponent<Velocity>(plane3).angularAcceleration = 10.0f;

auto boat1 = mainScene.CreateEntity();
mainScene.AddComponent<Transforming>(boat1);
mainScene.AddComponent<Input>(boat1);
mainScene.GetComponent<Input>(boat1).input = &inputs;
mainScene.AddComponent<Physics2D>(boat1);
mainScene.AddComponent<Rendering>(boat1);
mainScene.GetComponent<Rendering>(boat1).model = &boat1Model;
mainScene.AddComponent<Velocity>(boat1);
mainScene.GetComponent<Velocity>(boat1).maxSpeed = 20.0f;
mainScene.GetComponent<Velocity>(boat1).acceleration = 3.5f;
mainScene.GetComponent<Velocity>(boat1).angularAcceleration = 2.0f;

auto boat2 = mainScene.CreateEntity();
mainScene.AddComponent<Transforming>(boat2);
mainScene.AddComponent<Input>(boat2);
mainScene.GetComponent<Input>(boat2).input = &inputs;
mainScene.AddComponent<Physics2D>(boat2);
mainScene.AddComponent<Rendering>(boat2);
mainScene.GetComponent<Rendering>(boat2).model = &boat2Model;
mainScene.GetComponent<Transforming>(boat2).position = raylib::Vector3{-75, 0, 90};
mainScene.AddComponent<Velocity>(boat2);
mainScene.GetComponent<Velocity>(boat2).maxSpeed = 30.0f;
mainScene.GetComponent<Velocity>(boat2).acceleration = 3.0f;
mainScene.GetComponent<Velocity>(boat2).angularAcceleration = 1.5f;

auto boat3 = mainScene.CreateEntity();
mainScene.AddComponent<Transforming>(boat3);
mainScene.AddComponent<Input>(boat3);
mainScene.GetComponent<Input>(boat3).input = &inputs;
mainScene.AddComponent<Physics2D>(boat3);
mainScene.AddComponent<Rendering>(boat3);
mainScene.GetComponent<Rendering>(boat3).model = &boat3Model;
mainScene.GetComponent<Transforming>(boat3).position = raylib::Vector3{85, 0, 90};
mainScene.AddComponent<Velocity>(boat3);
mainScene.GetComponent<Velocity>(boat3).angularAcceleration = 4.0f;
mainScene.GetComponent<Velocity>(boat3).acceleration = 6.0f;
mainScene.GetComponent<Velocity>(boat3).maxSpeed = 60.0f;

auto boat4 = mainScene.CreateEntity();
mainScene.AddComponent<Transforming>(boat4);
mainScene.AddComponent<Input>(boat4);
mainScene.GetComponent<Input>(boat4).input = &inputs;
mainScene.AddComponent<Physics2D>(boat4);
mainScene.AddComponent<Rendering>(boat4);
mainScene.GetComponent<Rendering>(boat4).model = &boat4Model;
mainScene.GetComponent<Transforming>(boat4).position = raylib::Vector3{85, 0, -45};
mainScene.AddComponent<Velocity>(boat4);
mainScene.GetComponent<Velocity>(boat4).angularAcceleration = 5.0f;
mainScene.GetComponent<Velocity>(boat4).acceleration = 6.3f;
mainScene.GetComponent<Velocity>(boat4).maxSpeed = 70.0f;

auto boat5 = mainScene.CreateEntity();
mainScene.AddComponent<Transforming>(boat5);
mainScene.AddComponent<Input>(boat5);
mainScene.GetComponent<Input>(boat5).input = &inputs;
mainScene.AddComponent<Physics2D>(boat5);
mainScene.AddComponent<Rendering>(boat5);
mainScene.GetComponent<Rendering>(boat5).model = &boat5Model;
mainScene.GetComponent<Transforming>(boat5).position = raylib::Vector3{-85, 0, -45};
mainScene.AddComponent<Velocity>(boat5);
mainScene.GetComponent<Velocity>(boat5).angularAcceleration = 4.7f;
mainScene.GetComponent<Velocity>(boat5).acceleration = 6.3f;
mainScene.GetComponent<Velocity>(boat5).maxSpeed = 65.0f;

//Entity selection tool.
float selectedEntity = 0;

inputs["increaseVelocity"] = raylib::Action::key(KEY_W);
inputs["decreaseVelocity"] = raylib::Action::key(KEY_S);
inputs["increaseHeading"] = raylib::Action::key(KEY_A);
inputs["decreaseHeading"] = raylib::Action::key(KEY_D);
inputs["increasePitch"] = raylib::Action::key(KEY_Q);
inputs["decreasePitch"] = raylib::Action::key(KEY_E);
inputs["increaseRoll"] = raylib::Action::key(KEY_R);
inputs["decreaseRoll"] = raylib::Action::key(KEY_F);
inputs["stop"] = raylib::Action::key(KEY_SPACE);


inputs["change"] = raylib::Action::key(KEY_TAB).AddPressedCallback([&selectedEntity, &mainScene]{
    //std::cout << "Tab pressed" << std::endl;
    selectedEntity++; 
    //std::cout << mainScene.entityMasks.size() - 1 << std::endl;
    if(selectedEntity > mainScene.entityMasks.size() - 1){
    selectedEntity = 0;
    }
    std::cout << selectedEntity << std::endl;

}).move();



InitializeBuffer(mainScene);

bool keepRunning = true;
raylib::Vector3 entityPos = raylib::Vector3{0,0,0};
while(!window.ShouldClose() && keepRunning) {

    window.BeginDrawing();
		{
			// Clear screen
			window.ClearBackground(BLACK);


            camera.position = raylib::Vector3(entityPos.x - 100.0f, entityPos.y + 50.0f, entityPos.z);
            camera.target = entityPos;
			camera.BeginMode();
			{
				skybox.Draw();
				ground.Draw({});
                UpdatePhysics(mainScene, window.GetFrameTime());
                Draw(mainScene, selectedEntity, entityPos);
            }
			camera.EndMode();
            inputs.PollEvents();

			// Measure our FPS
			DrawFPS(10, 10);
		}
		window.EndDrawing();
	}


return 0;
}


//Godot Notes:
//Scene tree:
//A tree of scenes.
//Every entity becomes an object in a scene.
//Instead of having them in a flat list, we have them in a tree structure.
//What if we get rid of the idea that entities have components. 
//Instead we have a node which acts like an entity, and we build a tree of these.
//So a collision component would affect its parent node.

//Godot node tree architecture is similar.
//Their implementation code ends up being very very similar, but the way we interact with them ends up being 
//very very different.

//Like a prefab.