//------------------------------------------------------------------
// errorView.h
//
// Author:           JuanJakobo
// Date:             30.08.2022
// Description:      An UI class to display error texts
//-------------------------------------------------------------------

#ifndef ERRORVIEW
#define ERRORVIEW

#include "view.h"
#include "inkview.h"
#include "deviceModel.h"

#include <string>
#include <map>

class ErrorView final : public View
{
public:

    /**
        * Displays a text view
        *
        * @param ContentRect area of the screen where the list view is placed
        * @param Items items that shall be shown in the listview
        */
    ErrorView(const irect &contentRect, const std::string &text, int shownPage = 1);

    ~ErrorView();

    /**
        * Checkes if the listview has been clicked and either changes the page or returns item ID
        *
        * @param x x-coordinate
        * @param y y-coordinate
        * @return true if was clicked
        */
    bool checkIfEntryClicked(int x, int y) override;

    void draw() override;

private:
    int _textHeight;
    ifont *_textFont;
    std::string _text;

};
#endif
