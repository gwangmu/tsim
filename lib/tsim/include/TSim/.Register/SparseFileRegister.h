#pragma once

#include <TSim/Register/FileRegister.h>
#include <TSim/Utility/AccessKey.h>

#include <string>
#include <cinttypes>
#include <map>

using namespace std;

struct RegisterWord;
class Simulator;


class SparseFileRegister: public FileRegister
{
public:
    SparseFileRegister (const char *clsname, Type type, 
            Attr attr, RegisterWord *wproto);

    virtual RegisterWord* ParseRawString (string rawline) = 0;
    virtual const RegisterWord* GetWord (uint64_t addr) final;

private:
    map<RegisterWord *> words;
};
