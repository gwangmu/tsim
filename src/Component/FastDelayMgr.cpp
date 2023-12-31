#include <Component/FastDelayMgr.h>

#include <Message/AxonMessage.h>
#include <Message/DelayMetaMessage.h>
#include <Message/SignalMessage.h>
#include <Message/IndexMessage.h>

#include <Register/EmptyRegister.h>
#include <Script/SpikeFileScript.h>
#include <Script/SpikeInstruction.h>

#include <TSim/Utility/Prototype.h>
#include <TSim/Utility/Logging.h>
#include <TSim/Pathway/IntegerMessage.h>

#include <random>

USING_TESTBENCH;

FastDelayMgr::FastDelayMgr (string iname, Component* parent, uint8_t board_idx)
    : Module ("FastDelayMgr", iname, parent, 1)
{
    PORT_input = CreatePort ("Input", Module::PORT_INPUT,
            Prototype<AxonMessage>::Get());
    PORT_TSparity = CreatePort ("TSParity", Module::PORT_INPUT,
            Prototype<IntegerMessage>::Get());
    PORT_output = CreatePort ("Output", Module::PORT_OUTPUT,
            Prototype<AxonMessage>::Get());
    PORT_idle = CreatePort ("Idle", Module::PORT_OUTPUT,
            Prototype<IntegerMessage>::Get());

    num_delay_ = GET_PARAMETER (num_delay);
    min_delay_ = GET_PARAMETER (min_delay);

    delayed_spks_.clear();
    //delayed_spks_.resize(num_delay_);
 
    uint16_t num_boards = GET_PARAMETER (num_boards);

    this->board_idx_ = board_idx;
    this->num_neurons_ = GET_PARAMETER (num_neurons);
    this->neurons_per_board_ = num_neurons_ / num_boards;

    uint8_t num_propagators = GET_PARAMETER (num_propagators)
    this->neurons_per_prop_ = neurons_per_board_ / num_propagators;

    this->base_addr_ = GET_PARAMETER (base_addr);

    this->avg_syns_ = GET_PARAMETER (avg_synapses);
    board_syns_ = avg_syns_ * num_delay_ * (neurons_per_prop_);
   
    SetScript (new SpikeFileScript());
    Register::Attr regattr (64, 4096);
    SetRegister (new EmptyRegister (Register::SRAM, regattr));

    state_ = IDLE;
    is_idle_ = false;
    is_start_ = false;
    fetch_fin_ = true;
    ts_parity_ = false;

    // Pseudo-random
    // table
    uint32_t n = GET_PARAMETER(num_samples);
    uint32_t pint = GET_PARAMETER(probability);
    double p = pint / double(1000000);
    
    std::default_random_engine generator;
    std::binomial_distribution<int> distribution (n, p); 
    for (int i=0; i<256; i++)
    {
        synlen_table[i] = distribution(generator);
    }

    input_n = 0;
    data_cnt = 0;
}

void FastDelayMgr::Operation (Message **inmsgs, Message **outmsgs, 
        Instruction *instr)
{
    SpikeInstruction *spk_inst =
        static_cast<SpikeInstruction*> (instr);
    if (spk_inst && num_delay_ != 0)
    {
        if(spk_inst->type == SpikeInstruction::SpikeType::INH)
            delayed_spks_.push_back (DelayedSpk(1));
        else if(spk_inst->type == SpikeInstruction::SpikeType::DELAYED)
            delayed_spks_.push_back (DelayedSpk(1));
        else      
            delayed_spks_.push_back (DelayedSpk(num_delay_));

        delayed_spks_.back().spikes.assign 
            (spk_inst->spike_idx.begin(),
             spk_inst->spike_idx.end());
        delayed_spks_.back().is_inh = 
            (spk_inst->type == SpikeInstruction::SpikeType::INH);
    
        INFO_PRINT ("Spike list length: %zu", delayed_spks_.size());
       
        if(!is_start_)
        { 
            delay_it_ = delayed_spks_.begin();
            is_start_ = true;
        }
        data_cnt += spk_inst->spike_idx.size();

        if(data_cnt > 400000)
            SIM_FATAL ("[FDM] Delay SRAM is overflowed", GetFullName().c_str());
    }

    IntegerMessage *ts_msg = 
        static_cast<IntegerMessage*> (inmsgs[PORT_TSparity]);
    if(ts_msg)
    {
        if(ts_msg->value != ts_parity_)
        {
            INFO_PRINT ("[FDM] Get TS parity");
            fetch_fin_ = false;
            spk_idx_ = 0;
            delay_it_ = delayed_spks_.begin();
            ts_parity_ = ts_msg->value;
            input_n = 0;
        
            GetScript()->NextSection();
            INFO_PRINT ("[FAM] (next) Spike list length: %zu", 
                    delayed_spks_.size());

            state_ = START;
            state_counter_ = 3;

            is_start_ = false;
        }
    }

    AxonMessage *input_msg =
        static_cast<AxonMessage*> (inmsgs[PORT_input]);
    if(state_counter_ != 0)
    {
        inmsgs[PORT_input] = nullptr;
        state_counter_--;
    }    
    else if(input_msg)
    {
        // Insert
        state_ = INSERT;
        state_counter_ = 2;
        INFO_PRINT ("[FDM] Insert state %d/%d", 
                GetInQueSize (PORT_input), ++input_n);
    }
    else
    {
        // Fetch. Fast-mode support only continuous delay
        if(!fetch_fin_)
        {
            if(GetOutQueSize(PORT_output) < 2)
            {
                state_ = FETCH;
                state_counter_ = 2;            
                INFO_PRINT ("[FDM] FETCH state (fin: %d)", fetch_fin_);
            }
            else
            {
                state_ = IDLE;
                state_counter_ = 0;
            }
        }
        else if(!is_idle_)
        {
            INFO_PRINT ("[FDM] state counter %d (fin %d)", 
                    state_counter_, fetch_fin_);
            outmsgs[PORT_idle] = new IntegerMessage (1);
            is_idle_ = true;
        }
    }

    if(state_counter_ != 0 && is_idle_)
    {
        INFO_PRINT ("[FDM] state counter %d (fin %d)", 
                state_counter_, fetch_fin_);
        delete outmsgs[PORT_idle];
        outmsgs[PORT_idle] = new IntegerMessage (0);
        is_idle_ = false;
    }

    if(state_ == FETCH && (state_counter_ != 0))
    {
        if(delay_it_ == delayed_spks_.end() ||
                delayed_spks_.empty())
        {
            INFO_PRINT ("[FDM] Empty Spike list (fin %d, cnt %d)",
                    fetch_fin_, state_counter_);
            fetch_fin_ = true;
            state_counter_ = 0;
        }
        else
        {

            uint32_t out_idx = (*delay_it_).spikes[spk_idx_];
            // Send Message
            bool is_boardmsg = (out_idx / neurons_per_board_) == board_idx_;
            uint64_t ax_addr;
            uint16_t ax_len;

            uint16_t delay = (*delay_it_).delay;
            bool is_inh = (*delay_it_).is_inh;
            if(is_boardmsg) // Intra-spike
            {
                uint32_t sidx = out_idx % (neurons_per_prop_);
                
                ax_addr = avg_syns_ * num_delay_ * sidx + 
                            avg_syns_ * (num_delay_ - delay);
                ax_len = synlen_table[(sidx + delay)%256];
                int tgt = (num_delay_ == delay)? 1 : -1;
                
                if(is_inh)
                {
                    tgt = 1;
                    ax_len *= num_delay_;
                }
            
                outmsgs[PORT_output] = 
                    new AxonMessage (0, ax_addr, ax_len, 0, tgt, is_inh);
            
                INFO_PRINT ("[FDM] (%s) Fetch spike %d/%zu (addr: %lu, len %u), delay %d tgt %d",
                        GetFullNameWOClass().c_str(),
                        spk_idx_, (*delay_it_).spikes.size(),
                        ax_addr, ax_len, delay, tgt);
            }
            else // Inter-spike
            {
                uint32_t bidx = out_idx / neurons_per_board_;
                bidx = (bidx > board_idx_)? bidx-1 : bidx;

                uint32_t sidx = out_idx % (neurons_per_prop_);
               
                ax_addr = base_addr_ + bidx * board_syns_;
                ax_addr += avg_syns_ * num_delay_ * sidx + 
                            avg_syns_ * (num_delay_ - delay);
                ax_len = synlen_table[(sidx + delay)%256];

                if(num_delay_ != delay && !is_inh)
                    outmsgs[PORT_output] = 
                        new AxonMessage (0, ax_addr, ax_len);
            }

            INFO_PRINT ("[FDM] (%s) Fetch spike %d/%zu (addr: %lu, len %u)",
                    GetFullNameWOClass().c_str(),
                    spk_idx_, (*delay_it_).spikes.size(),
                    ax_addr, ax_len);
            GetRegister()->GetWord(0);

            // Advance 
            spk_idx_ += 1;
            if(spk_idx_ == (*delay_it_).spikes.size())
            {
                (*delay_it_).delay--;
                if(!(*delay_it_).delay)
                {
                    data_cnt -= (*delay_it_).spikes.size();
                    (*delay_it_).spikes.clear();
                    delay_it_ = delayed_spks_.erase (delay_it_);
                }
                else
                    delay_it_ ++;

                spk_idx_ = 0;
            }
        }
    }
}
