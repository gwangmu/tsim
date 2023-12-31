#include <Component/PacketConstructor.h>

#include <TSim/Utility/Prototype.h>
#include <TSim/Utility/Logging.h>

#include <Register/EmptyRegister.h>

#include <Message/AxonMessage.h>
#include <Message/SignalMessage.h>
#include <Message/SelectMessage.h>
#include <Message/PacketMessage.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;
using namespace TSim;

PacketConstructor::PacketConstructor (string iname, Component *parent)
    : Module ("PacketConstructor", iname, parent, 1)
{
    IPORT_TSEnd = CreatePort ("ts_end", Module::PORT_INPUT,
            Prototype<SignalMessage>::Get());
    IPORT_Axon = CreatePort ("axon", Module::PORT_INPUT,
            Prototype<AxonMessage>::Get());
    IPORT_boardID = CreatePort ("board_id", Module::PORT_INPUT,
            Prototype<SelectMessage>::Get());

    OPORT_Packet = CreatePort ("packet", Module::PORT_OUTPUT,
            Prototype<PacketMessage>::Get());
    
    Register::Attr regattr (64, 10);
    SetRegister (new EmptyRegister (Register::SRAM, regattr));
}

void PacketConstructor::Operation (Message **inmsgs, Message **outmsgs, 
        Instruction *instr)
{
    SignalMessage *end_msg = static_cast<SignalMessage*> (inmsgs[IPORT_TSEnd]); 
    AxonMessage *axon_msg = static_cast <AxonMessage*> (inmsgs[IPORT_Axon]);
    SelectMessage *sel_msg = static_cast <SelectMessage*> (inmsgs[IPORT_boardID]);

    if(axon_msg && sel_msg)
    {
        INFO_PRINT ("[PkC] Send packet to %d", sel_msg->value);  
        outmsgs[OPORT_Packet] = new PacketMessage (PacketMessage::AXON,
                sel_msg->value, axon_msg->value, axon_msg->len/*, axon_msg->delay*/);
        // FIXME: should insert axon_msg->delay
        //
        inmsgs[IPORT_TSEnd] = nullptr;
        
        GetRegister()->GetWord(0); // Only for check # of packets
    }
    else if(end_msg)
    {
        INFO_PRINT ("[PkC] %s Broadcast end message", 
                GetFullNameWOClass().c_str());
        outmsgs[OPORT_Packet] = new PacketMessage (PacketMessage::TSEND);

        // NOTE: unpop not-yet-processed axon metadata
        inmsgs[IPORT_Axon] = nullptr;
        inmsgs[IPORT_boardID] = nullptr;
    }
    else if(unlikely(axon_msg || sel_msg))
    {
        SIM_ERROR ("Packet constructor receive only either axon or board id", 
                GetFullName().c_str());
    }

    return;
}
