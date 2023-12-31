#pragma once

#define PERMIT(x) x##AccessKey x##_access_key
#define KEY(x) (x##AccessKey ())
#define TRANSFER_KEY(x) x##_access_key

#define CREATE_KEY(x)               \
    namespace TSim              \
    {                               \
        class x;                    \
        class x##AccessKey          \
        {                           \
        public:                     \
            friend class x;         \
            x##AccessKey () {}      \
        };                          \
    }

CREATE_KEY(Simulator)
CREATE_KEY(Pathway)
CREATE_KEY(Module)
CREATE_KEY(Component)
