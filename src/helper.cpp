#include <algorithm>
#include <cctype>
#include <chrono> 
#include "helper.h"

//A helper class to time an event/function call/etc.
class CTimerPimpl : public ITimer
{
    private:
        typedef  std::chrono::time_point<std::chrono::high_resolution_clock> NowType;
        NowType m_Start;

    public:
        CTimerPimpl()
        : m_Start(std::chrono::high_resolution_clock::now())
        {
        }
        
        long long GetDuration() override
        {
            auto duration = std::chrono::high_resolution_clock::now() - m_Start;
            return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
        }
};

//Private implementation of ITimer by instantiating CTimerPimpl.
ITimer::TimerPtr ITimer::Create()
{
    return std::make_unique<CTimerPimpl>();
}

//Helper function to change a string to lower case.
void StringToLower(std::string & str)
{
    std::transform(str.begin(), str.end(), str.begin(),[](unsigned char c) { return std::tolower(c); });
}