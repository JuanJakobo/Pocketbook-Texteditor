//------------------------------------------------------------------
// textView.h
//
// Author:           JuanJakobo
// Date:             21.09.2021
// Description:      An UI class to display text items
//-------------------------------------------------------------------

#ifndef TEXTVIEW
#define TEXTVIEW

#include "view.h"
#include "inkview.h"
#include "deviceModel.h"

#include <string>
#include <map>

class TextView final : public View
{
public:

    /**
        * Displays a text view
        *
        * @param ContentRect area of the screen where the list view is placed
        * @param Items items that shall be shown in the listview
        */
    TextView(const irect &contentRect, int shownPage, Device device, const std::string &filePath);

    ~TextView();

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
    int _lineCount;
    int _currentX;
    int _currentY;
    int _cursorThickness = 4;
    int _cursorPositionStr;
    std::string _currentText;
    std::map<int,int> _lineWidth;
    std::map<int,char> _keyBindings;
    std::map<int,char> _keyBindingsShift;
    std::map<int,char> _keyBindingsAltGr;
    int _textNextLineY;
    int _textBeginX;
    int _textEndX;
    int _textBeginY;
    int _textEndY;
    std::string _filePath;

    int _textHeight;
    ifont *_textFont;

    int drawChar(const char &c);
    void loadKeyMaps();
    void handleKeyEvents(int eventID, const std::string &path);
    void addPage();
    void removePage();

};
#endif
