#pragma once

#include <memory> 
#include <vector> 
#include <string> 
#include "settings.h"

//An interface used to create/load/search index data.
//Functionality controlled by the CSettings.

typedef std::vector<string>                 StringArray;

class IIndices
{
    public:
        virtual ~IIndices() = default; 

        virtual void CreateIndexFile() = 0;
        virtual bool LoadIndexFile() = 0;
        virtual StringArray Search() = 0;

        typedef  std::unique_ptr<IIndices> IndicesPtr;
        static  IndicesPtr Create(const CSettings & settings);
};