#include <TSim/Utility/Prototype.h>
#include <TSim/Utility/Logging.h>

#include <Component/DataSinkModule.h>
#include <Message/NeuronBlockMessage.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;


DataSinkModule::DataSinkModule (string iname, Component *parent)
    : Module ("DataSinkModule", iname, parent, 1)
{
    // create ports
    PORT_DATAIN = CreatePort ("datain", Module::PORT_INPUT, Prototype<NeuronBlockOutMessage>::Get());
    DEBUG_PRINT ("size=%zu", pname2port.size());
    CreatePort ("dummy", Module::PORT_OUTPUT, Prototype<NeuronBlockOutMessage>::Get());

    DEBUG_PRINT ("size=%zu", pname2port.size());
    recvdata = 0;
    DEBUG_PRINT ("size=%zu", pname2port.size());
}

// NOTE: called only if not stalled
void DataSinkModule::Operation (Message **inmsgs, Message **outmsgs, Instruction *instr)
{
    NeuronBlockOutMessage *inmsg = static_cast<NeuronBlockOutMessage *>(inmsgs[PORT_DATAIN]);

    if (inmsg) 
    {
        recvdata = inmsg->idx;
        DEBUG_PRINT ("idx = %u, spike = %d", recvdata, inmsg->spike);
    }
}
