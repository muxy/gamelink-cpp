#include "gamelink.h"
#include "gamelink_c.h"

using namespace gamelink;

MGL_RequestId MuxyGameLink_CreatePoll(MuxyGameLink GameLink, const char *PollId, const char *Prompt, const char **Options, uint32_t OptionsCount)
{
	gamelink::SDK* SDK = static_cast<gamelink::SDK*>(GameLink.SDK);
	std::vector<gamelink::string> Opts(Options, Options + OptionsCount);
	return SDK->CreatePoll(PollId, Prompt, Opts);
}

MGL_RequestId MuxyGameLink_SubscribeToPoll(MuxyGameLink GameLink, const char *PollId)
{
	gamelink::SDK* SDK = static_cast<gamelink::SDK*>(GameLink.SDK);
	return SDK->SubscribeToPoll(PollId);
}

MGL_RequestId MuxyGameLink_UnsubscribeFromPoll(MuxyGameLink GameLink, const char *PollId)
{
	gamelink::SDK* SDK = static_cast<gamelink::SDK*>(GameLink.SDK);
	return SDK->UnsubscribeFromPoll(PollId);
}

MGL_RequestId MuxyGameLink_DeletePoll(MuxyGameLink GameLink, const char *PollId)
{
	gamelink::SDK* SDK = static_cast<gamelink::SDK*>(GameLink.SDK);
	return SDK->DeletePoll(PollId);
}

MGL_RequestId MuxyGameLink_GetPoll(MuxyGameLink GameLink, const char *PollId, MGL_GetPollResponseCallback Callback, void *UserData)
{
	gamelink::SDK* SDK = static_cast<gamelink::SDK*>(GameLink.SDK);
	return SDK->GetPoll(PollId, [Callback, UserData](const schema::GetPollResponse& PollResp)
	{
		MGL_Schema_GetPollResponse WPollResp;
		WPollResp.Obj = &PollResp;
		Callback(UserData, WPollResp);
	});
}

uint32_t MuxyGameLink_OnPollUpdate(MuxyGameLink GameLink, MGL_PollUpdateResponseCallback Callback, void *UserData)
{
	gamelink::SDK* SDK = static_cast<gamelink::SDK*>(GameLink.SDK);

	return SDK->OnPollUpdate([Callback, UserData](const schema::PollUpdateResponse& PollResp)
	{
		MGL_Schema_PollUpdateResponse WPollResp;
		WPollResp.Obj = &PollResp;
		Callback(UserData, WPollResp);
	});
}

void MuxyGameLink_DetachOnPollUpdate(MuxyGameLink GameLink, uint32_t Id)
{	
	gamelink::SDK* SDK = static_cast<gamelink::SDK*>(GameLink.SDK);
	SDK->DetachOnPollUpdate(Id);
}

const char* MuxyGameLink_Schema_GetPollResponse_GetPollId(MGL_Schema_GetPollResponse PResp)
{
	const schema::GetPollResponseBody* Response = static_cast<const schema::GetPollResponseBody*>(PResp.Obj);
	return Response->poll.pollId.c_str();
}

const char* MuxyGameLink_Schema_GetPollResponse_GetPrompt(MGL_Schema_GetPollResponse PResp)
{
	const schema::GetPollResponseBody* Response = static_cast<const schema::GetPollResponseBody*>(PResp.Obj);
	return Response->poll.prompt.c_str();
}

uint32_t MuxyGameLink_Schema_GetPollResponse_GetOptionCount(MGL_Schema_GetPollResponse PResp)
{
	const schema::GetPollResponseBody* Response = static_cast<const schema::GetPollResponseBody*>(PResp.Obj);
	return Response->poll.options.size();
}

const char* MuxyGameLink_Schema_GetPollResponse_GetOptionAt(MGL_Schema_GetPollResponse PResp, uint32_t Index)
{
	const schema::GetPollResponseBody* Response = static_cast<const schema::GetPollResponseBody*>(PResp.Obj);
	return Response->poll.options[Index].c_str();
}

uint32_t MuxyGameLink_Schema_GetPollResponse_GetResultCount(MGL_Schema_GetPollResponse PResp)
{
	const schema::GetPollResponseBody* Response = static_cast<const schema::GetPollResponseBody*>(PResp.Obj);
	return Response->results.size();
}

int32_t MuxyGameLink_Schema_GetPollResponse_GetResultAt(MGL_Schema_GetPollResponse PResp, uint32_t Index)
{
	const schema::GetPollResponseBody* Response = static_cast<const schema::GetPollResponseBody*>(PResp.Obj);
	return Response->results[Index];	
}

double MuxyGameLink_Schema_GetPollResponse_GetMean(MGL_Schema_GetPollResponse PResp)
{
	const schema::GetPollResponseBody* Response = static_cast<const schema::GetPollResponseBody*>(PResp.Obj);
	return Response->mean;	
}

double MuxyGameLink_Schema_GetPollResponse_GetSum(MGL_Schema_GetPollResponse PResp)
{
	const schema::GetPollResponseBody* Response = static_cast<const schema::GetPollResponseBody*>(PResp.Obj);
	return Response->sum;	
}

int32_t MuxyGameLink_Schema_GetPollResponse_GetCount(MGL_Schema_GetPollResponse PResp)
{
	const schema::GetPollResponseBody* Response = static_cast<const schema::GetPollResponseBody*>(PResp.Obj);
	return Response->count;	
}


const char* MuxyGameLink_Schema_PollUpdateResponse_GetPollId(MGL_Schema_PollUpdateResponse PResp)
{
	const schema::PollUpdateBody* Response = static_cast<const schema::PollUpdateBody*>(PResp.Obj);
	return Response->poll.pollId.c_str();
}

const char* MuxyGameLink_Schema_PollUpdateResponse_GetPrompt(MGL_Schema_PollUpdateResponse PResp)
{
	const schema::PollUpdateBody* Response = static_cast<const schema::PollUpdateBody*>(PResp.Obj);
	return Response->poll.prompt.c_str();
}

uint32_t MuxyGameLink_Schema_PollUpdateResponse_GetOptionCount(MGL_Schema_PollUpdateResponse PResp)
{
	const schema::PollUpdateBody* Response = static_cast<const schema::PollUpdateBody*>(PResp.Obj);
	return Response->poll.options.size();
}

const char* MuxyGameLink_Schema_PollUpdateResponse_GetOptionAt(MGL_Schema_PollUpdateResponse PResp, uint32_t Index)
{
	const schema::PollUpdateBody* Response = static_cast<const schema::PollUpdateBody*>(PResp.Obj);
	return Response->poll.options[Index].c_str();
}

uint32_t MuxyGameLink_Schema_PollUpdateResponse_GetResultCount(MGL_Schema_PollUpdateResponse PResp)
{
	const schema::PollUpdateBody* Response = static_cast<const schema::PollUpdateBody*>(PResp.Obj);
	return Response->results.size();
}

int32_t MuxyGameLink_Schema_PollUpdateResponse_GetResultAt(MGL_Schema_PollUpdateResponse PResp, uint32_t Index)
{
	const schema::PollUpdateBody* Response = static_cast<const schema::PollUpdateBody*>(PResp.Obj);
	return Response->results[Index];	
}

double MuxyGameLink_Schema_PollUpdateResponse_GetMean(MGL_Schema_PollUpdateResponse PResp)
{
	const schema::PollUpdateBody* Response = static_cast<const schema::PollUpdateBody*>(PResp.Obj);
	return Response->mean;	
}

double MuxyGameLink_Schema_PollUpdateResponse_GetSum(MGL_Schema_PollUpdateResponse PResp)
{
	const schema::PollUpdateBody* Response = static_cast<const schema::PollUpdateBody*>(PResp.Obj);
	return Response->sum;	
}

int32_t MuxyGameLink_Schema_PollUpdateResponse_GetCount(MGL_Schema_PollUpdateResponse PResp)
{
	const schema::PollUpdateBody* Response = static_cast<const schema::PollUpdateBody*>(PResp.Obj);
	return Response->count;	
}