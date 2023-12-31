/***
 * TSim Message prototype (header)
 **/

#pragma once

#include <TSim/Pathway/Message.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;
using namespace TSim;


struct DeltaGMessage: public Message
{
public:
    // NOTE: must provide default constructor
    DeltaGMessage () : Message ("DeltaGMessage") {}

    DeltaGMessage (uint32_t destrhsid, uint64_t deltaG)
        : Message ("DeltaGMessage", destrhsid)
    {
        this->value = deltaG;
    }

public:
    uint64_t value;
};
