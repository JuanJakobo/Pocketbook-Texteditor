//------------------------------------------------------------------
// mainMenu.cpp
//
// Author:           JuanJakobo
// Date:             14.06.2020
//
//-------------------------------------------------------------------

#include "mainMenu.h"

#include <string>

using std::string;

MainMenu::MainMenu(const string &name)
{
    //hide Panel
    SetPanelType(0);
    _contentRectHorizontal = iRect(0, 0, ScreenWidth(), ScreenHeight(), 0);
    _contentRectVertical = iRect(0, 0, ScreenHeight(), ScreenWidth(), 0);
}

MainMenu::~MainMenu()
{
    free(_menu);
    free(_open);
    free(_orientation);
    free(_createFile);
    free(_exit);
}

int MainMenu::createMenu(const iv_menuhandler &handler, Views currentView)
{
    imenu mainMenu[] =
        {
            {ITEM_HEADER, 0, _menu, NULL},
            {ITEM_ACTIVE, 101, _open, NULL},
            {ITEM_ACTIVE, 102, _orientation, NULL},
            {currentView == Views::FIL ? (short)ITEM_ACTIVE : (short)ITEM_HIDDEN, 103, _createFile, NULL},
            {ITEM_ACTIVE, 104, _exit, NULL},
            {0, 0, NULL, NULL}};

    auto a = GetMenuRect(mainMenu);
    OpenMenu(mainMenu,0,ScreenWidth()/2-a.w/2,ScreenHeight()/2-a.h/2,handler);

    return 1;
}
