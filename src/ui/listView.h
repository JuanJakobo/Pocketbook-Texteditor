//------------------------------------------------------------------
// listView.h
//
// Author:           JuanJakobo
// Date:             04.08.2020
// Description:      An UI class to display items in a listview
//-------------------------------------------------------------------

#ifndef LISTVIEW
#define LISTVIEW

#include "inkview.h"
#include "listViewEntry.h"
#include "model.h"
#include "view.h"

#include <vector>
#include <memory>

class ListView : public View
{
public:
    /**
        * Displays a list view
        *
        * @param ContentRect area of the screen where the list view is placed
        * @param Items items that shall be shown in the listview
        */
    ListView(const irect &contentRect, int shownPage);

    virtual ~ListView();

    /**
        * Draws an single entry to the screen
        */
    void reDrawCurrentEntry();

    /**
        * inverts the color of the currently selected entry
        */
    void invertCurrentEntryColor();

    /**
        * Checkes if the listview has been clicked and either changes the page or returns item ID
        *
        * @param x x-coordinate
        * @param y y-coordinate
        * @return true if was clicked
        */
    bool checkIfEntryClicked(int x, int y) override;

    int getCurrentEntryItertator() const {return _selectedEntry;};

    /**
        * Clears the screen and draws entries and footer
        *
        */
    void draw() override;


protected:
    int _entryFontHeight;
    ifont *_entryFont;
    ifont *_entryFontBold;
    std::vector<std::shared_ptr<ListViewEntry>> _entries;
    int _selectedEntry;


    /**
        * Iterates through the items and sends them to the listViewEntry Class for drawing
        */
    void drawEntries();

    /**
        * updates an entry
        *
        * @param entryID the id of the item that shall be inverted
        */
    void updateEntry(int entryID);

};
#endif
