#pragma once

#include <TSim/Module/Module.h>
#include <TSim/Simulation/Testbench.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;
using namespace TSim;

class FastCoreTSMgr: public Module
{
public:
    FastCoreTSMgr (string iname, Component *parent);
    virtual void Operation (Message **inmsgs, Message **outmsgs, Instruction *instr);

private:
    /* Port IDs */
    // Read port  
    uint32_t PORT_DynEnd, PORT_AccIdle, PORT_SynEmpty;
    uint32_t PORT_curTS;

    // Write port
    uint32_t PORT_TSparity, PORT_DynFin, PORT_Accidle; 

    /* Internal State */
    bool dyn_end_, acc_idle_, syn_empty_;
    uint8_t cur_tsparity_, next_tsparity_;
};
