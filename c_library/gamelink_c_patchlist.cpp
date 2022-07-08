#include "gamelink.h"
#include "gamelink_c.h"

using namespace gamelink;

MGL_PatchList MuxyGameLink_PatchList_Make(void)
{
	MGL_PatchList PList;
	PList.Obj = new gamelink::PatchList();
	return PList;
}
void MuxyGameLink_PatchList_Kill(MGL_PatchList PList)
{
	delete static_cast<gamelink::PatchList *>(PList.Obj);
}

void MuxyGameLink_PatchList_UpdateStateWithInteger(MGL_PatchList PList, MGL_Operation Operation, const char* Path, int64_t Val)
{
	gamelink::PatchList* Patch = static_cast<gamelink::PatchList*>(PList.Obj);
	Patch->UpdateStateWithInteger(static_cast<gamelink::Operation>(Operation), Path, Val);
}

void MuxyGameLink_PatchList_UpdateStateWithDouble(MGL_PatchList PList, MGL_Operation Operation, const char* Path, double Val)
{
	gamelink::PatchList* Patch = static_cast<gamelink::PatchList*>(PList.Obj);
	Patch->UpdateStateWithDouble(static_cast<gamelink::Operation>(Operation), Path, Val);
}

void MuxyGameLink_PatchList_UpdateStateWithBoolean(MGL_PatchList PList, MGL_Operation Operation, const char* Path, bool Val)
{
	gamelink::PatchList* Patch = static_cast<gamelink::PatchList*>(PList.Obj);
	Patch->UpdateStateWithDouble(static_cast<gamelink::Operation>(Operation), Path, Val);
}

void MuxyGameLink_PatchList_UpdateStateWithString(MGL_PatchList PList, MGL_Operation Operation, const char* Path, const char* Val)
{
	gamelink::PatchList* Patch = static_cast<gamelink::PatchList*>(PList.Obj);
	Patch->UpdateStateWithString(static_cast<gamelink::Operation>(Operation), Path, Val);
}

void MuxyGameLink_PatchList_UpdateStateWithLiteral(MGL_PatchList PList, MGL_Operation Operation, const char* Path, const char* Val)
{
	gamelink::PatchList* Patch = static_cast<gamelink::PatchList*>(PList.Obj);
	Patch->UpdateStateWithLiteral(static_cast<gamelink::Operation>(Operation), Path, Val);
}

void MuxyGameLink_PatchList_UpdateStateWithNull(MGL_PatchList PList, MGL_Operation Operation, const char* Path)
{
	gamelink::PatchList* Patch = static_cast<gamelink::PatchList*>(PList.Obj);
	Patch->UpdateStateWithNull(static_cast<gamelink::Operation>(Operation), Path);
}

void MuxyGameLink_PatchList_UpdateStateWithJson(MGL_PatchList PList, MGL_Operation Operation, const char* Path, const char* Val)
{
	gamelink::PatchList* Patch = static_cast<gamelink::PatchList*>(PList.Obj);
	Patch->UpdateStateWithJson(static_cast<gamelink::Operation>(Operation), Path, Val);
}

void MuxyGameLink_PatchList_UpdateStateWithEmptyArray(MGL_PatchList PList, MGL_Operation Operation, const char* Path)
{
	gamelink::PatchList* Patch = static_cast<gamelink::PatchList*>(PList.Obj);
	Patch->UpdateStateWithEmptyArray(static_cast<gamelink::Operation>(Operation), Path);
}

bool MuxyGameLink_PatchList_Empty(MGL_PatchList PList)
{
	gamelink::PatchList* Patch = static_cast<gamelink::PatchList*>(PList.Obj);
	return Patch->Empty();
}

void MuxyGameLink_PatchList_Clear(MGL_PatchList PList)
{
	gamelink::PatchList* Patch = static_cast<gamelink::PatchList*>(PList.Obj);
	Patch->Clear();
}