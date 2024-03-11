#include <functional> 
#include <type_traits>
#include <vector>

namespace cs381 { 

    template<typename>
    struct Delegate {};

    template<typename Return, typename... Arguments>
    struct Delegate<Return(Arguments...)>{
        //Variable that holds a function pointer
        //Angle brackets so templated
        std::vector<std::function<Return(Arguments...)>> functions;

        template<typename Class>
        using MethodType = Return(Class::*)(Arguments...);


        void operator()(Arguments... args){
            //Calls the function for every function pointer in the vector.
            for(auto& f: functions)
                f(args...);
        }


        template<typename Class>
        void connect(Class& cls, MethodType<Class> f){
            this->operator+=([cls, f](Arguments... arg) -> Return {
                //Calls method pointers on classes.
                return cls.*f;
            });
        }


        void operator+=(std::function<Return(Arguments...)> f){
            functions.push_back(f);
        }
    };
}