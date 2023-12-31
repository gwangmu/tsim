// DEPRECATED

#if 0
#include <Component/PCIeController.h>

#include <TSim/Utility/Prototype.h>
#include <TSim/Utility/Logging.h>

#include <Message/AxonMessage.h>
#include <Message/SignalMessage.h>
#include <Message/SelectMessage.h>
#include <Message/PacketMessage.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;
using namespace TSim;

PCIeController::PCIeController (string iname, Component *parent)
    : Module ("PCIeController", iname, parent, 1)
{
    IPORT_Packet = CreatePort ("packet_in", Module::PORT_INPUT,
            Prototype<PacketMessage>::Get());
    
    OPORT_Packet = CreatePort ("packet_out", Module::PORT_OUTPUT,
            Prototype<PacketMessage>::Get());

    /// TODO Implement Board-to-Board communication
}

void PCIeController::Operation (Message **inmsgs, Message **outmsgs, Instruction *instr)
{
    PacketMessage *pkt_msg = static_cast<PacketMessage*> (inmsgs[IPORT_Packet]);

    if(pkt_msg)
    {
        int rhs = pkt_msg->rhs;
        
        // Send packet
        SIM_WARNING ("PCIe controller is not implemented. DROP the packet.", GetFullName().c_str());
    }
}
#endif
