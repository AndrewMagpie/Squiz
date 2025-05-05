#pragma once

#include <string>
using namespace std;

//Settings parsed from the command line that can control different aspects of the progrem.
class CSettings
{
public:
                CSettings();

            int ParseCommandLine(int argc, char* argv[], int iStart = 1);
   const char * GetSearchFile() const;
   const char * GetIndexFile() const;
           bool GetCreateIndexFile() const;
   const char * GetSearchText() const;
           bool SetSearchText(const char * strSearch);
           bool GetNoPrompt() const;
           bool GetReturnCodeMatches() const;
           bool GetHelp() const;

            enum EPrintType {eConsole, eConsoleAlways, eError, eVerbose, eNoisy};
            int Print(EPrintType eType, const char *strFormat, ...) const;

private:
           void UpdateFilenames();

private:
    bool    m_bHelp;                            //Display help on teh command line.
    bool    m_bQuiet;                           //Console is quiet (except for error).
    bool    m_bVerbose;                         //Console allows verbose messages.
    bool    m_bNoisy;                           //Console allows noisy messages.
    bool    m_bCreateIndexFile;                 //Force recreation of index file.
    bool    m_bNoPrompt;                        //Dont output instructions to type for a search line of text.
    bool    m_bReturnCodeNumberOfMatches;       //The return code of the program is the number of matches.
    string  m_strSearchText;                    //The line of text to search for.
    string  m_strSearchFile;                    //The file to search.
    string  m_strIndexFile;                     //The that contains the index data.
}; // CSettings