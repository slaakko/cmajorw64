// =================================
// Copyright (c) 2018 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_RT_SCREEN_INCLUDED
#define CMAJOR_RT_SCREEN_INCLUDED
#include <cmajor/rt/RtApi.hpp>

extern "C" RT_API void RtInitScreen();
extern "C" RT_API void RtDoneScreen();
extern "C" RT_API void RtRaw();
extern "C" RT_API void RtNoRaw();
extern "C" RT_API void RtCBreak();
extern "C" RT_API void RtNoCBreak();
extern "C" RT_API void RtKeyPad();
extern "C" RT_API void RtEcho();
extern "C" RT_API void RtNoEcho();
extern "C" RT_API void RtCursSet(int visibility);
extern "C" RT_API void RtRefresh();
extern "C" RT_API void RtGetMaxYX(int* rows, int* cols);
extern "C" RT_API void RtErase();
extern "C" RT_API void RtClear();
extern "C" RT_API void RtClearToEol();
extern "C" RT_API void RtGetYX(int* row, int* col);
extern "C" RT_API void RtMove(int row, int col);
extern "C" RT_API void RtAddCh(int ch);
extern "C" RT_API void RtAddStr(const char* str);
extern "C" RT_API int RtGetCh();
extern "C" RT_API void RtGetNStr(char* str, int size);
extern "C" RT_API void RtAttrOn(int attrs);
extern "C" RT_API void RtAttrOff(int attrs);
extern "C" RT_API void RtAttrSet(int attrs);
extern "C" RT_API const char* RtGetKeyName(int key);

const int keyControlA = 0x001;
const int keyControlB = 0x002;
const int keyControlC = 0x003;
const int keyControlD = 0x004;
const int keyControlE = 0x005;
const int keyControlF = 0x006;
const int keyControlG = 0x007;
const int keyBackspace = 0x008;
const int keyTab = 0x009;
const int keyControlJ = 0x00A;
const int keyControlK = 0x00B;
const int keyControlL = 0x00C;
const int keyEnter = 0x00D;
const int keyControlN = 0x00E;
const int keyControlO = 0x00F;
const int keyControlP = 0x010;
const int keyControlQ = 0x011;
const int keyControlR = 0x012;
const int keyControlS = 0x013;
const int keyControlT = 0x014;
const int keyControlU = 0x015;
const int keyControlV = 0x016;
const int keyControlW = 0x017;
const int keyControlX = 0x018;
const int keyControlY = 0x019;
const int keyControlZ = 0x01A;
const int keyEscape = 0x01B;
const int keyFS = 0x01C;
const int keyGS = 0x01D;
const int keyRS = 0x01E;
const int keyUS = 0x01F;

const int unicodePrivateStart = 0xE000;

const int keyDown = unicodePrivateStart + 0x102;
const int keyUp = unicodePrivateStart + 0x103;
const int keyLeft = unicodePrivateStart + 0x104;
const int keyRight = unicodePrivateStart + 0x105;
const int keyHome = unicodePrivateStart + 0x106;
const int keyF0 = unicodePrivateStart + 0x108;
const int keyF1 = unicodePrivateStart + 0x109;
const int keyF2 = unicodePrivateStart + 0x10a;
const int keyF3 = unicodePrivateStart + 0x10b;
const int keyF4 = unicodePrivateStart + 0x10c;
const int keyF5 = unicodePrivateStart + 0x10d;
const int keyF6 = unicodePrivateStart + 0x10e;
const int keyF7 = unicodePrivateStart + 0x10f;
const int keyF8 = unicodePrivateStart + 0x110;
const int keyF9 = unicodePrivateStart + 0x111;
const int keyF10 = unicodePrivateStart + 0x112;
const int keyF11 = unicodePrivateStart + 0x113;
const int keyF12 = unicodePrivateStart + 0x114;
const int keyDel = unicodePrivateStart + 0x14a;
const int keyIns = unicodePrivateStart + 0x14b;
const int keyPgDown = unicodePrivateStart + 0x152;
const int keyPgUp = unicodePrivateStart + 0x153;
const int keyPrint = unicodePrivateStart + 0x15a;
const int keyCancel = unicodePrivateStart + 0x161;
const int keyEnd = unicodePrivateStart + 0x166;
const int keyShiftDel = unicodePrivateStart + 0x17d;
const int keyShiftEnd = unicodePrivateStart + 0x180;
const int keyShiftHome = unicodePrivateStart + 0x184;
const int keyShiftLeft = unicodePrivateStart + 0x187;
const int keyShiftRight = unicodePrivateStart + 0x190;
const int keyResize = unicodePrivateStart + 0x222;
const int keyShiftUp = unicodePrivateStart + 0x223;
const int keyShiftDown = unicodePrivateStart + 0x224;

// PC only:

const int keyControlUp = unicodePrivateStart + 0x1e0;
const int keyControlDown = unicodePrivateStart + 0x1e1;
const int keyControlLeft = unicodePrivateStart + 0x1bb;
const int keyControlRight = unicodePrivateStart + 0x1bc;
const int keyControlPgUp = unicodePrivateStart + 0x1bd;
const int keyControlPgDown = unicodePrivateStart + 0x1be;
const int keyControlHome = unicodePrivateStart + 0x1bf;
const int keyControlEnd = unicodePrivateStart + 0x1c0;

namespace cmajor { namespace rt {

void InitScreen();
void DoneScreen();

} } // namespace cmajor::rt

#endif // CMAJOR_RT_SCREEN_INCLUDED
