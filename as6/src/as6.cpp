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

// #include "inputs.hpp"
#include "skybox.hpp"

#define ENTITYSIZE 8

template<typename T>
concept Transformer = requires(T t, raylib::Transform m) {
	{ t.operator()(m) } -> std::convertible_to<raylib::Transform>;
};

struct CalculateVelocityParams {
	static constexpr float acceleration = 5;
	static constexpr float angularAcceleration = 15;

	float targetSpeed;
	raylib::Degree targetHeading;
	float& speed;
	raylib::Degree& heading;
	float dt;

	float maxSpeed = 50;
	float minSpeed = 0;
};

bool ProcessInput(raylib::Degree& planeTargetHeading, float& planeTargetSpeed, size_t& selectedPlane);


raylib::Vector3 CaclulateVelocity(const CalculateVelocityParams& data);

void DrawBoundedModel(raylib::Model& model, Transformer auto transformer);
void DrawModel(raylib::Model& model, Transformer auto transformer);


struct Component { 
    struct Entity* object; 

    Component(struct Entity& e) : object(&e) {}

    virtual void setup(){ };
    virtual void cleanup(){ };
    virtual void tick(float dt){ };


};

//Special component that inherits component. 
struct TransformComponent : public Component { 
    //Using components constructor instead of a new one. 
    using Component::Component; 
    raylib::Vector3 position = {0, 0, 0};
	raylib::Vector3 velocity = raylib::Vector3::Zero();
	raylib::Degree heading = 0, targetHeading = 0;
	float speed = 0, targetSpeed = 0, dt_ = 0;

	float maxSpeed = 50;
	float minSpeed = 0;


	//static memory allocation is the allocation of memory at compile time, before the associated program is executed, 
	//unlike dynamic memory allocation or automatic memory allocation where memory is allocated as required at run time.

	float acceleration = 5; 
	float angularAcceleration = 15;

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

raylib::Vector3 CaclulateEntityVelocity(Entity& entity);
bool ProcessEntityInput(Entity& entity, size_t& selectedPlane, raylib::BufferedInput& inputs);


struct RenderingComponent : public Component {

	using Component::Component; 

	RenderingComponent(Entity& e, raylib::Model&& model) : Component(e), model(std::move(model)) { };

	raylib::Model model; //Assuming set on construction
	bool drawBox = false;

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

		//std::cout << pos.x << std::endl;

		auto transformer = [pos, head](raylib::Transform transform) {
			return transform.Translate(pos).RotateY(raylib::Degree(head));
		};

		//To axis angle returns a std::pair of a <vector3, and a radian>.
		//Axis to rotate around, and how far to rotate around that axis.

		auto [axis, angle] = transform.rotation.ToAxisAngle();

		//model.Draw(transform.position, axis, angle);
		if(drawBox){
			DrawBoundedModel(model, transformer);
		}
		else{
			DrawModel(model, transformer);
		}
	}

	
};

struct PhysicsComponent : public Component { 

	using Component::Component; 



	void tick(float dt){
	auto ref = object->GetComponent<TransformComponent>();
		

		
		//Because its optional reference, may or may not exist. 
		//If it doesn't exist, we return.
		 if(!ref){
			std::cout << "No reference found" << std::endl;
			return;
		 } 


	auto& transform = ref->get();



	transform.velocity = CaclulateEntityVelocity(*object);
	transform.position = transform.position + transform.velocity*dt;


	}
};

int main() {
	// Create window
	const int screenWidth = 1200;
	const int screenHeight = 600;
	raylib::Window window(screenWidth, screenHeight, "CS381 - Assignment 6");
	// cs381::Inputs inputs(window);

	// Create camera
	auto camera = raylib::Camera(
		raylib::Vector3(0, 100, -200), // Position
		raylib::Vector3(0, 0, 200), // Target
		raylib::Vector3::Up(), // Up direction2
		45.0f,
		CAMERA_PERSPECTIVE
	);

	// Create skybox
	cs381::SkyBox skybox("textures/skybox.png");

	// Create ground
	auto mesh = raylib::Mesh::Plane(10000, 10000, 50, 50, 25);
	raylib::Model ground = ((raylib::Mesh*)&mesh)->LoadModelFrom();
	raylib::Texture grass("textures/water.jpg");
	grass.SetFilter(TEXTURE_FILTER_BILINEAR);
	grass.SetWrap(TEXTURE_WRAP_REPEAT);
	ground.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = grass;

	// Create airplane
	raylib::Model plane("../meshes/PolyPlane.glb");
    plane.transform = raylib::Transform(plane.transform).Scale(1.5f, 1.5f, 1.5f);

	/*Entity airplane1;
	Entity airplane2;
	Entity airplane3; 

	Entity boat1; 
	Entity boat2; 

	Entity planeArr[3] = {Entity(std::move(airplane1)), Entity(std::move(airplane2)), Entity(std::move(airplane3))};*/

	std::vector<Entity> entities;

	//Difference between pushback and emplace back: 
	//Will construct the object for you. Push back will copy in an object that already exists. 
	//When we emplace back, we theoretically don't have an object already.
	//Push_back will throw an error because we deleted our copy constructor for optimization purposes.
	Entity& plane1 = entities.emplace_back();
	Entity& plane2 = entities.emplace_back();
	Entity& plane3 = entities.emplace_back();
	Entity& boat1 =  entities.emplace_back();
	Entity& boat2 =  entities.emplace_back();
	Entity& boat3 =  entities.emplace_back();
	Entity& boat4 =  entities.emplace_back();
	Entity& boat5 =  entities.emplace_back();


	//entities[0].AddComponent<RenderingComponent>();
	//plane1.AddComponent<RenderingComponent>();



	//Set the model and attributes of the entities
	for(int i = 0; i < entities.size(); i++){
		entities[i].AddComponent<PhysicsComponent>();
		if(i < 3){
			entities[i].AddComponent<RenderingComponent>(raylib::Model("../meshes/PolyPlane.glb"));
			entities[i].GetComponent<TransformComponent>()->get().acceleration = 50;
			entities[i].GetComponent<TransformComponent>()->get().maxSpeed = 150;

		}
	}

	entities[0].GetComponent<TransformComponent>()->get().position = raylib::Vector3{0, 75, 0};

	entities[1].GetComponent<TransformComponent>()->get().position = raylib::Vector3{75, 75, 0};

	entities[2].GetComponent<TransformComponent>()->get().position = raylib::Vector3{-75, 75, 0};

	//Set model and attributes of the ships.
	entities[3].AddComponent<RenderingComponent>(raylib::Model("../meshes/ddg51.glb"));
	entities[3].GetComponent<RenderingComponent>()->get().model.transform = raylib::Transform(entities[3].GetComponent<RenderingComponent>()->get().model.transform).RotateX(raylib::Degree(90)).RotateY(raylib::Degree(90));
	entities[3].GetComponent<TransformComponent>()->get().angularAcceleration = 1;
	entities[3].GetComponent<TransformComponent>()->get().acceleration = 1;
	entities[3].GetComponent<TransformComponent>()->get().maxSpeed = 30;


	entities[4].AddComponent<RenderingComponent>(raylib::Model("../meshes/OilTanker.glb"));
	entities[4].GetComponent<RenderingComponent>()->get().model.transform = raylib::Transform(entities[4].GetComponent<RenderingComponent>()->get().model.transform).Scale(0.007, 0.007, 0.007);
	entities[4].GetComponent<RenderingComponent>()->get().model.transform = raylib::Transform(entities[4].GetComponent<RenderingComponent>()->get().model.transform).RotateX(raylib::Degree(90)).RotateY(raylib::Degree(270));
	entities[4].GetComponent<TransformComponent>()->get().position = raylib::Vector3{-75, 0, 90};
	entities[4].GetComponent<TransformComponent>()->get().angularAcceleration = 0.5;
	entities[4].GetComponent<TransformComponent>()->get().acceleration = 0.5;
	entities[4].GetComponent<TransformComponent>()->get().maxSpeed = 30;

	entities[5].AddComponent<RenderingComponent>(raylib::Model("../meshes/OrientExplorer.glb"));
	entities[5].GetComponent<RenderingComponent>()->get().model.transform = raylib::Transform(entities[5].GetComponent<RenderingComponent>()->get().model.transform).Scale(0.013, 0.013, 0.013);
	entities[5].GetComponent<RenderingComponent>()->get().model.transform = raylib::Transform(entities[5].GetComponent<RenderingComponent>()->get().model.transform).RotateX(raylib::Degree(90)).RotateY(raylib::Degree(270));
	entities[5].GetComponent<TransformComponent>()->get().position = raylib::Vector3{85, 0, 90};
	entities[5].GetComponent<TransformComponent>()->get().angularAcceleration = 1.5;
	entities[5].GetComponent<TransformComponent>()->get().acceleration = 1.5;
	entities[5].GetComponent<TransformComponent>()->get().maxSpeed = 50;

	entities[6].AddComponent<RenderingComponent>(raylib::Model("../meshes/SmitHouston_Tug.glb"));
	entities[6].GetComponent<RenderingComponent>()->get().model.transform = raylib::Transform(entities[6].GetComponent<RenderingComponent>()->get().model.transform).Scale(1, 1, 1);
	entities[6].GetComponent<RenderingComponent>()->get().model.transform = raylib::Transform(entities[6].GetComponent<RenderingComponent>()->get().model.transform).RotateX(raylib::Degree(0)).RotateY(raylib::Degree(90));
	entities[6].GetComponent<TransformComponent>()->get().position = raylib::Vector3{85, 0, -45};
	entities[6].GetComponent<TransformComponent>()->get().angularAcceleration = 3;
	entities[6].GetComponent<TransformComponent>()->get().acceleration = 4;
	entities[6].GetComponent<TransformComponent>()->get().maxSpeed = 75;

	entities[7].AddComponent<RenderingComponent>(raylib::Model("../meshes/CargoG_HOSBrigadoon.glb"));
	entities[7].GetComponent<RenderingComponent>()->get().model.transform = raylib::Transform(entities[7].GetComponent<RenderingComponent>()->get().model.transform).Scale(0.013, 0.013, 0.013);
	entities[7].GetComponent<RenderingComponent>()->get().model.transform = raylib::Transform(entities[7].GetComponent<RenderingComponent>()->get().model.transform).RotateX(raylib::Degree(90)).RotateY(raylib::Degree(270));
	entities[7].GetComponent<TransformComponent>()->get().position = raylib::Vector3{-85, 0, -45};
	entities[7].GetComponent<TransformComponent>()->get().angularAcceleration = 2;
	entities[7].GetComponent<TransformComponent>()->get().acceleration = 2.3;
	entities[7].GetComponent<TransformComponent>()->get().maxSpeed = 70;
	

	

	size_t selectedPlane = 0;
	raylib::BufferedInput inputs;


	// Main loop
	bool keepRunning = true;
	while(!window.ShouldClose() && keepRunning) {
		// Updates
		// Process input for the selected plane
		/*switch(selectedPlane) {
			break; case 0: keepRunning = ProcessInput(plane0TargetHeading, plane0TargetSpeed, selectedPlane); 
						   //keepRunning = ProcessEntityInput(entities[0], selectedPlane);
			break; case 1: keepRunning = ProcessInput(plane1TargetHeading, plane1TargetSpeed, selectedPlane); 
						   //keepRunning = ProcessEntityInput(entities[1], selectedPlane);
			break; case 2: keepRunning = ProcessInput(plane2TargetHeading, plane2TargetSpeed, selectedPlane); 
						   //keepRunning = ProcessEntityInput(entities[2], selectedPlane);
		}*/

		for(int i = 0; i < entities.size(); i++){
			entities[i].GetComponent<TransformComponent>()->get().dt_ = window.GetFrameTime();
			if(i == selectedPlane){
				//std::cout << "entity " << i << " selected" << std::endl;
				//std::cout << "x pos: " << entities[i].GetComponent<TransformComponent>()->get().position.x << std::endl;
				keepRunning = ProcessEntityInput(entities[i], selectedPlane, inputs);
				entities[i].GetComponent<RenderingComponent>()->get().drawBox = true;
				
			}

			else{
				entities[i].GetComponent<RenderingComponent>()->get().drawBox = false;
			}
			
			//entities[i].GetComponent<TransformComponent>()->get().velocity = CaclulateEntityVelocity(entities[i]);
			//entities[i].GetComponent<TransformComponent>()->get().tick(window.GetFrameTime());

		}

		//Event based queue that processes the events based on when they happen.
		//All of these actions, whatever we subscribe to them with will happen.
		//
		inputs.PollEvents();
		
		// Apply simple physics to plane0
		/*plane0Velocity = CaclulateVelocity({
			.targetSpeed = plane0TargetSpeed,
			.targetHeading = plane0TargetHeading,
			.speed = plane0Speed,
			.heading = plane0Heading,
			.dt = window.GetFrameTime()
		});
		plane0Position = plane0Position + plane0Velocity * window.GetFrameTime();
		auto plane0Transformer = [plane0Position, plane0Heading](raylib::Transform transform) {
			return transform.Translate(plane0Position).RotateY(raylib::Degree(plane0Heading));
		};

		// Apply simple physics to plane1
		plane1Velocity = CaclulateVelocity(CalculateVelocityParams{
			.targetSpeed = plane1TargetSpeed,
			.targetHeading = plane1TargetHeading,
			.speed = plane1Speed,
			.heading = plane1Heading,
			.dt = window.GetFrameTime()
		});
		plane1Position = plane1Position + plane1Velocity * window.GetFrameTime();
		auto plane1Transformer = [plane1Position, plane1Heading](raylib::Transform transform) {
			return transform.Translate(plane1Position).RotateY(raylib::Degree(plane1Heading));
		};

		// Apply simple physics to plane2
		plane2Velocity = CaclulateVelocity(CalculateVelocityParams{
			.targetSpeed = plane2TargetSpeed,
			.targetHeading = plane2TargetHeading,
			.speed = plane2Speed,
			.heading = plane2Heading,
			.dt = window.GetFrameTime()
		});
		plane2Position = plane2Position + plane2Velocity * window.GetFrameTime();
		auto plane2Transformer = [plane2Position, plane2Heading](raylib::Transform transform) {
			return transform.Translate(plane2Position).RotateY(raylib::Degree(plane2Heading));
		};*/


		// Rendering
		window.BeginDrawing();
		{
			// Clear screen
			window.ClearBackground(BLACK);

			camera.BeginMode();
			{
				// Render skybox and ground
				skybox.Draw();
				ground.Draw({});

				// Draw the planes with a bounding box around the selected plane
				/*switch(selectedPlane) {
					break; case 0: {
						DrawBoundedModel(plane, plane0Transformer);
						DrawModel(plane, plane1Transformer);
						DrawModel(plane, plane2Transformer);
					} break; case 1: {
						DrawModel(plane, plane0Transformer);
						DrawBoundedModel(plane, plane1Transformer);
						DrawModel(plane, plane2Transformer);
					} break; case 2: {
						DrawModel(plane, plane0Transformer);
						DrawModel(plane, plane1Transformer);
						DrawBoundedModel(plane, plane2Transformer);
					}
				}*/
				
				for(int i = 0; i < entities.size(); i++){
					entities[i].tick(window.GetFrameTime());
				}
				 
				//(entities[3].GetComponent<RenderingComponent>()->get()).RenderingComponent::tick(window.GetFrameTime()); //.RenderingComponent::tick(window.GetFrameTime());
			}
			camera.EndMode();

			// Measure our FPS
			DrawFPS(10, 10);
		}
		window.EndDrawing();
	}

	return 0;
}

// Input handling
/*bool ProcessInput(raylib::Degree& planeTargetHeading, float& planeTargetSpeed, size_t& selectedPlane) {
	static bool wPressedLastFrame = false, sPressedLastFrame = false;
	static bool aPressedLastFrame = false, dPressedLastFrame = false;
	static bool tabPressedLastFrame = false;

	// If we hit escape... shutdown
	if(IsKeyDown(KEY_ESCAPE))
		return false;

	// WASD updates plane velocity
	if(IsKeyDown(KEY_W) && !wPressedLastFrame)
		planeTargetSpeed += 1;
	if(IsKeyDown(KEY_S) && !sPressedLastFrame)
		planeTargetSpeed -= 1;
	if(IsKeyDown(KEY_A) && !aPressedLastFrame)
		planeTargetHeading += 5;
	if(IsKeyDown(KEY_D) && !dPressedLastFrame)
		planeTargetHeading -= 5;  

	// Space sets velocity to 0!
	if(IsKeyDown(KEY_SPACE))
		planeTargetSpeed = 0;

	// Tab selects the next plane
	if(IsKeyDown(KEY_TAB) && !tabPressedLastFrame)
		//selectedPlane = (selectedPlane + 1) % 3;
        //std::cout << selectedPlane << std::endl;

	// Save the state of the key for next frame
	wPressedLastFrame = IsKeyDown(KEY_W);
	sPressedLastFrame = IsKeyDown(KEY_S);
	aPressedLastFrame = IsKeyDown(KEY_A);
	dPressedLastFrame = IsKeyDown(KEY_D);

	tabPressedLastFrame = IsKeyDown(KEY_TAB);
	return true;
}*/

bool ProcessEntityInput(Entity& entity, size_t& selectedPlane, raylib::BufferedInput& inputs){

	//static raylib::BufferedInput inputs;
	bool quitFlag = false;
	float speed = 0;

	auto ref = entity.GetComponent<TransformComponent>();

		//Because its optional reference, may or may not exist. 
		//If it doesn't exist, we return.
		 if(!ref){
			std::cout << "No reference found" << std::endl;
			return false;
		 } 
	auto& transform = ref->get();


	static bool wPressedLastFrame = false, sPressedLastFrame = false;
	static bool aPressedLastFrame = false, dPressedLastFrame = false;
	static bool tabPressedLastFrame = false;

	// If we hit escape... shutdown
	//if(IsKeyDown(KEY_ESCAPE))
	//	return false;

	//What is an action?
	//
	//Whenever the buttion is pressed, we get a callback.
	//We add some actions into the inputs library, and then, 
	//every frame we call inputs.PollEvents();


	/*inputs["quit"] = raylib::Action::key(KEY_SPACE).SetPressedCallback([&quitFlag]{
		quitFlag = true;
		std::cout << "Quitting" << std::endl;
	}).move();
	if(quitFlag) return false;*/

	
	if(!wPressedLastFrame){
	inputs["increaseVelocity"] = raylib::Action::key(KEY_W).SetPressedCallback([&transform]{
		
		transform.targetSpeed += 1;
	}).move();
	}

	if(!sPressedLastFrame){
	inputs["decreaseVelocity"] = raylib::Action::key(KEY_S).SetPressedCallback([&transform]{
		
		transform.targetSpeed -= 1;
	}).move();
	}

	if(!aPressedLastFrame){
	inputs["increaseHeading"] = raylib::Action::key(KEY_A).SetPressedCallback([&transform]{
		
		transform.targetHeading += 5;
	}).move();
	}

	if(!dPressedLastFrame){
	inputs["decreaseHeading"] = raylib::Action::key(KEY_D).SetPressedCallback([&transform]{
		
		transform.targetHeading -= 5;
	}).move();
	}

	inputs["stop"] = raylib::Action::key(KEY_SPACE).SetPressedCallback([&transform]{
		transform.targetSpeed = 0;
	}).move();

	if(!tabPressedLastFrame){
	inputs["change"] = raylib::Action::key(KEY_TAB).SetPressedCallback([&selectedPlane]{
		
		selectedPlane = (selectedPlane + 1) % ENTITYSIZE;
	}).move();
	}


	// Tab selects the next plane

	// Save the state of the key for next frame
	wPressedLastFrame = IsKeyDown(KEY_W);
	sPressedLastFrame = IsKeyDown(KEY_S);
	aPressedLastFrame = IsKeyDown(KEY_A);
	dPressedLastFrame = IsKeyDown(KEY_D);

	tabPressedLastFrame = IsKeyDown(KEY_TAB);

	return true;

}

raylib::Vector3 CaclulateVelocity(const CalculateVelocityParams& data) {
	static constexpr auto AngleClamp = [](raylib::Degree angle) -> raylib::Degree {
		float decimal = float(angle) - int(angle);
		int whole = int(angle) % 360;
		whole += (whole < 0) * 360;
		return decimal + whole;
	};

	float target = Clamp(data.targetSpeed, data.minSpeed, data.maxSpeed);
	if(data.speed < target) data.speed += data.acceleration * data.dt;
	else if(data.speed > target) data.speed -= data.acceleration * data.dt;
	data.speed = Clamp(data.speed, data.minSpeed, data.maxSpeed);

	target = AngleClamp(data.targetHeading);
	float difference = abs(target - data.heading);
	if(target > data.heading) {
		if(difference < 180) data.heading += data.angularAcceleration * data.dt;
		else if(difference > 180) data.heading -= data.angularAcceleration * data.dt;
	} else if(target < data.heading) {
		if(difference < 180) data.heading -= data.angularAcceleration * data.dt;
		else if(difference > 180) data.heading += data.angularAcceleration * data.dt;
	} 
	if(difference < .5) data.heading = target; // If the heading is really close to correct 
	data.heading = AngleClamp(data.heading);
	raylib::Radian angle = raylib::Degree(data.heading);

	
	return {cos(angle) * data.speed, 0, -sin(angle) * data.speed};
}

raylib::Vector3 CaclulateEntityVelocity(Entity& entity){

	auto ref = entity.GetComponent<TransformComponent>();

		//Because its optional reference, may or may not exist. 
		//If it doesn't exist, we return.
		 if(!ref){
			std::cout << "No reference found" << std::endl;
			return raylib::Vector3::Zero();
		 } 

	auto& transform = ref->get();

	static constexpr auto AngleClamp = [](raylib::Degree angle) -> raylib::Degree {
		//Splits the heading into two values. Both whole and decimal.
		float decimal = float(angle) - int(angle);
		int whole = int(angle) % 360;
		whole += (whole < 0) * 360;
		return decimal + whole;
	};

	float target = Clamp(transform.targetSpeed, transform.minSpeed, transform.maxSpeed);
	if(transform.speed < target) transform.speed += transform.acceleration * transform.dt_;
	else if(transform.speed > target) transform.speed -= transform.acceleration * transform.dt_;
	transform.speed = Clamp(transform.speed, transform.minSpeed, transform.maxSpeed);

	target = AngleClamp(transform.targetHeading);
	float difference = abs(target - transform.heading);
	if(target > transform.heading) {
		if(difference < 180) transform.heading += transform.angularAcceleration * transform.dt_;
		else if(difference > 180) transform.heading -= transform.angularAcceleration * transform.dt_;
	} else if(target < transform.heading) {
		if(difference < 180) transform.heading -= transform.angularAcceleration * transform.dt_;
		else if(difference > 180) transform.heading += transform.angularAcceleration * transform.dt_;
	} 
	if(difference < .5) transform.heading = target; // If the heading is really close to correct 
	transform.heading = AngleClamp(transform.heading);
	raylib::Radian angle = raylib::Degree(transform.heading);

	return {cos(angle) * transform.speed, 0, -sin(angle) * transform.speed};

	return raylib::Vector3{0, 0, 0};

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