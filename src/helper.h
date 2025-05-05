#pragma once

//Helper functions, classes, etc

#include <memory> 
#include <string> 

//A helper class to time an event/function call/etc.
class ITimer
{
    public:
        virtual ~ITimer() = default; 
        virtual long long GetDuration() = 0;

    typedef  std::unique_ptr<ITimer> TimerPtr;
    static  TimerPtr Create();
};

//Helper function to change a string to lower case.
void StringToLower(std::string & str);




