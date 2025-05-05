//Settings parsed from the command line that can control different aspects of the progrem.

#include <cstring>
#include <stdarg.h>

#include "settings.h"

#define DEFALT_SEARCH_FILE "squiz-coding-challenge-lepanto.txt"

CSettings::CSettings()
: m_bHelp(false)
, m_bQuiet(false)
, m_bVerbose(false)
, m_bNoisy(false)
, m_bCreateIndexFile(false)
, m_bNoPrompt(false)
, m_bReturnCodeNumberOfMatches(false)
{
    UpdateFilenames();
}

//Parses the command line to set the programs settings.
//Returns the index of the first invalid setting, otherwise -1
int CSettings::ParseCommandLine(int argc, char* argv[], int iStart)
{
    if ((argc == 0) ||  (iStart >= argc))
        return -1;

    if ((iStart < 0))
        iStart = 0;

    for(int i=iStart; i < argc; ++i)
    {
        #define IF_CHECK_ARG_BOOL(str1,  str2, bVar)  if ((strcmp(argv[i], str1) == 0) || (strcmp(argv[i], str2) == 0)) bVar = true;
        #define IF_CHECK_ARG_CSTR(str1,  str2, sVar)  if ((strcmp(argv[i], str1) == 0) || (strcmp(argv[i], str2) == 0)) { if( ((i+1) < argc) && (argv[i+1][0] != '-')) { ++i; sVar = argv[i]; } else return i; }

             IF_CHECK_ARG_BOOL("-h", "-help",           m_bHelp)
        else IF_CHECK_ARG_BOOL("-q", "-quiet",          m_bQuiet)
        else IF_CHECK_ARG_BOOL("-v", "-verbose",        m_bVerbose)
        else IF_CHECK_ARG_BOOL("-n", "-Noisy",          m_bNoisy)
        else IF_CHECK_ARG_BOOL("-i", "-index",          m_bCreateIndexFile)
        else IF_CHECK_ARG_BOOL("-np", "-noprompt",      m_bNoPrompt)
        else IF_CHECK_ARG_BOOL("-rc", "-returncode",    m_bReturnCodeNumberOfMatches)
        else IF_CHECK_ARG_CSTR("-s", "-search",         m_strSearchFile)
        else IF_CHECK_ARG_CSTR("-f", "-find",           m_strSearchText)
        else
            return i;  //Invalid command line index.
    }

    m_strIndexFile.clear();  //Trigger the rename (if ParseCommandLine() gets called multiple times. Paranoia++).
    UpdateFilenames();

    if(GetHelp())
    {
        const char * strFile = (argc > 0) ? argv[0] : "squiz";
        Print(eConsole, "%s [-h] -[q] [-v] [-n] [-i] [-np] [-rc] [-s SearchFileName] [-f TextToFind]\n", strFile);
        Print(eConsole, "\tWhere: \n");
        Print(eConsole, "\t\t -h   Displays this help text.\n");
        Print(eConsole, "\t\t -q   Quiet mode. No console output.\n");
        Print(eConsole, "\t\t -v   Verbose mode. Verbose messages will display on the console.\n");
        Print(eConsole, "\t\t -n   Noisy mode. Noisy messages will display on the console.\n");
        Print(eConsole, "\t\t -i   Recreate the index file.\n");
        Print(eConsole, "\t\t -np  No prompt text for the search text will be displayed.\n");
        Print(eConsole, "\t\t -rc  The return code is set to the number of matches.\n");
        Print(eConsole, "\t\t -s   Specify the search file. Defaults to: %s\n", DEFALT_SEARCH_FILE);
        Print(eConsole, "\t\t -f   Specify the text to search for\n");
        Print(eConsole, "\tExample: %s -i -f \"Green Apples\" -s mydata.txt\n", strFile);
        Print(eConsole, "\t\t This will search for either of the words, Green or Apples, in\n");
        Print(eConsole, "\t\t mydata.txt and output the best match.\n");
    }

    return -1;
}

void CSettings::UpdateFilenames()
{
    if(m_strSearchFile.empty())
        m_strSearchFile = DEFALT_SEARCH_FILE;

    //Index filename  depends on the search filename
    if(m_strIndexFile.empty())
    {
        m_strIndexFile = GetSearchFile();
        m_strIndexFile += ".idx";
    }
}

bool CSettings::GetHelp() const
{
    return m_bHelp;
}

const char * CSettings::GetSearchFile() const
{
    return  m_strSearchFile.c_str();
}

const char * CSettings::GetIndexFile() const
{
    return  m_strIndexFile.c_str();
}

bool CSettings::GetCreateIndexFile() const
{
    return m_bCreateIndexFile;
}

const char * CSettings::GetSearchText() const
{
    return m_strSearchText.c_str();
}

bool CSettings::SetSearchText(const char * strSearch)
{
    if((strSearch == nullptr) || (*strSearch == '\0'))
        m_strSearchText.clear();
    else
        m_strSearchText = strSearch;

    return m_strSearchText.empty() == false;
}

bool CSettings::GetNoPrompt() const
{
    return m_bNoPrompt;
}

bool CSettings::GetReturnCodeMatches() const
{
    return m_bReturnCodeNumberOfMatches;
}

//Function to print text to console depending on verbose/quiet/noisy settings.
int  CSettings::Print(EPrintType eType, const char *strFormat, ...) const
{
    if ((m_bQuiet) && (eType != eConsoleAlways))
        return 0;

    if((eType==eVerbose) && (m_bVerbose == false))
        return 0;

    if((eType==eNoisy) && (m_bNoisy == false))
        return 0;

    va_list args;
    va_start(args, strFormat); // Initialize the va_list with the last fixed parameter

    int iRet = vfprintf(eType == eError ? stderr : stdout, strFormat, args);

    va_end(args); 

    return iRet;
}


