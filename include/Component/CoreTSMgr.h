#pragma once

#include <TSim/Module/Module.h>
#include <TSim/Simulation/Testbench.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;
using namespace TSim;

class CoreTSMgr: public Module
{
public:
    CoreTSMgr (string iname, Component *parent);
    virtual void Operation (Message **inmsgs, Message **outmsgs, Instruction *instr);

private:
    enum Neighbor {NBC, NB, AMQ, Acc, SDQ};
    // Neuron Block Controller, Neuron Block, Axon Metadata Queue, Accumulator, Synapse Data Queue

    /* Port IDs */
    // Read port  
    uint32_t PORT_NBC, PORT_NB, PORT_AMQ, PORT_Acc, PORT_SDQ;
    uint32_t PORT_curTS;

    // Write port
    uint32_t PORT_TSparity, PORT_DynFin; 

    /* Internal State */
    bool state[5];
    uint8_t cur_tsparity_, next_tsparity_;
    bool is_dynfin;
};
