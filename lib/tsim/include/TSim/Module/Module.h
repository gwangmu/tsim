#pragma once

#include <TSim/Base/Unit.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;

class Script;
class Register;
class Endpoint;
class Simulator;
class Message;
class Instruction;


class Module: public Unit 
{
public:
    /* Universal */
    Module (const char* clsname, string iname, Component *parent, uint32_t pdepth);

    inline Script* GetScript () { return script; }
    inline Register* GetRegister () { return reg; }

    /* Called by 'Simulator' */
    virtual IssueCount Validate (PERMIT(Simulator));
    virtual void PreClock (PERMIT(Simulator)) final;
    virtual void PostClock (PERMIT(Simulator)) final;

    /* Called by parent 'Component' */
    bool SetScript (Script *script); 
    bool SetRegister (Register *reg);

protected:  
    /* Called by 'Unit' */
    virtual void OnCreatePort (Port &newport);
    virtual bool IsValidConnection (Port *port, Endpoint *endpt);

    /* Called by this 'Module' */
    virtual void Operation (Message **inmsgs, Message **outmsgs, 
            const uint32_t *outque_size, Instruction *instr) = 0;

private:
    static const uint32_t MAX_PDEPTH = 64;

    // pipeline management
    inline void MarkBusyPipeline () { pbusy_state |= (1 << ((omsgidx + pdepth) & omsgidxmask)); }
    inline void CommitPipeline () { pbusy_state &= ~(1 << omsgidx); }

    inline void RefreshInMsgValidCount () { inmsg_valid_count = pdepth; }
    inline void UpdateInMsgValidCount () { if (inmsg_valid_count > 0) inmsg_valid_count--; }

    inline bool IsIdle () { return pbusy_state == 0 && inmsg_valid_count == 0; }

    Message **nextinmsgs;
    Message ***nextoutmsgs;
    uint32_t *outque_size;
    uint32_t omsgidx;
    uint32_t omsgidxmask;

    uint32_t pdepth;
    uint64_t pbusy_state;
    uint32_t inmsg_valid_count;

    // module state
    bool stalled;
    
    // add-ons
    Script *script;
    Register *reg;
};
