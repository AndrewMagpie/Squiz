/*
CIndicesPimpl is a private implementation of a class that implements IIndices,
It is responsible for:
    1. Creating an index file. A text file that based on the search file name.
    2. Loading  an index file.
    3. Searching for a text string by querying the index data word by word.
       A match is one or more lines from search file that has the most words in the text string.

The index file format (text):
    [Last modified time of the search file]
    [Number of lines in the search file, L]
    [Offset of line 1] [Offset of line 2]  ... [Offset of line L-1] [Offset of line L]
    [Number of unique words in the search file, U]
    [Word 1] [Number of lines Word 1 is in, WC1] [1st Line Word 1 is in] [2nd Line Word 1 is in] ... [WC1th - 1 Line Word 1 is in]  [WC1th Line Word 1 is in]
    [Word 2] [Number of lines Word 2 is in, WC2] [1st Line Word 2 is in] [2nd Line Word 2 is in] ... [WC2th - 1 Line Word 2 is in]  [WC2th Line Word 2 is in]
    ...
    [Word U] [Number of lines Word U is in, WCU] [1st Line Word U is in] [2nd Line Word 2 is in] ... [WCUth - 1 Line Word U is in]  [WCUth Line Word U is in]

From this format
    An array of lines offsets can the loaded (see m_aLineOffsets below).
    A dictionary where the key is a string (unique word) and the data is an array of lines can the loaded (see m_mapWordInLines below).

    By enumerating words of the text string (users input), each word can be looked up in the dictionary to determine which lines its in.
    Summing each words line count will determine which lines should be considered a match!
*/

#include <filesystem>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <sys/stat.h>

#include "index.h"
#include "program.h"
#include "settings.h"
#include "helper.h"

#define NOISY(fmt, ...)             m_Settings.Print(CSettings::eNoisy,   "  Noisy - " fmt "\n", ##__VA_ARGS__)
#define VERBOSE(fmt, ...)           m_Settings.Print(CSettings::eVerbose, "Verbose - " fmt "\n", ##__VA_ARGS__)
#define VERBOSE_START(fmt, ...)     m_Settings.Print(CSettings::eVerbose, "Verbose - " fmt,      ##__VA_ARGS__)
#define VERBOSE_NEXT(fmt, ...)      m_Settings.Print(CSettings::eVerbose,              fmt,      ##__VA_ARGS__)
#define VERBOSE_END(fmt, ...)       m_Settings.Print(CSettings::eVerbose,              fmt "\n", ##__VA_ARGS__)
#define PRINTLINE(fmt, ...)         m_Settings.Print(CSettings::eConsole,              fmt "\n", ##__VA_ARGS__)
#define PRINT(fmt, ...)             m_Settings.Print(CSettings::eConsole,              fmt, ##__VA_ARGS__)

//#define _DEBUG_HELP_ 1

typedef std::vector<unsigned int>           UINTArray;
typedef std::map<std::string, UINTArray>    StringToUINTArray;
typedef std::map<unsigned int, UINTArray>   UINTToUINTArray;

class CIndicesPimpl : public IIndices
{
public:
    CIndicesPimpl(const CSettings & settings)
        : m_Settings(settings)
    {
    }

    //Create the index file by enumerating the search file.
    // Overrides the interface function in IIndices.
    // Throws ProgramException.
    void CreateIndexFile() override
    {
        auto timerCreate = ITimer::Create();
        CreateIndexFileEx();
        VERBOSE("Loading of indices took %i ms", timerCreate->GetDuration()); 
    }

    //Loads the index file.
    // Overrides the interface function in IIndices.
    // Returns true if the load is successful.
    // Returns false if a recreation of the index file is required.
    // Throws ProgramException.
    bool LoadIndexFile() override
    {
        auto timerCreate = ITimer::Create();
        bool bRet = LoadIndexFileEx();
        if(bRet)
            VERBOSE("Loading of indices took %i ms", timerCreate->GetDuration()); 
        return bRet;
    }

    //Search for a the text string (set in the settings) by finding its words in the index data.
    // Overrides the interface function in IIndices.
    // Returns an aray of string that match.
    // Throws ProgramException.
    StringArray Search() override
    {
        auto timerCreate = ITimer::Create();
        StringArray aMatches = SearchEx();
        VERBOSE("Searching took %i ms", timerCreate->GetDuration()); 
        return aMatches;
    }

private:

    void Clear()
    {
        m_lastModifyTime = 0;
        m_aLineOffsets.clear();
        m_mapWordInLines.clear();
    }

    //Create the index file.
    // Throws ProgramException.
    void CreateIndexFileEx()
    {
        Clear();

        const char * strIndexFile = m_Settings.GetIndexFile();

        // Remove the current index file if it exists.
        if (filesystem::exists(strIndexFile))
        {
            if (filesystem::remove(strIndexFile) == false)
                ThrowProgramException(-10, "Cant remove indexfile: %s", strIndexFile);
        }

        // Create the index data by enumerating the search file.
        CreateIndexDataFromSearchFile();

        #ifdef _DEBUG_HELP_
        PrintIndices();
        #endif

        // Save the index data to the index file.
        SaveIndexFileData();
    }

    // Create the index data by enumerating the search file.
    // Throws ProgramException.
    void CreateIndexDataFromSearchFile()
    {
        const char * strSearchFile = m_Settings.GetSearchFile();

        // Open the search file, throw if unable to.
        VERBOSE("Loading search file.");
        std::ifstream fSearch(strSearchFile);
        if (fSearch.is_open() == false)
            ThrowProgramException(-11, "Cant open the search file: %s", strSearchFile);

        // Last modify time of search file.
        struct stat result = {0};
        if (stat(strSearchFile, &result) == 0)
        {
            m_lastModifyTime = result.st_mtime;
            VERBOSE("Search file last modify time: %i", result.st_mtime);
        }
        //Ignore if cant get stats.

        //Enumerate the lines in the search file.
        unsigned int iLineCount = 0;
        string strLine;
        while(true)
        {
            //Get the current file offset and add it to m_aLineOffsets
            std::streampos pos = fSearch.tellg();
            std::streamoff offset = static_cast<std::streamoff>(pos);
            unsigned int uPos =  offset;
            if(!getline(fSearch, strLine))
                break;
            m_aLineOffsets.push_back(uPos);

            StringToLower(strLine);

            VERBOSE("Line %6i, Pos %u: %s", ++iLineCount, uPos, strLine.c_str());

            EnumerateWords(strLine, [&](string && strWord) -> bool
            {
                auto [iterator, bNew] = m_mapWordInLines.try_emplace(std::move(strWord), UINTArray {iLineCount});
                if(bNew == false)
                {
                    if(iterator->second.back() != iLineCount)
                        iterator->second.push_back(iLineCount);
                }
                return true;
            });
        }

        fSearch.close();  //If above code throws, this gets auto closed in ifstream destructor.
    }

    //Loads the index file.
    // Returns true if the load is successful.
    // Returns false if a recreation of the index file is required.
    // Throws ProgramException
    bool LoadIndexFileEx()
    {
        Clear();

        const char * strIndexFile = m_Settings.GetIndexFile();
        const char * strSearchFile = m_Settings.GetSearchFile();

        std::ifstream fIndex(strIndexFile);
        if (fIndex.is_open() == false)
            ThrowProgramException(-12, "Cant open the index file: %s", strIndexFile);
    
        //Compare the modify time of the index file and the search file.
        //  If they differ, then return false to trigger the recreation of the index file.
        time_t modTimeOfIndex = 0;
        {
            fIndex >> modTimeOfIndex;
            VERBOSE("Index file last modified time:  %i", modTimeOfIndex);
            struct stat result = {0};
            if (stat(strSearchFile, &result) == 0)
            {
                result.st_mtime;
                VERBOSE("Search file last modified time: %i", result.st_mtime);
                if(result.st_mtime != modTimeOfIndex)
                {
                    VERBOSE("Search file has been modified since the index file was created.");
                    return false;
                }
            }
        }
        fIndex.close();  //If above code throws or returns early, this gets auto closed in ifstream destructor.

        //Continue loading the data from the file
        return LoadIndexFileData();
    }

    //Searches the indices for a text line (set in the settings), word by word.
    // Returns an aray of lines that match.
    // Throws ProgramException.
    StringArray SearchEx() const
    {
        const char * strSearch = m_Settings.GetSearchText();
        if(strSearch == nullptr)
            ThrowProgramException(-13, "Invalid search text!");


        UINTToUINTArray mapLineNumToWordIndex;  //Key: Line number,  Value: word index (into strSearch)
        StringArray aSearchStrings;             //The individual words in strSearch
        unsigned int uWordIndex = 0;            //The current word being enumerated.

        //For each word in strSearch
        EnumerateWords(strSearch, [&](string && strWord) -> bool
        {
            StringToLower(strWord);
            aSearchStrings.push_back(strWord.c_str());

            VERBOSE_START("Search Word[%2u]: %15s -> Lines:", uWordIndex, strWord.c_str());

            //Find the word in the indices
            auto it = m_mapWordInLines.find(strWord);
        
            if (it == m_mapWordInLines.end()) 
            {
                VERBOSE_END(" Not found.");
                ++uWordIndex;
                return {};
            }

            //For each line the word was found in, store the line it in mapLineNumToWordIndex
            for (const unsigned int & uLine : it->second)
            {
                VERBOSE_NEXT(" %u", uLine);

                auto [iterator, bNew] = mapLineNumToWordIndex.try_emplace(uLine, UINTArray {uWordIndex});
                if(bNew == false)
                {
                    if(iterator->second.back() != uWordIndex)
                        iterator->second.push_back(uWordIndex);
                }
            }

            VERBOSE_END("");
            ++uWordIndex;
            return true;
        });

        //Find the lines with the maximum matches.
        unsigned int uMaxCount = 0;
        UINTArray aLinesWithMaxCount;

        for(const auto & Line : mapLineNumToWordIndex)
        {
            //Track which lines has the greatest number of matching words.
            unsigned int uThisMaxCount = Line.second.size();
            if(uThisMaxCount >= uMaxCount)
            {
                if(uThisMaxCount > uMaxCount)
                    aLinesWithMaxCount.clear();

                aLinesWithMaxCount.push_back(Line.first);
                uMaxCount = uThisMaxCount;
            }

            VERBOSE_START("Line %3u contains the %2u words:", Line.first, Line.second.size());
            for(const auto & aWordIndexes : Line.second)
            {
                VERBOSE_NEXT(" %s", aSearchStrings[aWordIndexes].c_str());
            }
            VERBOSE_END("");
        }

        //Get the full lines of text from the search file (the index data contains the line offsets).
        StringArray aMatchingLines;
        VERBOSE_START("%u matching words on line(s): ", uMaxCount);
        for(const unsigned int uLine : aLinesWithMaxCount)
        {
            string strMatch = GetSourceFileLine(uLine-1);
            VERBOSE_NEXT(" %u", uLine);
            if (strMatch.empty() == false)
                aMatchingLines.push_back(std::move(strMatch));
            else 
                VERBOSE_NEXT(" empty");

        }
        VERBOSE_END("");
        return aMatchingLines;
    }

    string GetSourceFileLine(unsigned int uLine) const
    {
        if (uLine < m_aLineOffsets.size())
        {
            unsigned int uFileOffset = m_aLineOffsets[uLine];

            const char * strSearchFile = m_Settings.GetSearchFile();

            // Open the search file, throw if unable to.
            //VERBOSE("Loading search file for line %u.", uLine);
            std::ifstream fSearch(strSearchFile);
            if (fSearch.is_open() == false)
                ThrowProgramException(-14, "Cant open the search file: %s", strSearchFile);

            //VERBOSE("Moving search file to offset %u.",  uFileOffset);
            fSearch.seekg(uFileOffset, std::ios::beg); 
            if (!fSearch)
                ThrowProgramException(-15, "Cant seek to %u in the search file: %s", uFileOffset, strSearchFile);

            std::string strSearchline;
            std::getline(fSearch, strSearchline);
            //VERBOSE("Got line %u from search file: %s", uLine, strSearchline.c_str());

            return strSearchline;
        }

        return "";
    }

    //Loads the index file data.
    // returns true if the load is successful.
    // returns false if a recreation of the index file is required.
    // Throws ProgramException.
    bool LoadIndexFileData(time_t modTimeOfIndex = 0)
    {
        const char * strIndexFile = m_Settings.GetIndexFile();
        std::ifstream fIndex(strIndexFile);
        if (fIndex.is_open() == false)
            ThrowProgramException(-16, "Cant open the index file: %s", strIndexFile);
        
        fIndex >> m_lastModifyTime;                                     //Last modify time

        //Read line offsets. The count, then that number of offsets.
        size_t nCount = 0;
        fIndex >> nCount;                                               //Number of lines
        VERBOSE("Found %u line offsets.",nCount);
        m_aLineOffsets.resize(nCount);
        for(size_t i = 0; i<nCount; ++i)
            fIndex >> m_aLineOffsets[i];                                //The offset of each line

        //Read: word count, [ word, line count [lines] ]
        nCount = 0;
        fIndex >> nCount;                                               //The numberof unique words
        VERBOSE("Found %u words.",nCount);
        for(size_t i = 0; i<nCount; ++i)
        {
            string strWord;
            fIndex >> strWord;                                          //Each word

            size_t nLineCount = 0;
            fIndex >> nLineCount;                                       //The number of lines the word appears.

            UINTArray aLinesForWord(nLineCount, 0);

            for(size_t j = 0; j<nLineCount; ++j)
                fIndex >> aLinesForWord[j];                             //Each line the word appears ins.

            //Create the map of words to the lines it appears in
            string strWordDup = strWord.c_str();
            auto [_, bNew] = m_mapWordInLines.try_emplace(std::move(strWord), std::move(aLinesForWord));
            if(bNew == false)
            {
                //Found the word twice in the index file.  This shouldn't happen!
                VERBOSE("Found word twice: %s",strWordDup.c_str());
                Clear();
                return false; //Trigger the recreation of the index file.
            }
        }

        fIndex.close();  //If above code throws or returns early, this gets auto closed in ifstream destructor.
        return true;  //Successfully loaded.
    }

    //Saves the index data to file.
    // Throws ProgramException.
    void SaveIndexFileData() const
    {
        const char * strIndexFile = m_Settings.GetIndexFile();

        std::ofstream fIndex(strIndexFile);
        if (fIndex.is_open() == false)
            ThrowProgramException(-17, "Cant create the index file: %s", strIndexFile);

        fIndex << m_lastModifyTime << std::endl;                        //Last modify time

        fIndex << m_aLineOffsets.size() << std::endl;                   //Number of lines
        for(const unsigned int & uLineOffset : m_aLineOffsets)
            fIndex << uLineOffset << " ";                               //The offset of each line
        fIndex << std::endl;

        fIndex << m_mapWordInLines.size() << std::endl;                 //The numberof unique words
        for(const auto & WordInLines : m_mapWordInLines)
        {
            fIndex << WordInLines.first.c_str() << " ";                 //Each word
            fIndex << WordInLines.second.size() << " ";                 //The number of lines the word appears.
            for(const unsigned int uLines : WordInLines.second)
                fIndex << uLines << " ";                                //Each line the word appears ins.
            fIndex << std::endl;
        }
    }

    #ifdef _DEBUG_HELP_
    //Helper function to print the index data.
    void PrintIndices()
    {
        PRINTLINE("Indices: ");
        PRINTLINE("  Last Modified: %u", m_lastModifyTime);
        PRINTLINE("          Lines: %u", m_aLineOffsets.size());
    
        for(const auto & data : m_mapWordInLines)
        {
            PRINT("          Word %12s in lines: ", data.first.c_str());
    
            for(const auto & uLine : data.second)
            PRINT("%u ", uLine);
    
            PRINTLINE("");
        }
    }
    #endif

    //Helper method to enumerate words in a string, calls a callback (lambda?) for each word.
    template <typename Callback>
    void EnumerateWords(const string & strLine, Callback cb) const
    {
        //Enumerate the words in the line of text.  Ignore non-alphas.
        auto itRealEnd = strLine.end();
        auto itEnd = strLine.begin();   //Setting the starting point.  I know its the end, but read the code below.
        while (itEnd != itRealEnd)
        {
            auto itStart = std::find_if(itEnd,itRealEnd, [](unsigned char c) { return std::isalpha(c);  });
            if(itStart == itRealEnd)  break;

            itEnd = std::find_if(itStart, itRealEnd, [](unsigned char c) { return !std::isalpha(c);  });

            std::string strWord(itStart, itEnd);

            if(cb(std::move(strWord))==false)
                break;
        }
    }

 private:
    const CSettings &   m_Settings;             //Determines filenames, settings, etc.
    time_t              m_lastModifyTime;       //The time the search file was last modified.
    UINTArray           m_aLineOffsets;         //The start of each line within the search file.
    StringToUINTArray   m_mapWordInLines;       //The list of lines each word is in. The values in the array are positions in m_aLineOffsets;
}; // CIndicesPimpl

//Private implementation of IIndices
//Private implementation of IIndices by instantiating CIndicesPimpl.
IIndices::IndicesPtr IIndices::Create(const CSettings & settings)
{
    return std::make_unique<CIndicesPimpl>(settings);
}