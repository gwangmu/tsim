#include <Component/NeuroSim.h>

#include <TSim/Pathway/Wire.h>
#include <TSim/Pathway/FanoutWire.h>
#include <TSim/Pathway/RRFaninWire.h>
#include <TSim/Utility/Prototype.h>

#include <Component/DataSourceModule.h>
#include <Component/DataSinkModule.h>
#include <Component/DataEndpt.h>

#include <Component/NeuroChip.h>
#include <Component/Propagator.h>
#include <Component/Controller.h>

#include <Message/AxonMessage.h>
#include <Message/ExampleMessage.h>
#include <Message/NeuronBlockMessage.h>
#include <Message/DeltaGMessage.h>
#include <Message/StateMessage.h>
#include <Message/SignalMessage.h>
#include <Message/SelectMessage.h>
#include <Message/IndexMessage.h>
#include <Message/NeuronBlockMessage.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;

NeuroSim::NeuroSim (string iname, Component *parent)
    : Component ("NeuroSim", iname, parent)
{
    // NOTE: children automatically inherit parent's clock
    //  but they can override it by redefining their own.
    SetClock ("main");

    /** Parameters **/
    const int num_boards = 1;
    const int num_chips = 1;
    const int num_propagators = 4;
    
    const int num_cores = 8;

    /** Components **/
    std::vector<Component*> neurochips;
    for (int i=0; i<num_chips; i++)
        neurochips.push_back (new NeuroChip ("chip" + to_string(i), this, num_cores, num_propagators));
    
    std::vector<Component*> propagators;
    for (int i=0; i<num_propagators; i++)
        propagators.push_back(new Propagator ("propagator" + to_string(i), this));

    //Controller *controller = new Controller ("controller", this, num_board);

    /** Modules **/

    /** Module & Wires **/
    // create pathways
    Pathway::ConnectionAttr conattr (0, 32);
    
    // Modules
    Module *ds_parity = new DataSourceModule ("ds_parity", this);
    
    Module *ds_board = new DataSinkModule <AxonMessage, uint32_t> ("ds_board", this);
    Module *ds_idle = new DataSinkModule <SignalMessage, bool> ("ds_idle", this);

    // Wires
    std::vector<RRFaninWire*> axon_data;
    for (int i=0; i<num_propagators; i++)
        axon_data.push_back (new RRFaninWire (this, conattr, Prototype<AxonMessage>::Get(), num_chips));
    FanoutWire *cur_TSParity = new FanoutWire (this, conattr, Prototype<SignalMessage>::Get(), 
            num_cores + num_propagators);
    
    std::vector<Wire*> syn_data;
    std::vector<Wire*> syn_parity;
    std::vector<Wire*> syn_cidx;
    std::vector<Wire*> board_axon;
    std::vector<Wire*> prop_idle;
    for (int i=0; i<num_propagators; i++)
    {
        syn_data.push_back (new Wire (this, conattr, Prototype<SynapseMessage>::Get()));
        syn_parity.push_back (new Wire (this, conattr, Prototype<SignalMessage>::Get()));
        syn_cidx.push_back (new Wire (this, conattr, Prototype<SelectMessage>::Get()));
        board_axon.push_back (new Wire (this, conattr, Prototype<AxonMessage>::Get()));
        prop_idle.push_back (new Wire (this, conattr, Prototype<SignalMessage>::Get()));
    }
   

    /** Connect **/
    for (int i=0; i<num_chips; i++)
    {
        for (int p=0; p<num_propagators; p++)
            neurochips[i]->Connect ("Axon", axon_data[p]->GetEndpoint (Endpoint::LHS, i));

        for (int c=0; c<num_cores; c++)
        {
            neurochips[i]->Connect ("CurTSParity" + to_string(c), 
                    cur_TSParity->GetEndpoint (Endpoint::RHS, c));
        }
    }

    ds_parity->Connect ("dataout", cur_TSParity->GetEndpoint (Endpoint::LHS));
    
    for (int i=0; i<num_propagators; i++)
    {
        propagators[i]->Connect ("Axon", axon_data[i]->GetEndpoint (Endpoint::RHS));
        propagators[i]->Connect ("PropTS", cur_TSParity->GetEndpoint (Endpoint::RHS, num_cores + i));
        
        propagators[i]->Connect ("Synapse", syn_data[i]->GetEndpoint (Endpoint::LHS));
        propagators[i]->Connect ("SynTS", syn_parity[i]->GetEndpoint (Endpoint::LHS));
        propagators[i]->Connect ("Index", syn_cidx[i]->GetEndpoint (Endpoint::LHS));
        
        propagators[i]->Connect ("BoardAxon", board_axon[i]->GetEndpoint (Endpoint::LHS));
        propagators[i]->Connect ("Idle", prop_idle[i]->GetEndpoint (Endpoint::LHS));
        ds_board->Connect ("datain", board_axon[i]->GetEndpoint (Endpoint::RHS));
        ds_idle->Connect ("datain", prop_idle[i]->GetEndpoint (Endpoint::RHS));

        for (int c=0; c<num_chips; c++)
        {
            neurochips[c]->Connect ("SynapseData" + to_string(i), syn_data[i]->GetEndpoint (Endpoint::RHS));
            neurochips[c]->Connect ("SynTS" + to_string(i), syn_parity[i]->GetEndpoint (Endpoint::RHS));
            neurochips[c]->Connect ("SynCidx" + to_string(i), syn_cidx[i]->GetEndpoint (Endpoint::RHS));

        }
    }
}



