#include <Component/NeuroChip.h>

#include <TSim/Pathway/Wire.h>
#include <TSim/Pathway/FanoutWire.h>
#include <TSim/Pathway/RRFaninWire.h>

#include <TSim/Utility/Prototype.h>
#include <TSim/Device/AndGate.h>
#include <TSim/Pathway/IntegerMessage.h>

#include <Component/DataSourceModule.h>
#include <Component/DataSinkModule.h>
#include <Component/DataEndpt.h>

#include <Component/NeuroCore.h>
#include <Component/FastCore.h>
#include <Component/SlowCore.h>
#include <Component/SynDataDistrib.h>
#include <Component/AxonTransmitter.h>

#include <Message/AxonMessage.h>
#include <Message/ExampleMessage.h>
#include <Message/NeuronBlockMessage.h>
#include <Message/DeltaGMessage.h>
#include <Message/StateMessage.h>
#include <Message/SignalMessage.h>
#include <Message/IndexMessage.h>
#include <Message/NeuronBlockMessage.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;
using namespace TSim;

USING_TESTBENCH;

NeuroChip::NeuroChip (string iname, Component *parent, 
        int num_cores, int num_propagators, uint16_t chip_idx)
    : Component ("NeuroChip", iname, parent)
{
    /** Parameters **/
    int axon_meta_queue_size = 16; 
    int fast = GET_PARAMETER (fast);
    int syn_data_queue_size = 64;

    /** Components **/
    std::vector<Component*> cores;
    uint16_t core_idx = chip_idx * num_cores; 

    for (int i=0; i<num_cores; i++)
    {
        if(fast == 1)
            cores.push_back (
                    new FastCore ("core" + to_string(i) , 
                                  this, num_propagators, core_idx + i));
        else if(fast == 0)
            cores.push_back (
                    new NeuroCore ("core" + to_string(i) , 
                                   this, num_propagators));
    }
    /** Modules **/
    Module *axon_transmitter = new AxonTransmitter ("axon_transmitter", this);
    Module *syn_distributor = 
            new SynDataDistrib ("syn_distributor", this, num_propagators);
    AndGate *dynfin_and = new AndGate ("dynfin_and", this, num_cores + 1);
    AndGate *accidle_and = new AndGate ("accidle_and", this, num_cores);
    AndGate *ts_buf = new AndGate ("ts_buf", this, 1);

    /** Wires **/
    // create pathways
    Pathway::ConnectionAttr conattr (0, 32);

    // Wires
    std::vector<Wire*> core_DynFin;
    std::vector<Wire*> core_AccIdle;
    for (int i =0; i<num_cores; i++)
    {
        core_DynFin.push_back (
                new Wire (this, conattr, 
                          Prototype<IntegerMessage>::Get()));
        core_AccIdle.push_back (
                new Wire (this, conattr, 
                          Prototype<IntegerMessage>::Get()));
    }
    Wire* transmitter_idle = 
        new Wire (this, conattr, Prototype<IntegerMessage>::Get());

    std::vector<FanoutWire*> syn_data;
    std::vector<FanoutWire*> syn_parity;
    for (int i=0; i<num_propagators; i++)
    {
        syn_data.push_back (
                new FanoutWire (this, conattr, 
                                Prototype<SynapseMessage>::Get(), num_cores));
        syn_parity.push_back (
                new FanoutWire (this, conattr, 
                                Prototype<SignalMessage>::Get(), num_cores));
    }
  
    RRFaninWire *axon_data = 
        new RRFaninWire (this, conattr, 
                        Prototype<AxonMessage>::Get(), num_cores);
    FanoutWire *cur_tsparity = 
        new FanoutWire (this, conattr, 
                        Prototype<IntegerMessage>::Get(), num_cores);

    /** Connect **/
    axon_transmitter->Connect ("axon_in", 
                                axon_data->GetEndpoint (Endpoint::RHS));
    axon_transmitter->Connect ("idle", 
                                transmitter_idle->GetEndpoint (Endpoint::LHS));
    dynfin_and->Connect 
        ("input" + to_string(num_cores), 
         transmitter_idle->GetEndpoint (Endpoint::RHS));

    ts_buf->Connect ("output", cur_tsparity->GetEndpoint (Endpoint::LHS));

    for (int i=0; i<num_cores; i++)
    {
        cores[i]->Connect ("DynFin", 
                            core_DynFin[i]->GetEndpoint (Endpoint::LHS));
        dynfin_and->Connect ("input" + to_string(i), 
                            core_DynFin[i]->GetEndpoint (Endpoint::RHS));
        cores[i]->Connect ("AccIdle", 
                            core_AccIdle[i]->GetEndpoint (Endpoint::LHS));
        accidle_and->Connect ("input" + to_string(i), 
                            core_AccIdle[i]->GetEndpoint (Endpoint::RHS));

        cores[i]->Connect ("CurTSParity", 
                            cur_tsparity->GetEndpoint(Endpoint::RHS, i));

        cores[i]->Connect ("AxonData", 
                            axon_data->GetEndpoint (Endpoint::LHS, i));
        axon_data->GetEndpoint(
                Endpoint::LHS, i)->SetCapacity (axon_meta_queue_size);
            
        for (int j=0; j<num_propagators; j++)
        {
            cores[i]->Connect ("SynData" + to_string(j), 
                                syn_data[j]->GetEndpoint (Endpoint::RHS, i));
            cores[i]->Connect ("SynTS" + to_string(j), 
                                syn_parity[j]->GetEndpoint (Endpoint::RHS, i));

           syn_data[j]->GetEndpoint (Endpoint::RHS, i)->
                            SetCapacity (syn_data_queue_size);
           syn_parity[j]->GetEndpoint (Endpoint::RHS, i)->
                            SetCapacity (syn_data_queue_size);
        }
    }
    
    for (int j=0; j<num_propagators; j++)
    {
        syn_distributor->Connect ("syn_out" + to_string(j), 
                                  syn_data[j]->GetEndpoint (Endpoint::LHS));
        syn_distributor->Connect ("syn_ts_out" + to_string(j), 
                                  syn_parity[j]->GetEndpoint (Endpoint::LHS));
    }
    
    //for (int i=0; i<num_cores; i++)
    //    ExportPort ("CurTSParity" + to_string(i), cores[i], "CurTSParity");
    ExportPort ("CurTSParity", ts_buf, "input0");

    for (int i=0; i<num_propagators; i++)
    {
        ExportPort ("Axon" + to_string(i), axon_transmitter, "axon_out" + to_string(i));
        ExportPort ("SynapseData" + to_string(i), syn_distributor, "syn_in" + to_string(i));
        ExportPort ("SynCidx" + to_string(i), syn_distributor, "syn_cidx" + to_string(i));
        ExportPort ("SynTS" + to_string(i), syn_distributor, "syn_ts_in" + to_string(i));
    }

    ExportPort ("DynFin", dynfin_and, "output");
    ExportPort ("AccIdle", accidle_and, "output");
}

