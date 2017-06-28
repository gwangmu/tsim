#include <Component/NBController.h>

#include <TSim/Utility/Prototype.h>
#include <TSim/Utility/Logging.h>

#include <Message/DeltaGMessage.h>
#include <Message/StateMessage.h>
#include <Message/SignalMessage.h>
#include <Message/IndexMessage.h>
#include <Message/NeuronBlockMessage.h>

#include <cinttypes>
#include <string>
#include <vector>

NBController::NBController (string iname, Component *parent, uint32_t max_index) 
    : Module ("NBController", iname, parent, 1)
{
    IPORT_DeltaG = CreatePort ("deltaG", Module::PORT_INPUT, 
            Prototype<DeltaGMessage>::Get());
    //IPORT_State = CreatePort ("state", Module::PORT_INPUT, 
    //        Prototype<StateMessage>::Get());
    //IPORT_Tsparity = CreatePort ("tsparity", Module::PORT_INPUT, 
    //        Prototype<SignalMessage>::Get());
    //
    OPORT_NB = CreatePort ("nb", Module::PORT_OUTPUT, 
            Prototype<NeuronBlockInMessage>::Get());
    //OPORT_End = CreatePort ("end", Module::PORT_OUTPUT, 
    //        Prototype<SignalMessage>::Get());
    //OPORT_SRAM = CreatePort ("sram", Module::PORT_OUTPUT, 
    //        Prototype<IndexMessage>::Get());


    /* variable initialization */
    max_idx_ = max_index;
    idx_counter_ = 0;
    ts_parity_ = false;
}

void NBController::Operation (Message **inmsgs, Message **outmsgs, Instruction *instr)
{
    /* Process Inputs */
    StateMessage *state_msg = static_cast<StateMessage*>(inmsgs[IPORT_State]);
    DeltaGMessage *deltaG_msg = static_cast<DeltaGMessage*>(inmsgs[IPORT_DeltaG]);
    SignalMessage *parity_msg = static_cast<SignalMessage*>(inmsgs[IPORT_Tsparity]);

    if(parity_msg)
    {
        ts_parity_ = parity_msg->value;
        idx_counter_ = 0;
    }

    if(state_msg)
    {
        state_reg = state_msg->value;
        state_ready = true;
    }
    if(deltaG_msg)
    {
        deltaG_reg = deltaG_msg->deltaG;
        deltaG_ready = true;
    }

    // Data is ready
    if (deltaG_ready && state_ready)
    {
        outmsgs[OPORT_NB] = new NeuronBlockInMessage (0, idx_counter_, state_reg, deltaG_reg);

        DEBUG_PRINT ("Initiate neuron block (Nidx: %d)", idx_counter_);

        // Increase Neuron index
        idx_counter_++;

        if(idx_counter_ == max_idx_)
        {
            outmsgs[OPORT_End] = new SignalMessage(0, true);
            DEBUG_PRINT ("Finish controller jobs");
        }
        else
        {
            // Make SRAM request
            outmsgs[OPORT_SRAM] = new IndexMessage (-1, idx_counter_);
        }


        deltaG_ready = false;
        state_ready = false;
    }




}
