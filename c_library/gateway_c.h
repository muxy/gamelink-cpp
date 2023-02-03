#pragma once

#include <stdint.h>
#include <stddef.h>

typedef uint16_t GW_RequestID;
static const GW_RequestID GW_ANY_REQUEST_ID      = UINT16_MAX;
static const GW_RequestID GW_REJECTED_REQUEST_ID = UINT16_MAX-1;

typedef struct
{
    void *SDK;
} MuxyGateway;

typedef struct
{
    const char *JWT;
    const char *RefreshToken;
    const char *TwitchName;
    bool HasError;
} GW_AuthenticateResponse;

typedef struct
{
    const char *GameName;
    // Base64 encoded image
    const char *GameLogo;
    const char *Theme;
} GW_GameMetadata;

typedef struct
{
    const char *Bytes;
    size_t Length;
} GW_Payload;

typedef void (*GW_AuthenticateResponseCallback)(void *User, const GW_AuthenticateResponse *AuthResp);
typedef void (*GW_PayloadCallback)(void *User, const GW_Payload *Payload);

MuxyGateway  MuxyGateway_Make(const char* GameID);
void         MuxyGateway_Kill(MuxyGateway Gateway);
GW_RequestID MuxyGateway_AuthenticateWithPIN(MuxyGateway Gateway, const char *PIN, GW_AuthenticateResponseCallback Callback, void *User);
GW_RequestID MuxyGateway_AuthenticateWithRefreshToken(MuxyGateway Gateway, const char *RefreshToken, GW_AuthenticateResponseCallback Callback, void *User);
bool         MuxyGateway_IsAuthenticated(MuxyGateway Gateway);

GW_RequestID MuxyGateway_SetGameMetadata(MuxyGateway Gateway, const GW_GameMetadata *Meta);

bool MuxyGateway_ReceiveMessage(MuxyGateway Gateway, const char *Bytes, uint32_t Length);
bool MuxyGateway_HasPayloads(MuxyGateway Gateway);
void MuxyGateway_ForeachPayload(MuxyGateway Gateway, GW_PayloadCallback Callback, void *User);

const char* MuxyGateway_GetSandboxURL(MuxyGateway Gateway);
const char* MuxyGateway_GetProductionURL(MuxyGateway Gateway);
