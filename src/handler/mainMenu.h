//------------------------------------------------------------------
// mainMenu.h
//
// Author:           JuanJakobo
// Date:             14.06.2020
// Description:      Handles the menubar and the menu
//-------------------------------------------------------------------

#ifndef MAIN_MENU
#define MAIN_MENU

#include "inkview.h"
#include "view.h"

#include <string>

class MainMenu
{
public:
    /**
        * Defines fonds, sets global Event Handler and starts new content 
        * 
        * @param name name of the application
        */
    MainMenu(const std::string &name);

    ~MainMenu();

    irect &getContentRect() { return (GetOrientation() == 0 || GetOrientation() == 3) ? _contentRectHorizontal : _contentRectVertical; };

    /**
        * Shows the menu on the screen, lets the user choose menu options and then redirects the handler to the caller
        * 
        * @return int returns if the event was handled
        */
    int createMenu(const iv_menuhandler &handler);

private:
    imenu _mainMenu;
    irect _contentRectHorizontal;
    irect _contentRectVertical;

    char *_menu = strdup("Menu");
    char *_open = strdup("Open editor");
    char *_orientation = strdup("Change orientation");
    char *_exit = strdup("Close App");
};
#endif
