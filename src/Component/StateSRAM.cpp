#include <Component/StateSRAM.h>

#include <Message/IndexMessage.h>
#include <Message/DeltaGMessage.h>
#include <Message/StateMessage.h>

#include <TSim/Utility/Prototype.h>
#include <TSim/Utility/Logging.h>

StateSRAM::StateSRAM (string iname, Component* parent,
        uint32_t row_size, uint32_t col_size)
    : Module ("StateSRAM", iname, parent, 1)
{
    this->row_size_ = row_size;
    this->col_size_ = col_size;

    read_n = 0;
    write_n = 0;
    
    RPORT_addr = CreatePort ("r_addr", Module::PORT_INPUT,
            Prototype<IndexMessage>::Get());
    RPORT_data = CreatePort ("r_data", Module::PORT_OUTPUT,
            Prototype<StateMessage>::Get());

    WPORT_addr = CreatePort ("w_addr", Module::PORT_INPUT,
            Prototype<IndexMessage>::Get());
    WPORT_data = CreatePort ("w_data", Module::PORT_INPUT,
            Prototype<StateMessage>::Get());

}


void StateSRAM::Operation (Message **inmsgs, Message **outmsgs, Instruction *instr)
{
    /* Read */
    IndexMessage *raddr_msg = static_cast<IndexMessage*>(inmsgs[RPORT_addr]);

    if(raddr_msg)
    {
        uint32_t read_addr = raddr_msg->value;

        State value = 0;
        outmsgs[RPORT_data] = new StateMessage (0, value);

        INFO_PRINT("[StSRAM] Receive read request, and send message");
    }


    /* Write */
    IndexMessage *waddr_msg = static_cast<IndexMessage*>(inmsgs[WPORT_addr]);
    StateMessage *wdata_msg = static_cast<StateMessage*>(inmsgs[WPORT_data]);

    if(waddr_msg && wdata_msg)
    {
        INFO_PRINT("[StSRAM] Receive write request");
        uint32_t write_addr = waddr_msg->value;
    }
    else if (unlikely (waddr_msg || wdata_msg))
    {
        SIM_ERROR ("SRAM receive only either an addr or data", GetFullName().c_str()); 
        return;
    }
}
