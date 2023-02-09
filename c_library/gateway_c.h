#pragma once

#include <stdint.h>
#include <stddef.h>

typedef uint16_t MGW_RequestID;
static const MGW_RequestID MGW_ANY_REQUEST_ID      = UINT16_MAX;
static const MGW_RequestID MGW_REJECTED_REQUEST_ID = UINT16_MAX-1;

typedef struct
{
    void *SDK;
} MGW_SDK;

typedef struct
{
    const char *JWT;
    const char *RefreshToken;
    const char *TwitchName;
    bool HasError;
} MGW_AuthenticateResponse;

typedef struct
{
    const char *GameName;
    // Base64 encoded image
    const char *GameLogo;
    const char *Theme;
} MGW_GameMetadata;

typedef struct
{
    const char *Bytes;
    size_t Length;
} MGW_Payload;

typedef void (*MGW_AuthenticateResponseCallback)(void *User, const MGW_AuthenticateResponse *AuthResp);
typedef void (*MGW_PayloadCallback)(void *User, const MGW_Payload *Payload);

MGW_SDK  MGW_MakeSDK(const char* GameID);
void         MGW_KillSDK(MGW_SDK Gateway);
MGW_RequestID MGW_SDK_AuthenticateWithPIN(MGW_SDK Gateway, const char *PIN, MGW_AuthenticateResponseCallback Callback, void *User);
MGW_RequestID MGW_SDK_AuthenticateWithRefreshToken(MGW_SDK Gateway, const char *RefreshToken, MGW_AuthenticateResponseCallback Callback, void *User);
bool         MGW_SDK_IsAuthenticated(MGW_SDK Gateway);

MGW_RequestID MGW_SDK_SetGameMetadata(MGW_SDK Gateway, const MGW_GameMetadata *Meta);

bool MGW_SDK_ReceiveMessage(MGW_SDK Gateway, const char *Bytes, uint32_t Length);
bool MGW_SDK_HasPayloads(MGW_SDK Gateway);
void MGW_SDK_ForeachPayload(MGW_SDK Gateway, MGW_PayloadCallback Callback, void *User);

const char* MGW_SDK_GetSandboxURL(MGW_SDK Gateway);
const char* MGW_SDK_GetProductionURL(MGW_SDK Gateway);

// Polling.
static const int MGW_POLL_LOCATION_DEFAULT = 0;
static const int MGW_POLL_MODE_CHAOS = 0;
static const int MGW_POLL_MODE_ORDER = 1;

typedef struct {
	int Winner;
	int WinningVoteCount;

	const int* Results;
	size_t ResultCount;

	int Count;
	double Mean;
	bool IsFinal;
} MGW_PollUpdate;

typedef void (*MGW_PollUpdateCallback)(void*, MGW_PollUpdate*);

typedef struct {
	const char* Prompt;

	int Location;
	int Mode;

	const char** Options;
	size_t OptionsCount;

	int Duration;

	MGW_PollUpdateCallback OnUpdate;
	void* User;
} MGW_PollConfiguration;

void MGW_SDK_StartPoll(MGW_SDK Gateway, MGW_PollConfiguration config);
void MGW_SDK_StopPoll(MGW_SDK Gateway);


static const int MGW_ACTIONSTATE_UNAVAILABLE = 0;
static const int MGW_ACTIONSTATE_AVAILABLE = 1;
static const int MGW_ACTIONSTATE_HIDDEN = 2;

static const int MGW_ACTIONCATEGORY_NEUTRAL = 0;
static const int MGW_ACTIONCATEGORY_HINDER = 1;
static const int MGW_ACTIONCATEGORY_HELP = 2;

static const int MGW_ACTION_INFINITE_USES = -1;

typedef struct {
	const char* ID;
	int Category;
	int State;
	int Impact;

	const char* Name;
	const char* Description;
	const char* Icon;

	int Count;
} MGW_Action;

void MGW_SDK_SetActions(MGW_SDK Gateway, const MGW_Action* Begin, const MGW_Action* End);
void MGW_SDK_EnableAction(MGW_SDK Gateway, const char* id);
void MGW_SDK_DisableAction(MGW_SDK Gateway, const char* id);
void MGW_SDK_SetActionCount(MGW_SDK Gateway, const char* id, int count);

typedef struct {
	const char* Label;
	const char* Value;
	const char* Icon;
} MGW_GameText;

typedef struct {
	MGW_GameText* Texts;
	size_t TextsCount;
} MGW_GameTexts;

void MGW_SDK_SetGameTexts(MGW_SDK Gateway, const MGW_GameTexts* Texts);