#include "program.h"

//https://stackoverflow.com/questions/1961209/making-some-text-in-printf-appear-in-green-and-red
#define RED     "\033[31m"
#define RESET   "\033[0m"

int main(int argc, char* argv[])
 {
    try
    {
        return RunProgram(argc, argv);  //Throws IntStringException
    }
    catch (const ProgramException& e)
    {
        fprintf(stderr, RED "Error(%i) - %s\n" RESET, e.second, e.first.c_str());
        return e.second;
    }
    catch (const std::exception& e)
    {
        fprintf(stderr, RED "!Error! - %s\n" RESET, e.what());
        return -100;
    }
    catch (...)
    {
        fprintf(stderr, RED "Unknown Error!\n" RESET);
        return -200;
    }
}
