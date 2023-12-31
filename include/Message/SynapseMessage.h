#pragma once

#include <TSim/Pathway/Message.h>
#include <TSim/Utility/Logging.h>

#include <cinttypes>
#include <string>
#include <vector>

struct SynapseMessage: public Message
{
public:
    // NOTE: must provide default constructor
    SynapseMessage () : Message ("SynapseMessage") {}

    SynapseMessage (uint32_t destrhsid, uint32_t weight, uint16_t idx,
            bool TSparity=false)
        : Message ("SynapseMessage", destrhsid)
    {
        this->weight = weight;
        this->idx = idx;
        this->TSparity = TSparity;
    }

public:
    uint32_t weight;
    uint16_t idx;
    bool TSparity;
};

