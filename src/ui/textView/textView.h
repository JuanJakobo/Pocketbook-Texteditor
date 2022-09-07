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
    TextView(const irect &contentRect, Device device, const std::string &filePath, int shownPage = 1);

    ~TextView();

    /**
        * Checkes if the buttons on the bottom have been clicked, if they have been, executes function
        *
        * @param x x-coordinate
        * @param y y-coordinate
        * @return always false as changing cursor is currently not supported
        */
    bool checkIfEntryClicked(int x, int y) override;

    void draw() override;
    void drawPage() override;

private:
    int _lineCount;
    int _currentX;
    int _currentY;
    int _cursorThickness = 4;
    int _cursorPositionStr;
    std::map<int,int> _pageCharPos;
    std::map<int,int> _lastPageLineCount;
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

    //TODO documention and different methods in one

    /**
        * Draws a Char to the screen, if there is no more space, either calls addPage or jumps to next line
        *
        * @param char that shall be drawn
        * @return width of the char
        */
    int drawChar(const char &c);

    /**
        * loads a Keymap into the application to replicate keys
        *
        * @return true on sucess, false on error
        */
    bool loadKeyMaps();

    /**
        * handles the keyevents that pop up
        *
        * @param eventID of the input device
        * @param path of the file that shall be written
        */
    void handleKeyEvents(int eventID, const std::string &path);

    /**
        * adds a new, empty Page
        *
        */
    void addPage();

    /**
        * removes a page
        *
        */
    void removePage();

};
#endif
