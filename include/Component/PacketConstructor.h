#pragma once


#include <TSim/Module/Module.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;
using namespace TSim;


class PacketConstructor: public Module
{
public:
    PacketConstructor (string iname, Component *parent);
    virtual void Operation (Message **inmsgs, Message **outmsgs, Instruction *instr);

private:
    /* Port IDs */
    // Input port
    uint32_t IPORT_TSEnd, IPORT_Axon, IPORT_boardID;

    // Output port
    uint32_t OPORT_Packet;
};
