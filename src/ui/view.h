//------------------------------------------------------------------
// View.h
//
// Author:           JuanJakobo
// Date:             04.08.2020
// Description:      An UI class to display items in a listview
//-------------------------------------------------------------------

#ifndef VIEW
#define VIEW

#include "inkview.h"

#include <vector>
#include <memory>

enum class Views
{
    FIL,
    DEVICE,
    TEXT,
    ERROR
};

class View
{
public:
    /**
        * Displays a list view
        *
        * @param ContentRect area of the screen where the list view is placed
        * @param Items items that shall be shown in the listview
        */
    View(const irect &contentRect, int shownPage);

    virtual ~View();

    int getShownPage(){return _shownPage;};

    /**
        * Navigates to the next page
        */
    void nextPage() { this->actualizePage(_shownPage + 1); };

    /**
        * Navigates to the prev page
        */
    void prevPage() { this->actualizePage(_shownPage - 1); };

    /**
        * Navigates to first page
        */
    void firstPage() { this->actualizePage(1); };

    /**
        * Checkes if the listview has been clicked and either changes the page or returns item ID
        *
        * @param x x-coordinate
        * @param y y-coordinate
        * @return true if was clicked
        */
    virtual bool checkIfEntryClicked(int x, int y) = 0;

    /**
        * Clears the screen and draws entries and footer
        *
        */
    virtual void draw() = 0;


    Views getCurrentView(){return _currentView;};

    void setContentRect(const irect &contentRect){ _contentRect = contentRect;};


protected:
    Views _currentView;
    int _footerHeight;
    int _footerFontHeight;
    int _entryFontHeight;
    irect _contentRect;
    ifont  *_footerFont;
    int _page = 1;
    int _shownPage;
    irect _pageIcon;
    irect _nextPageButton;
    irect _prevPageButton;
    irect _firstPageButton;
    irect _lastPageButton;
    int _selectedEntry;

    /**
        * Draws the footer including a page changer
        */
    void drawFooter();

    /**
        * Navigates to the selected page
        *
        * @param pageToShow page that shall be shown
        */
    void actualizePage(int pageToShow);
};
#endif
