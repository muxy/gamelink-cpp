
// Simulate including a header with gamelink_single.hpp before 
// trying to define the implementation.
#include "gamelink_single.hpp"

#define MUXY_GAMELINK_SINGLE_IMPL
#include "gamelink_single.hpp"

int main()
{
    gamelink::SDK sdk;
    return 0;
}