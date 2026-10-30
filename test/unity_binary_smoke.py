import ctypes as C
import json
import sys

class SDK(C.Structure):
    _fields_ = [('ptr', C.c_void_p)]
class Payload(C.Structure):
    _fields_ = [('bytes', C.c_void_p), ('length', C.c_size_t)]
class GameText(C.Structure):
    _fields_ = [('label', C.c_char_p), ('value', C.c_char_p), ('icon', C.c_char_p)]
class Action(C.Structure):
    _fields_ = [('id', C.c_char_p), ('category', C.c_int32), ('state', C.c_int32), ('impact', C.c_int32), ('name', C.c_char_p), ('description', C.c_char_p), ('icon', C.c_char_p), ('count', C.c_int32)]

lib = C.CDLL(sys.argv[1])
callback_type = C.CFUNCTYPE(None, C.c_void_p, C.POINTER(Payload))
lib.MGW_MakeSDK.argtypes = [C.c_char_p]
lib.MGW_MakeSDK.restype = SDK
lib.MGW_KillSDK.argtypes = [SDK]
lib.MGW_SDK_SetActions.argtypes = [SDK, C.POINTER(Action), C.c_uint64]
lib.MGW_SDK_SetGameTexts.argtypes = [SDK, C.POINTER(GameText), C.c_uint64]
lib.MGW_SDK_HasPayloads.argtypes = [SDK]
lib.MGW_SDK_HasPayloads.restype = C.c_uint32
lib.MGW_SDK_ForeachPayload.argtypes = [SDK, callback_type, C.c_void_p]

sdk = lib.MGW_MakeSDK(b'game-test')
def drain():
    result = []
    @callback_type
    def collect(_, ptr):
        p = ptr.contents
        result.append(json.loads(C.string_at(p.bytes, p.length)))
    lib.MGW_SDK_ForeachPayload(sdk, collect, None)
    return result
try:
    text = GameText(b'Health', b'50', b'fa-solid:heart')
    lib.MGW_SDK_SetGameTexts(sdk, C.byref(text), 1)
    msg = drain()
    assert len(msg) == 1 and msg[0]['data']['state'][0]['value'][0]['label'] == 'Health', msg
    lib.MGW_SDK_SetGameTexts(sdk, None, 0)
    msg = drain()
    assert len(msg) == 1 and msg[0]['data']['state'][0]['value'] == [], msg
    lib.MGW_SDK_SetGameTexts(sdk, None, 1)
    assert not lib.MGW_SDK_HasPayloads(sdk)
    action = Action(b'healthpack', 2, 1, 1, b'Health', b'description', b'fa-solid:heart', 1)
    lib.MGW_SDK_SetActions(sdk, C.byref(action), 1)
    msg = drain()
    assert len(msg) == 1 and msg[0]['data']['actions'][0]['id'] == 'healthpack', msg
    lib.MGW_SDK_SetActions(sdk, None, 0)
    msg = drain()
    assert len(msg) == 1 and msg[0]['data']['actions'] == [], msg
    lib.MGW_SDK_SetActions(sdk, None, 1)
    assert not lib.MGW_SDK_HasPayloads(sdk)
    print('Shared library: nonempty and empty game texts and actions passed')
finally:
    lib.MGW_KillSDK(sdk)
