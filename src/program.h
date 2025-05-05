#pragma once

#include <utility>
#include <string>

typedef std::pair<std::string, int> ProgramException;

//Helper function to create and throw a ProgramException.
void ThrowProgramException(int iError, const char * strFormat, ...);

//Run the program by parsing the commandline for program settings, then searches a text file users input.
//Throws IntStringPair
int RunProgram(int argc, char* argv[]); 