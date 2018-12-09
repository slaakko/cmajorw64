// =================================
// Copyright (c) 2018 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/rt/Screen.hpp>
#include <memory>
#include <unordered_map>
#ifdef _WIN32
#include <cmajor/system/ext/pdcurs36/curses.h>
#else
#include <ncurses.h>
#endif

namespace cmajor { namespace rt {

std::unordered_map<int, int> keyMap;
std::unordered_map<int, const char*> keyNameMap;

void InitScreen()
{
    keyMap[KEY_DOWN] = keyDown;
    keyMap[KEY_UP] = keyUp;
    keyMap[KEY_LEFT] = keyLeft;
    keyMap[KEY_RIGHT] = keyRight;
    keyMap[KEY_HOME] = keyHome;
    keyMap[KEY_F0] = keyF0;
    keyMap[KEY_F0 + 1] = keyF1;
    keyMap[KEY_F0 + 2] = keyF2;
    keyMap[KEY_F0 + 3] = keyF3;
    keyMap[KEY_F0 + 4] = keyF4;
    keyMap[KEY_F0 + 5] = keyF5;
    keyMap[KEY_F0 + 6] = keyF6;
    keyMap[KEY_F0 + 7] = keyF7;
    keyMap[KEY_F0 + 8] = keyF8;
    keyMap[KEY_F0 + 9] = keyF9;
    keyMap[KEY_F0 + 10] = keyF10;
    keyMap[KEY_F0 + 11] = keyF11;
    keyMap[KEY_F0 + 12] = keyF12;
    keyMap[KEY_DC] = keyDel;
    keyMap[KEY_IC] = keyIns;
    keyMap[KEY_NPAGE] = keyPgDown;
    keyMap[KEY_PPAGE] = keyPgUp;
    keyMap[KEY_PRINT] = keyPrint;
    keyMap[KEY_CANCEL] = keyCancel;
    keyMap[KEY_END] = keyEnd;
    keyMap[KEY_SDC] = keyShiftDel;
    keyMap[KEY_SEND] = keyShiftEnd;
    keyMap[KEY_SHOME] = keyShiftHome;
    keyMap[KEY_SLEFT] = keyShiftLeft;
    keyMap[KEY_SRIGHT] = keyShiftRight;
    keyMap[KEY_RESIZE] = keyResize;
#ifdef _WIN32
    keyMap[KEY_SUP] = keyShiftUp;
    keyMap[KEY_SDOWN] = keyShiftDown;
    keyMap[CTL_LEFT] = keyControlLeft;
    keyMap[CTL_RIGHT] = keyControlRight;
    keyMap[CTL_UP] = keyControlUp;
    keyMap[CTL_DOWN] = keyControlDown;
    keyMap[CTL_PGUP] = keyControlPgUp;
    keyMap[CTL_PGDN] = keyControlPgDown;
    keyMap[CTL_HOME] = keyControlHome;
    keyMap[CTL_END] = keyControlEnd;
#endif

    keyNameMap[keyDown] = "DOWN";
    keyNameMap[keyUp] = "UP";
    keyNameMap[keyLeft] = "LEFT";
    keyNameMap[keyRight] = "RIGHT";
    keyNameMap[keyHome] = "HOME";
    keyNameMap[keyF0] = "F0";
    keyNameMap[keyF1] = "F1";
    keyNameMap[keyF2] = "F2";
    keyNameMap[keyF3] = "F3";
    keyNameMap[keyF4] = "F4";
    keyNameMap[keyF5] = "F5";
    keyNameMap[keyF6] = "F6";
    keyNameMap[keyF7] = "F7";
    keyNameMap[keyF8] = "F8";
    keyNameMap[keyF9] = "F9";
    keyNameMap[keyF10] = "F10";
    keyNameMap[keyF11] = "F11";
    keyNameMap[keyF12] = "F12";
    keyNameMap[keyDel] = "DEL";
    keyNameMap[keyIns] = "INS";
    keyNameMap[keyPgDown] = "PAGE DOWN";
    keyNameMap[keyPgUp] = "PAGE UP";
    keyNameMap[keyPrint] = "PRINT";
    keyNameMap[keyCancel] = "CANCEL";
    keyNameMap[keyEnd] = "END";
    keyNameMap[keyShiftDel] = "SHIFT DEL";
    keyNameMap[keyShiftEnd] = "SHIFT END";
    keyNameMap[keyShiftHome] = "SHIFT HOME";
    keyNameMap[keyShiftLeft] = "SHIFT LEFT";
    keyNameMap[keyShiftRight] = "SHIFT RIGHT";
    keyNameMap[keyResize] = "RESIZE";
    keyNameMap[keyShiftUp] = "SHIFT UP";
    keyNameMap[keyShiftDown] = "SHIFT DOWN";
    keyNameMap[keyControlLeft] = "CONTROL LEFT";
    keyNameMap[keyControlRight] = "CONTROL RIGHT";
    keyNameMap[keyControlUp] = "CONTROL UP";
    keyNameMap[keyControlDown] = "CONTROL DOWN";
    keyNameMap[keyControlPgUp] = "CONTROL PAGE UP";
    keyNameMap[keyControlPgDown] = "CONTROL PAGE DOWN";
    keyNameMap[keyControlHome] = "CONTROL HOME";
    keyNameMap[keyControlEnd] = "CONTROL END";
}

void DoneScreen()
{
}

} } // namespace cmajor::rt

extern "C" RT_API void RtInitScreen()
{
    initscr();
}

extern "C" RT_API void RtDoneScreen()
{
    endwin();
}

extern "C" RT_API void RtRaw()
{
    raw();
}

extern "C" RT_API void RtNoRaw()
{
    noraw();
}

extern "C" RT_API void RtCBreak()
{
    cbreak();
}

extern "C" RT_API void RtNoCBreak()
{
    nocbreak();
}

extern "C" RT_API void RtKeyPad()
{
    keypad(stdscr, true);
}

extern "C" RT_API void RtEcho()
{
    echo();
}

extern "C" RT_API void RtNoEcho()
{
    noecho();
}

extern "C" RT_API void RtCursSet(int visibility)
{
    curs_set(visibility);
}

extern "C" RT_API void RtRefresh()
{
    refresh();
}

extern "C" RT_API void RtGetMaxYX(int* rows, int* cols)
{
    int r;
    int c;
    getmaxyx(stdscr, r, c);
    *rows = r;
    *cols = c;
}

extern "C" RT_API void RtErase()
{
    erase();
}

extern "C" RT_API void RtClear()
{
    clear();
}

extern "C" RT_API void RtClearToEol()
{
    clrtoeol();
}

extern "C" RT_API void RtGetYX(int* row, int* col)
{
    int r;
    int c;
    getyx(stdscr, r, c);
    *row = r;
    *col = c;
}

extern "C" RT_API void RtMove(int row, int col)
{
    move(row, col);
}

extern "C" RT_API void RtAddCh(int ch)
{
    addch(ch);
}

extern "C" RT_API void RtAddStr(const char* str)
{
    addstr(str);
}

extern "C" RT_API int RtGetCh()
{
    int ch = getch();
    auto it = cmajor::rt::keyMap.find(ch);
    if (it != cmajor::rt::keyMap.cend())
    {
        return it->second;
    }
    return ch;
}

extern "C" RT_API void RtGetNStr(char* str, int size)
{
    getnstr(str, size);
}

extern "C" RT_API void RtAttrOn(int attrs)
{
    attron(attrs);
}

extern "C" RT_API void RtAttrOff(int attrs)
{
    attroff(attrs);
}

extern "C" RT_API void RtAttrSet(int attrs)
{
    attrset(attrs);
}

extern "C" RT_API const char* RtGetKeyName(int key)
{
    auto it = cmajor::rt::keyNameMap.find(key);
    if (it != cmajor::rt::keyNameMap.cend())
    {
        return it->second;
    }
    return "UNKNOWN";
}
