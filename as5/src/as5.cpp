#include <iostream>
#include "raylib-cpp.hpp"
#include "rlgl.h"
#include <vector>
#include <memory>
#include <optional>
#include <concepts>
#include "delegate.hpp"
#include <BufferedInput.hpp>
#include <ranges>
#define GUI_VOLUMECONTROL_IMPLEMENTATION

//#include "raygui.cpp"




cs381::Delegate<void()> PingButton;
cs381::Delegate<void()> AdjustVolume;
cs381::Delegate<void()> ChangeSlider;





struct Component { 
    struct Entity* object; 

    Component(struct Entity& e) : object(&e) {}

    virtual void setup() = 0;
    virtual void cleanup() = 0;
    virtual void tick(float dt) = 0;


};

//Special component that inherits component. 
struct TransformComponent : public Component { 
    //Using components constructor instead of a new one. 
    using Component::Component; 
    raylib::Vector3 position = {0, 0, 0};
    //Quaternion?????
    raylib::Quaternion rotation = raylib::Quaternion::Identity();

    void setup() {

    }

    void cleanup() {

    }

    void tick(float dt){

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

        //Compile time if statement 
        if constexpr(std::is_same_v<T, TransformComponent>){
            //Optimization
            //We KNOW the first component will be the transform, thats why we use 0 here.
            T* cast = dynamic_cast<T>(components[0].get());

            //If not nullptr, return value stored in cast.
            if(cast) return *cast;
        }

        /*for(auto& c: components){

            //c.get() returns a pointer to the managed object.
            //dynamic cast : If the pointer can be validly converted into a T , then do it.
            //If not , give us a nullptr.
            T* cast = dynamic_cast<T>(c.get());

            //If not nullptr, return value stored in cast.
            if(cast) return *cast;
        }*/

        //null pointer for optionals.
        return std::nullopt;
    }

    void tick(float dt){
        for(auto& c: components)
            c->tick(dt);
    }


};



#include "VolumeControl.h"

int main(){

PingButton += []() {
    std::cout << "Ping!" << std::endl;
};


raylib::Window window(300, 350, "CS381 - Assignment 5");
raylib::Text text;
int windHeight = 0, windWidth = 0;


raylib::BufferedInput inputs;

inputs["ping"] = raylib::Action::key(KEY_SPACE).SetPressedCallback([]{
    PingButton();
}).move();

inputs["change"] = raylib::Action::key(KEY_TAB).SetPressedCallback([]{
    ChangeSlider();
}).move();

inputs["paste"] = raylib::Action::button_set({raylib::Button::key(KEY_LEFT_CONTROL), raylib::Button::key(KEY_V)}, true);

raylib::AudioDevice audio;
int currentSlider = 0;

raylib::Sound ping("../sounds/rupee.mp3");

PingButton += [&ping](){
    ping.Play();
};

ChangeSlider += [&currentSlider]()-> void{
    currentSlider += 1;
    if(currentSlider > 2){
        currentSlider = 0;
    }
};

ChangeSlider += [&currentSlider]() -> void{
    std::cout << "Changing slider to " << currentSlider << std::endl;
};

raylib::Music music("../sounds/cynthia.mp3");
raylib::Music crowd("../sounds/markiplier.mp3");

auto guiState = InitGuiVolumeControl();


//PingButton();

while(!window.ShouldClose()) { 
 //Instead of checking every frame, we look to see what inputs need to be processed since the last frame. Inputs are 
 //put into the queue we created.
 inputs.PollEvents();

 window.BeginDrawing();
 {

    ClearBackground(raylib::Color::Black());
    GuiVolumeControl(&guiState);

    //ping.Play();
    music.Play();
    music.Update();

    crowd.Play();
    crowd.Update();

    ping.SetVolume(guiState.SFXSliderValue / (float)100.0f);
    music.SetVolume(guiState.MusicSliderValue / (float)100.0f);
    crowd.SetVolume(guiState.DialogueSliderValue / (float)100.0f);

    if(IsKeyDown(KEY_RIGHT)){
        switch(currentSlider){
            case 0:  
            guiState.SFXSliderValue += 0.2;
            break;
            case 1:
            guiState.MusicSliderValue += 0.2;
            break;
            case 2: 
            guiState.DialogueSliderValue += 0.2;
            break;

        }
        
    }

    if(IsKeyDown(KEY_LEFT)){
        switch(currentSlider){
            case 0:  
            guiState.SFXSliderValue -= 0.2;
            break;
            case 1:
            guiState.MusicSliderValue -= 0.2;
            break;
            case 2: 
            guiState.DialogueSliderValue -= 0.2;
            break;

        }
    }

 }
 window.EndDrawing();
}


return 0;

}

#define GUI_VOLUMECONTROL_IMPLEMENTATION
