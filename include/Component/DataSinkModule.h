#pragma once

#include <TSim/Module/Module.h>
#include <TSim/Simulation/Testbench.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;
using namespace TSim;

//class ExampleTestbench;
class NeuroSimTestbench;

template <class M, class T>
class DataSinkModule: public Module
{
    VISIBLE_TO(ExampleTestbench);
    VISIBLE_TO(NeuroSimTestbench);

public:
    DataSinkModule (string iname, Component *parent);
    virtual void Operation (Message **inmsgs, Message **outmsgs, Instruction *instr);

private:
    // Port IDs
    uint32_t PORT_DATAIN;

    // Internal states
    T recvdata;
};
