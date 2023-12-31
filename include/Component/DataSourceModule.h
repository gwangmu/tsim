#pragma once

#include <TSim/Module/Module.h>
#include <TSim/Simulation/Testbench.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;
using namespace TSim;


class DataSourceModule: public Module
{
public:
    DataSourceModule (string iname, Component *parent);
    virtual void Operation (Message **inmsgs, Message **outmsgs, Instruction *instr);

private:
    // Port IDs
    uint32_t PORT_DATAOUT;

    // Internal states
    bool is_idle;
    uint32_t counter;
};
