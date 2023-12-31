#pragma once

#include <TSim/Module/Module.h>

#include <cinttypes>
#include <string>
#include <vector>
#include <list>

using namespace std;
using namespace TSim;


class AxonStreamer: public Module
{
public:
    AxonStreamer (string iname, Component *parent, uint8_t io_buf_size);
    virtual void Operation (Message **inmsgs, Message **outmsgs, Instruction *instr);

private:
    /* Port IDs */
    // Input port
    uint32_t IPORT_Axon;

    // Output port
    uint32_t OPORT_Addr, OPORT_idle; 

    // DRAM parameters
    uint32_t read_bytes;

    // Parameters
    uint8_t num_streamer_;
    std::vector<uint16_t> tag_counter_;

    // Internal state
    //uint8_t is_idle_; // bitmap
    std::vector<bool> is_idle_;
    uint8_t ongoing_task_;

    std::list<uint8_t> free_list_; // counter
    std::vector<uint8_t> work_list_; // counter
    uint8_t stream_out_; // counter
    struct StreamJob
    {
        StreamJob() :
            base_addr(0), ax_len(0), read_addr(0), tag(0) {}
        StreamJob(uint32_t base, uint16_t len, uint32_t read, 
                uint8_t tag, bool is_inh) :
            base_addr(base), ax_len(len), read_addr(read), 
            tag(tag), is_inh(is_inh) {}

        uint32_t base_addr;
        uint16_t ax_len;
        uint32_t read_addr;
        uint8_t tag;
        bool is_inh;
        bool off_ofs;
    };
    std::vector<StreamJob> streaming_task_;
        
    uint32_t base_addr_;
    uint16_t ax_len_;
    uint32_t read_addr_;
    uint8_t tag_;
};
