// =================================
// Copyright (c) 2018 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/rt/Screen.hpp>
#include <memory>
#include <unordered_map>
#include <fstream>
#ifdef _WIN32
#include <cmajor/system/ext/pdcurs36/curses.h>
#else
#include <ncurses.h>
// extension on Windows Subsystem for Linux:
#define KEY_SUP 337
#define KEY_SDOWN 336
#define CTL_LEFT 545
#define CTL_RIGHT 560
#define CTL_UP 566
#define CTL_DOWN 525
#define CTL_PGUP 555
#define CTL_PGDN 550
#define CTL_HOME 535
#define CTL_END 530
#endif

namespace cmajor { namespace rt {

std::unordered_map<int, int> keyMap;

void InitScreen()
{
    keyMap['\r'] = keyEnter;
    keyMap['\n'] = keyEnter;
    keyMap[KEY_DOWN] = keyDown;
    keyMap[KEY_UP] = keyUp;
    keyMap[KEY_LEFT] = keyLeft;
    keyMap[KEY_RIGHT] = keyRight;
    keyMap[KEY_HOME] = keyHome;
    keyMap[KEY_F(0)] = keyF0;
    keyMap[KEY_F(1)] = keyF1;
    keyMap[KEY_F(2)] = keyF2;
    keyMap[KEY_F(3)] = keyF3;
    keyMap[KEY_F(4)] = keyF4;
    keyMap[KEY_F(5)] = keyF5;
    keyMap[KEY_F(6)] = keyF6;
    keyMap[KEY_F(7)] = keyF7;
    keyMap[KEY_F(8)] = keyF8;
    keyMap[KEY_F(9)] = keyF9;
    keyMap[KEY_F(10)] = keyF10;
    keyMap[KEY_F(11)] = keyF11;
    keyMap[KEY_F(12)] = keyF12;
    keyMap[KEY_DC] = keyDel;
    keyMap[KEY_IC] = keyIns;
    keyMap[KEY_NPAGE] = keyPgDown;
    keyMap[KEY_PPAGE] = keyPgUp;
    keyMap[KEY_PRINT] = keyPrint;
    keyMap[KEY_END] = keyEnd;
    keyMap[KEY_SDC] = keyShiftDel;
    keyMap[KEY_SEND] = keyShiftEnd;
    keyMap[KEY_SHOME] = keyShiftHome;
    keyMap[KEY_SLEFT] = keyShiftLeft;
    keyMap[KEY_SRIGHT] = keyShiftRight;
    keyMap[KEY_RESIZE] = keyResize;
    keyMap[KEY_SUP] = keyShiftUp;
    keyMap[KEY_SDOWN] = keyShiftDown;
    keyMap[CTL_UP] = keyControlUp;
    keyMap[CTL_DOWN] = keyControlDown;
    keyMap[CTL_LEFT] = keyControlLeft;
    keyMap[CTL_RIGHT] = keyControlRight;
    keyMap[CTL_PGUP] = keyControlPgUp;
    keyMap[CTL_PGDN] = keyControlPgDown;
    keyMap[CTL_HOME] = keyControlHome;
    keyMap[CTL_END] = keyControlEnd;
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
    using cmajor::rt::keyMap;
    int ch = getch();
    auto it = keyMap.find(ch);
    if (it != keyMap.cend())
    {
        ch = it->second;
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
