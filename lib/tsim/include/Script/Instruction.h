#pragma once

#include <Base/Metadata.h>
#include <string>

using namespace std;

struct Instruction: public Metadata
{
public:
    Instruction (const char *clsname): Metadata (clsname, "") {}
};
