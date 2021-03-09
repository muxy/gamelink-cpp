#include "schema/envelope.h"

namespace gamelink
{
    namespace schema
    {
        ReceiveMeta::ReceiveMeta()
            :request_id(0)
            ,timestamp(0)
        {}
        SendParameters::SendParameters()
            :request_id(0xFFFF)
        {}
    }
}