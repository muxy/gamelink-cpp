/*
	Macros for making interop easier. Not publically accessible.
*/
#define C_CALLBACK(cb, ud, type) \
	[cb, ud](const gamelink::schema:: type& arg) { MGL_Schema_##type resp; resp.Obj = &arg; cb(UserData, resp); }
