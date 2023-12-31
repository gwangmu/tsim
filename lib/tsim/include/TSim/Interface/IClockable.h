#pragma once

#include <TSim/Utility/AccessKey.h>

namespace TSim
{
    class IClockable
    {
    public:
        typedef void (IClockable::*ClockFunction) (PERMIT(Simulator));
    
        virtual void PreClock (PERMIT(Simulator)) = 0;
        virtual void PostClock (PERMIT(Simulator)) = 0;
    };
}
