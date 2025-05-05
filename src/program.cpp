/* 
    This file contains the a Program class that searches a text file for a line of text.
    It does this by indexing the text file, saving the index file, and then using the
    index data to search for the words in the line of text.

    A valid match is the lines that contains the most number of words.

    It is case insensitive and ignores non-alpha text.
*/

#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <stdarg.h>

#include "program.h"
#include "settings.h"
#include "index.h"

#define VERBOSE(fmt, ...)                   m_Settings.Print(CSettings::eVerbose,       "Verbose - " fmt "\n", ##__VA_ARGS__)
#define PRINTLINE(fmt, ...)                 m_Settings.Print(CSettings::eConsole,                    fmt "\n", ##__VA_ARGS__)
#define PRINTLINE_IGNORE_QUIET(fmt, ...)    m_Settings.Print(CSettings::eConsoleAlways,              fmt "\n", ##__VA_ARGS__)
#define PRINT(fmt, ...)                     m_Settings.Print(CSettings::eConsole,                    fmt,      ##__VA_ARGS__)


class Program
{
    private:
        CSettings   m_Settings;

    public:
        Program()
        {
        }

        //Run a program that searches for the best match of a string in a text file that contains one or more lines of text.
        //Throws ProgramException.
        int Run(int argc, char* argv[])
        {
            //Parse the command line to get the programs settings.
            if (CreateSettings(argc, argv) == false)
                return 0;

            //Does the search file exist?
            VerifySearchFile();

            //Open/create the index file.
            auto Indices = VerifyIndexFile();

            int iRet = 0;
            //Prompt for the text to find (or use the command line settings).
            if(VerifyFindText())
            {
                StringArray aMatches  = Indices->Search();

                if(aMatches.empty())
                    PRINTLINE("No matches found.");
                else
                {
                    if(aMatches.size() > 1)
                        PRINTLINE("Found %u matches for: %s", aMatches.size(), m_Settings.GetSearchText());

                    for(const auto & strMatch : aMatches)
                        PRINTLINE("%s", strMatch.c_str());
                }

                if (m_Settings.GetReturnCodeMatches())
                    iRet = aMatches.size();
            }

            return iRet;
        }

    private:
        bool CreateSettings(int argc, char* argv[])
        {
            //Parse the command line to get the programs settings.
            if(int iInvalid = m_Settings.ParseCommandLine(argc, argv); iInvalid >= 0)
                ThrowProgramException(-1, "Error: Invalid command line %i: %s\n", iInvalid, argv[iInvalid]);

            if(m_Settings.GetHelp())
                return false;

            VERBOSE("Command line parsed.");
            return true;
        }

        void VerifySearchFile()
        {
            //Check the existence of search file
            const char * strSearchFile = m_Settings.GetSearchFile();
            if (filesystem::exists(strSearchFile) == false)
                ThrowProgramException(-2, "Search file does not exist: %s", strSearchFile);
            VERBOSE("Search file found: %s", strSearchFile);
        }

        IIndices::IndicesPtr VerifyIndexFile()
        {
            //Check the index file (the search file + .idx)
            const char * strIndexFile  = m_Settings.GetIndexFile();

            bool bCreateIndexFile = false;
            bool bGetCreateIndexFile = m_Settings.GetCreateIndexFile();
            if (bGetCreateIndexFile || filesystem::exists(strIndexFile) == false)
            {
                if(bGetCreateIndexFile)
                    VERBOSE("Creating index file: %s...", strIndexFile);
                else
                    VERBOSE("Index file does not exist: %s.  Recreating...", strIndexFile);

                bCreateIndexFile = true;
            }
            else
            {
                VERBOSE("Index file found: %s.  Loading...", strIndexFile);

                auto Indices = IIndices::Create(m_Settings);
                if(Indices->LoadIndexFile() == false)
                {
                    VERBOSE("Index file is too old: %s.  Recreating...", strIndexFile);
                    bCreateIndexFile = true;
                }
                else
                    return Indices;
            }

            if(bCreateIndexFile)
            {
                auto Indices = IIndices::Create(m_Settings);
                Indices->CreateIndexFile();
                if (filesystem::exists(strIndexFile) == false)
                    ThrowProgramException(-3, "Can't create index file: %s", strIndexFile);
                return Indices;
            }

            return nullptr;  //Never gets here.
        }

        bool VerifyFindText()
        {
            //Prompt (or use the command line option) a search line.
            {
                std::string strSearch(m_Settings.GetSearchText());
                if(strSearch.empty())
                {
                    if(m_Settings.GetNoPrompt() == false)
                        PRINT("Type a line of text to search for: ");

                    std::getline(std::cin, strSearch);

                    //if(m_Settings.GetNoPrompt() == false)
                    //    PRINTLINE("");

                    if(strSearch.empty())
                    {
                        PRINTLINE("Nothing to search for.");
                        return false;
                    }

                    return m_Settings.SetSearchText(strSearch.c_str());
                }
                return true;
            }
        }
};

//Helper function to create and throw a ProgramException.
void ThrowProgramException(int iError, const char * strFormat, ...)
{
    int iSize = -1;
    {
        va_list args;
        va_start(args, strFormat); // Initialize the va_list with the last fixed parameter
        iSize = std::vsnprintf(nullptr, 0, strFormat, args);
        va_end(args); 
    }

    if(iSize > 0)
    {
        std::string strError(iSize+1, 0);

        va_list args;
        va_start(args, strFormat); // Initialize the va_list with the last fixed parameter
        iSize = std::vsnprintf(strError.data(), strError.size(), strFormat, args);
        va_end(args); 

        if(iSize > 0)
            throw make_pair(std::move(strError), iError);
    }

    throw make_pair("Internal Format Error", iError);
}

//Throws ProgramException.
int RunProgram(int argc, char* argv[])
{
    Program P;
    return P.Run(argc, argv);
}