#pragma once


#include <TSim/Module/Module.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;


class PCIeController: public Module
{
public:
    PCIeController (string iname, Component *parent);
    virtual void Operation (Message **inmsgs, Message **outmsgs, 
            const uint32_t *outque_size, Instruction *instr);

private:
    /* Port IDs */
    // Input port
    uint32_t IPORT_Packet, IPORT_PCIe;

    // Output port
    uint32_t OPORT_Packet, OPORT_PCIe;
};