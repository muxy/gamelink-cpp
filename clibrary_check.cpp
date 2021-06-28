#include "c_library/gamelink_c.h"

int main(int argc, char* argv[])
{
    MuxyGameLink sdk = MuxyGameLink_Make();
    bool isAuthenticated = MuxyGameLink_IsAuthenticated(sdk);
    if (isAuthenticated)
    {
        return 0;
    }

    return 1;
}