// Simulate including a header with gamelink_single.hpp before 
// trying to define the implementation.
#include "gamelink_single.hpp"

#define MUXY_GAMELINK_SINGLE_IMPL
#include "gamelink_single.hpp"

int main()
{
    gamelink::SDK sdk;
    bool isAuthenticated = sdk.IsAuthenticated();
    if (!isAuthenticated)
    {
        return 0;
    }

	gateway::SDK gw("Some Game");

    return 1;
}