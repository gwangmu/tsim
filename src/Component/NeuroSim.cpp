#include <Component/NeuroSim.h>

#include <TSim/Pathway/Wire.h>
#include <TSim/Pathway/FanoutWire.h>
#include <TSim/Pathway/RRFaninWire.h>
#include <TSim/Utility/Prototype.h>
#include <TSim/Device/AndGate.h>
#include <TSim/Pathway/IntegerMessage.h>
#include <TSim/Simulation/Testbench.h>

#include <Component/NeuroChip.h>
#include <Component/Propagator.h>
#include <Component/Controller.h>
#include <Component/InputFeeder.h>

#include <Message/SynapseMessage.h>
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
using namespace TSim;

USING_TESTBENCH;

NeuroSim::NeuroSim (string iname, Component *parent, int board_idx)
    : Component ("NeuroSim", iname, parent)
{
    // NOTE: children automatically inherit parent's clock
    //  but they can override it by redefining their own.
    SetClock ("main");

    /** Parameters **/
    const int num_boards = GET_PARAMETER (num_boards);
    const int num_chips = GET_PARAMETER (num_chips);
    const int num_propagators = GET_PARAMETER (num_propagators);
    
    const int num_cores = GET_PARAMETER (num_cores);

    const int axon_entry_queue_size = 64;
    const int decoder_queue_size = 1048576;

    /** Components **/
    std::vector<Component*> neurochips;
    for (int i=0; i<num_chips; i++)
        neurochips.push_back (new NeuroChip ("chip" + to_string(i), this, 
                                             num_cores, num_propagators, i));
    
    std::vector<Component*> propagators;
    for (int i=0; i<num_propagators; i++)
        propagators.push_back(
                new Propagator ("propagator" + to_string(i), 
                                this, board_idx, i));

    Controller *controller = new Controller ("controller", this, num_boards);

    /** Modules **/
    AndGate *idle_and = new AndGate ("idle_and", this, num_propagators);
    AndGate *dynfin_and = new AndGate ("dynfin_and", this, num_chips + 1);
    AndGate *accidle_and = new AndGate ("accidle_and", this, num_chips);

    Module *input_feeder;
    input_feeder = new InputFeeder ("input_feeder", this, num_propagators);

    /** Module & Wires **/
    // create pathways
    Pathway::ConnectionAttr conattr (0, 32);

    // Wires
    std::vector<RRFaninWire*> axon_data;
    std::vector<Wire*> bypass_data;
    for (int i=0; i<num_propagators; i++)
    {
        axon_data.push_back (new RRFaninWire (this, conattr, 
                                              Prototype<AxonMessage>::Get(), 
                                              num_chips + 3));
        bypass_data.push_back (new Wire (this, conattr, 
                                         Prototype<AxonMessage>::Get()
                                         ));
    }

    FanoutWire *cur_TSParity = new FanoutWire (this, conattr, 
                                           Prototype<IntegerMessage>::Get(), 
                                           num_chips + (2*num_propagators) + 1);
    
    std::vector<FanoutWire*> syn_data;
    std::vector<FanoutWire*> syn_parity;
    std::vector<FanoutWire*> syn_cidx;
    std::vector<Wire*> prop_idle;
    for (int i=0; i<num_propagators; i++)
    {
        syn_data.push_back (new FanoutWire (this, conattr, 
                                            Prototype<SynapseMessage>::Get(), 
                                            num_chips));
        syn_parity.push_back (new FanoutWire (this, conattr, 
                                              Prototype<SignalMessage>::Get(), 
                                              num_chips));
        syn_cidx.push_back (new FanoutWire (this, conattr, 
                                            Prototype<SelectMessage>::Get(), 
                                            num_chips));
        prop_idle.push_back (new Wire (this, conattr,
                                       Prototype<IntegerMessage>::Get()));
    }
    
    RRFaninWire *board_axon = new RRFaninWire (this, conattr, 
                                               Prototype<AxonMessage>::Get(), 
                                               num_propagators);
    RRFaninWire *board_id = new RRFaninWire (this, conattr, 
                                             Prototype<SelectMessage>::Get(), 
                                             num_propagators);
    Wire *prop_idle_and = new Wire (this, conattr, 
                                    Prototype<IntegerMessage>::Get());

    std::vector<Wire*> chip_dynfin;
    std::vector<Wire*> chip_accidle;
    for (int i=0; i<num_chips; i++)
    {
        chip_dynfin.push_back (new Wire (this, conattr, 
                                         Prototype<IntegerMessage>::Get()));
        chip_accidle.push_back (new Wire (this, conattr, 
                                         Prototype<IntegerMessage>::Get()));
    }

    Wire *dynfin = new Wire (this, conattr, Prototype<IntegerMessage>::Get()); 
    Wire *accidle = new Wire (this, conattr, Prototype<IntegerMessage>::Get()); 
    Wire *input_idle = 
            new Wire (this, conattr, Prototype<IntegerMessage>::Get()); 

    /** Connect **/
    for (int i=0; i<num_chips; i++)
    {
        for (int p=0; p<num_propagators; p++)
        {
            neurochips[i]->Connect ("Axon" + to_string(p), 
                                  axon_data[p]->GetEndpoint (Endpoint::LHS, i));
            axon_data[p]->GetEndpoint (Endpoint::LHS, i)->SetCapacity(2);
        }
        
        neurochips[i]->Connect ("CurTSParity", 
                cur_TSParity->GetEndpoint (Endpoint::RHS, i));

        neurochips[i]->Connect ("DynFin", 
                                chip_dynfin[i]->GetEndpoint (Endpoint::LHS));
        dynfin_and->Connect ("input" + to_string(i+1), 
                             chip_dynfin[i]->GetEndpoint (Endpoint::RHS));
        
        neurochips[i]->Connect ("AccIdle", 
                chip_accidle[i]->GetEndpoint (Endpoint::LHS));
        accidle_and->Connect ("input" + to_string(i), 
                             chip_accidle[i]->GetEndpoint (Endpoint::RHS));
    }

    idle_and->Connect ("output", prop_idle_and->GetEndpoint (Endpoint::LHS));
    dynfin_and->Connect ("output", dynfin->GetEndpoint (Endpoint::LHS));
    accidle_and->Connect ("output", accidle->GetEndpoint (Endpoint::LHS));

    controller->Connect ("TSParity", cur_TSParity->GetEndpoint (Endpoint::LHS));
    controller->Connect ("Idle", prop_idle_and->GetEndpoint (Endpoint::RHS));
    controller->Connect ("DynFin", dynfin->GetEndpoint (Endpoint::RHS));   
    controller->Connect ("AccIdle", accidle->GetEndpoint (Endpoint::RHS));   
    
    controller->Connect ("AxonIn", board_axon->GetEndpoint (Endpoint::RHS));
    controller->Connect ("BoardID", board_id->GetEndpoint (Endpoint::RHS));
    board_axon->GetEndpoint (Endpoint::RHS)
              ->SetCapacity (decoder_queue_size);
    board_id->GetEndpoint (Endpoint::RHS)
              ->SetCapacity (decoder_queue_size);

    // Input Feeder
    input_feeder->Connect ("ts_parity", 
                           cur_TSParity->GetEndpoint (Endpoint::RHS, 
                                                      num_chips));
    input_feeder->Connect ("idle", input_idle->GetEndpoint (Endpoint::LHS)); 
    dynfin_and->Connect ("input0", input_idle->GetEndpoint (Endpoint::RHS));

    for (int i=0; i<num_propagators; i++)
    {
        input_feeder->Connect ("axon" + to_string(i), 
                               axon_data[i]->GetEndpoint (Endpoint::LHS,
                                                          num_chips+1));

        propagators[i]->Connect ("Axon", 
                                 axon_data[i]->GetEndpoint (Endpoint::RHS));
        propagators[i]->Connect ("Bypass", 
                                 bypass_data[i]->GetEndpoint (Endpoint::RHS));
        propagators[i]->Connect ("PropTS", 
                                cur_TSParity->GetEndpoint (Endpoint::RHS, 
                                                          num_chips + (2*i+1)));
        propagators[i]->Connect ("DelayTS", 
                                cur_TSParity->GetEndpoint (Endpoint::RHS, 
                                                          num_chips + (2*i+2)));
        propagators[i]->Connect ("DelayAxon", 
                                 axon_data[i]->GetEndpoint (Endpoint::LHS, 
                                                           num_chips));
        axon_data[i]
            ->GetEndpoint (Endpoint::LHS, num_chips)
            ->SetCapacity (4);
        axon_data[i]
            ->GetEndpoint (Endpoint::LHS, num_chips + 1)
            ->SetCapacity (2);


        propagators[i]->Connect ("Synapse", 
                                 syn_data[i]->GetEndpoint (Endpoint::LHS));
        propagators[i]->Connect ("SynTS", 
                                 syn_parity[i]->GetEndpoint (Endpoint::LHS));
        propagators[i]->Connect ("Index", 
                                 syn_cidx[i]->GetEndpoint (Endpoint::LHS));
        
        propagators[i]->Connect ("BoardAxon", 
                                 board_axon->GetEndpoint (Endpoint::LHS, i));
        propagators[i]->Connect ("BoardID", 
                                 board_id->GetEndpoint (Endpoint::LHS, i));
        board_axon->GetEndpoint (Endpoint::LHS, i)
                  ->SetCapacity (axon_entry_queue_size);
        board_id->GetEndpoint (Endpoint::LHS, i)    
                  ->SetCapacity (axon_entry_queue_size);
        
        propagators[i]->Connect 
                ("Idle", prop_idle[i]->GetEndpoint (Endpoint::LHS));
    
        syn_data[i]->GetEndpoint (Endpoint::LHS)
                   ->SetCapacity (axon_entry_queue_size);
        syn_parity[i]->GetEndpoint (Endpoint::LHS)
                     ->SetCapacity (axon_entry_queue_size);
        syn_cidx[i]->GetEndpoint (Endpoint::LHS)
                   ->SetCapacity (axon_entry_queue_size);

        idle_and->Connect ("input" + to_string(i), 
                           prop_idle[i]->GetEndpoint (Endpoint::RHS)); 
        controller->Connect ("AxonOut" + to_string(i), 
                            axon_data[i]->GetEndpoint 
                                            (Endpoint::LHS, num_chips + 2));
        controller->Connect ("Bypass" + to_string(i), 
                            bypass_data[i]->GetEndpoint 
                                            (Endpoint::LHS));
        axon_data[i]
            ->GetEndpoint (Endpoint::LHS, num_chips + 2)
            ->SetCapacity (decoder_queue_size);
        bypass_data[i]
            ->GetEndpoint (Endpoint::RHS)
            ->SetCapacity (decoder_queue_size);

        for (int c=0; c<num_chips; c++)
        {
            neurochips[c]->Connect (
                    "SynapseData" + to_string(i), 
                    syn_data[i]->GetEndpoint (Endpoint::RHS, c));
            neurochips[c]->Connect (
                    "SynTS" + to_string(i), 
                    syn_parity[i]->GetEndpoint (Endpoint::RHS, c));
            neurochips[c]->Connect 
                    ("SynCidx" + to_string(i), 
                    syn_cidx[i]->GetEndpoint (Endpoint::RHS, c));

            syn_data[i]->GetEndpoint(Endpoint::RHS, c)
                       ->SetCapacity(2);
            syn_parity[i]->GetEndpoint(Endpoint::RHS, c)
                       ->SetCapacity(2);
            syn_cidx[i]->GetEndpoint(Endpoint::RHS, c)
                       ->SetCapacity(2);

        }
    }

    ExportPort ("TxExport", controller, "TxExport");
    ExportPort ("RxImport", controller, "RxImport");
}



