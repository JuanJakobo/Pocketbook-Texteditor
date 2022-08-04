//------------------------------------------------------------------
// textView.h
//
// Author:           JuanJakobo
// Date:             21.09.2021
// Description:      An UI class to display text items
//-------------------------------------------------------------------

#ifndef TEXTVIEW
#define TEXTVIEW

#include "inkview.h"
#include "deviceModel.h"

#include <string>
#include <map>

class TextView //: public View
{
public:

    /**
        * Displays a text view 
        * 
        * @param ContentRect area of the screen where the list view is placed
        * @param Items items that shall be shown in the listview
        */
    TextView(const irect &contentRect, int page, Device device, const std::string &filePath);

    ~TextView();

    //TODO need one class above that has these called View
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
    bool checkIfEntryClicked(int x, int y);
    
private:
    int _lineCount;
    int _currentX;
    int _currentY;
    int _cursorThickness;
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

    int _textHeight;
    int _footerHeight;
    int _footerFontHeight;
    const irect _contentRect;
    ifont *_footerFont;
    ifont *_textFont;
    int _page = 1;
    int _shownPage;
    irect _pageIcon;
    //irect _nextPageButton;
    //irect _prevPageButton;
    //irect _firstPageButton;
    //irect _lastPageButton;

    int drawChar(const char &c);
    void drawPageFromFile(const std::string &path);
    void loadKeyMaps();
    void handleKeyEvents(int eventID, const std::string &path);
    void addPage();
    void removePage();

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
