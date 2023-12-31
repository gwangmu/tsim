#include <TSim/Register/SparseFileRegister.h>
#include <TSim/Register/RegisterWord.h>
#include <TSim/Utility/StaticBranchPred.h>

#include <string>
#include <cinttypes>
#include <map>

using namespace std;
using namespace TSim;

namespace NeuroSim
{
    struct RegisterWord;
    class Simulator;
}

SparseFileRegister::SparseFileRegister (const char *clsname, Type type, 
        Attr attr, RegisterWord *wproto)
    : FileRegister (clsname, type, attr, wproto) {}

const RegisterWord* SparseFileRegister::GetWord (uint64_t addr)
{
    IncrReadCount();

    if (unlikely (addr > GetAttr().addrsize))
        SIM_FATAL ("accessing out-of-bound address (addr: 0x%lu)", 
                GetName().c_str(), addr);
    if (!words.count (addr))
        return nullptr;
    else
        return words[addr];
}

bool SparseFileRegister::SetWord (uint64_t addr, RegisterWord *word)
{
    IncrWriteCount();

    if (addr == -1) return true;
    else if (unlikely (addr > GetAttr().addrsize))
    {
        SIM_FATAL ("writing out-of-bound address (addr: 0x%lu", 
                GetName().c_str(), addr);
        return false;
    }

    delete words[addr];
    words[addr] = word;

    return true;
}

bool SparseFileRegister::InitWord (uint64_t addr, RegisterWord *word)
{
    // NOTE: same as SetWord, except not increasing write count
    if (addr == -1) return true;
    else if (unlikely (addr > GetAttr().addrsize))
    {
        SIM_FATAL ("writing out-of-bound address (addr: 0x%lu", 
                GetName().c_str(), addr);
        return false;
    }

    delete words[addr];
    words[addr] = word;

    return true;
}
