#pragma once

#include <TSim/Pathway/Message.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;

struct DramMessage: public Message
{
public:
    // NOTE: must provide default constructor
    DramMessage () : Message ("DramMessage") {}

    DramMessage (uint32_t destrhsid, uint64_t val32, uint16_t val16, bool intra_board, bool idx)
        : Message ("DramMessage", destrhsid)
    {
        this->val32 = val32;
        this->val16 = val16;
        this->intra_board = intra_board;
        this->target_idx = idx;
    }

public:
    uint64_t val32;
    uint16_t val16;
    bool intra_board;
    uint8_t target_idx;
};
