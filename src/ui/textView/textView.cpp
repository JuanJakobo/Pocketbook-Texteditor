//------------------------------------------------------------------
// textView.cpp
//
// Author:           JuanJakobo
// Date:             16.09.2021
//
//-------------------------------------------------------------------

#include "inkview.h"
#include "textView.h"
#include "eventHandler.h"

#include "deviceModel.h"
#include "log.h"

#include <string>
#include <fstream>
#include <sstream>

#include <linux/input.h>


using std::string;

struct input_event event;

TextView::TextView(const irect *contentRect, int page, Device device, const string &filePath) : _contentRect(contentRect), _shownPage(page)
{
    auto pageHeight = 0;
    auto contentHeight = _contentRect->h - _footerHeight;

    _footerHeight = _contentRect->h / 10;
    _footerFontHeight = 0.3 * _footerHeight;
    _textHeight = ScreenHeight()/35;

    _cursorThickness = 4;

    _textNextLineY = _textHeight + 10;
    int margin = 20; //ScreenWidth()/50;
    _textBeginX = margin;
    _textEndX = ScreenWidth() - _textBeginX - margin;
    _textBeginY = contentRect->y; //+ margin;
    _textEndY = _contentRect->h+_contentRect->y -_footerHeight; // - margin;

    _footerFont = OpenFont("LiberationMono", _footerFontHeight, FONT_STD);
    _textFont = OpenFont("Roboto", _textHeight , FONT_STD);


    _pageIcon = iRect(_contentRect->w - 100, _contentRect->h + _contentRect->y - _footerHeight, 100, _footerHeight, ALIGN_CENTER);
    //_firstPageButton = iRect(_contentRect->x, _contentRect->h + _contentRect->y - _footerHeight, 130, _footerHeight, ALIGN_CENTER);
    //_prevPageButton = iRect(_contentRect->x + 150, _contentRect->h + _contentRect->y - _footerHeight, 130, _footerHeight, ALIGN_CENTER);
    //_nextPageButton = iRect(_contentRect->x + 300, _contentRect->h + _contentRect->y - _footerHeight, 130, _footerHeight, ALIGN_CENTER);
    //_lastPageButton = iRect(_contentRect->x + 450, _contentRect->h + _contentRect->y - _footerHeight, 130, _footerHeight, ALIGN_CENTER);

    FillAreaRect(_contentRect, WHITE);
    SetFont(_footerFont, BLACK);
    drawPageFromFile(filePath);
    loadKeyMaps();
    drawFooter();
    PartialUpdate(_contentRect->x, _contentRect->y, _contentRect->w, _contentRect->h);
    handleKeyEvents(device.eventID,filePath);

}

TextView::~TextView()
{
    CloseFont(_textFont);
    CloseFont(_footerFont);
}

int TextView::drawChar(const char &c)
{
    int charWidth = CharWidth(c);

    //in own function
    if(_currentX + charWidth > _textEndX){
        FillArea(_currentX,_currentY, _cursorThickness, _textHeight, WHITE);
        PartialUpdate(_currentX,_currentY, _cursorThickness,_textHeight);
        _lineCount++;
        _lineWidth.insert(std::pair<int,int>(_lineCount, _currentX));
        _currentX = _textBeginX;
        _currentY += _textNextLineY;
    }

    if(_currentY >= _textEndY)
    {
        addPage();
        FillArea(_currentX,_currentY, _cursorThickness, _textHeight, WHITE);
        PartialUpdate(_currentX,_currentY, _cursorThickness,_textHeight);
    }

    _cursorPositionStr = _currentText.size();
    SetFont(_textFont, BLACK);
    DrawTextRect(_currentX,_currentY,charWidth,_textHeight,&c,ALIGN_CENTER);
    _currentX += charWidth;

    return charWidth;

}

void TextView::drawPageFromFile(const string &path)
{

    _currentX = _textBeginX;
    _currentY = _textBeginY;
    //TODO rename
    _lineCount = 0;
    _currentText.clear();

    SetFont(_textFont, BLACK);
    if (iv_access(path.c_str(), W_OK) == 0){
        std::ifstream inFile(path);
        string line;
        bool multipleLines = false;

        if(inFile.is_open()){
            while (getline(inFile,line)){
                if(multipleLines){
                    _lineCount++;
                    _lineWidth.insert(std::pair<int,int>(_lineCount, _currentX));
                    _currentX = _textBeginX;
                    _currentY += _textNextLineY;
                    _currentText += "\n";
                }

                for(const char &c : line)
                {
                    multipleLines = true;
                    _currentText += c;
                    // TODO does not have to draw all pages
                    drawChar(c);
                }

            }
            inFile.close();
        }
        FillArea(_currentX,_currentY, _cursorThickness,_textHeight, BLACK);
    }
    else
    {
        Message(ICON_ERROR,"Error","File not found.",1000);
    }
}

void TextView::loadKeyMaps()
{
    string keyBindingsPath = CONFIG_FOLDER + "/map.keys";

    if (iv_access(keyBindingsPath.c_str(), W_OK) == 0){
        std::ifstream inFile(keyBindingsPath);

        if(inFile.is_open()){
            std::string line;
            auto shift = false;
            auto altGr = false;
            int key;
            char value;

            while(getline(inFile,line))
            {
                if(line.find("shift") != std::string::npos){
                    shift = true;
                    altGr = false;
                }else if(line.find("altgr") != std::string::npos){
                    shift = false;
                    altGr = true;
                }else{
                    std::stringstream ss(line);

                    ss >> key >> value;

                    if(shift)
                        _keyBindingsShift[key] = value;
                    else if(altGr)
                        _keyBindingsAltGr[key] = value;
                    else
                        _keyBindings[key] = value;
                }

            }
            inFile.close();
        }
    }else{
        //TODO add default keymap file
        Message(ICON_INFORMATION,"Info", "No keymap file found. Please place a key file into /system/config/texteditor.",2000);
    }
}

void TextView::handleKeyEvents(int eventID, const string &path)
{
    std::ifstream eventFile("/dev/input/event" + std::to_string(eventID), std::ifstream::in);

    if(_currentText.empty()){
        _currentX = _textBeginX;
        _currentY = _textBeginY;
        _lineCount = 0;
        _cursorPositionStr = 0;
    }

    if(eventFile.is_open()) {

        auto shiftPressed = false;
        auto altGrPressed = false;

        char data[sizeof(event)];
        bool inputSession = true;

        Message(ICON_INFORMATION, "Information", "To cancel input mode press ESC.", 2000);

        //TODO make ü avialble
        unsigned char key;

        while(inputSession) {
            SetFont(_textFont, BLACK);
            eventFile.read(data,sizeof(event));
            memcpy(&event, data, sizeof(event));

            if(event.type == EV_KEY) {
                if(event.value == EV_KEY)
                {
                    if(event.code == KEY_ESC) {
                        inputSession = false;
                        if(!_currentText.empty()){
                            std::ofstream text(path);
                            text << _currentText;
                            text.close();
                            Message(ICON_INFORMATION,"Information", "Input Mode closed. File saved.",1000);
                        }else{
                            remove(path.c_str());
                            Message(ICON_INFORMATION,"Information", "Input Mode closed. File removed.",1000);
                        }
                    }else if(event.code == KEY_BACKSPACE) {
                        if(!_currentText.empty()){

                            if(_currentX <= _textBeginX){
                                if(_currentText.at(_cursorPositionStr-1) == '\n'){
                                    _currentText.erase(_cursorPositionStr-1);
                                    _cursorPositionStr = _currentText.size();
                                }
                                FillArea(_currentX,_currentY, _currentX+_cursorThickness,_textHeight+5, WHITE);
                                PartialUpdate(_currentX,_currentY,_currentX+_cursorThickness,_textHeight+5);

                                _currentY -= _textNextLineY;
                                if(_currentY < _textBeginY)
                                    removePage();
                                //doopelt
                                auto it = _lineWidth.find(_lineCount);
                                if(it != _lineWidth.end()){
                                    _currentX = it->second;
                                }
                                _lineWidth.erase(_lineCount);
                                _lineCount--;
                            }

                            if(!_currentText.empty()){

                                int charWidth = CharWidth(_currentText.at(_cursorPositionStr-1));
                                _currentX -= charWidth;
                                _currentText.erase(_cursorPositionStr-1);
                                _cursorPositionStr = _currentText.size();

                                FillArea(_currentX,_currentY, charWidth+_cursorThickness,_textHeight+5, WHITE);
                                FillArea(_currentX,_currentY, _cursorThickness,_textHeight, BLACK);
                                PartialUpdate(_currentX,_currentY,charWidth+_cursorThickness,_textHeight+5);
                            }
                            else
                            {
                                FillArea(_currentX,_currentY, _cursorThickness,_textHeight, BLACK);
                                PartialUpdate(_currentX,_currentY,_cursorThickness,_textHeight+5);
                            }
                        }else{
                            Message(ICON_INFORMATION, "Information", "no more characters to delete.", 2000);
                        }

                    }	else if(event.code == KEY_TAB){

                    }else if(event.code == KEY_ENTER) {

                        _lineCount++;
                        _lineWidth.insert(std::pair<int,int>(_lineCount, _currentX));
                        _currentText += "\n";

                        if((_currentY+_textNextLineY) >= _textEndY){
                            _currentY += _textNextLineY;
                            addPage();
                        }else{

                            FillArea(_currentX,_currentY, _cursorThickness, _textHeight, WHITE);
                            PartialUpdate(_currentX,_currentY, _cursorThickness,_textHeight);

                            _currentY += _textNextLineY;
                            _currentX = _textBeginX;

                            FillArea(_currentX,_currentY, _cursorThickness,_textHeight, BLACK);
                            PartialUpdate(_currentX,_currentY,_cursorThickness,_textHeight);
                        }

                        _cursorPositionStr = _currentText.size();

                    }else if(event.code == KEY_LEFTCTRL || event.code == KEY_RIGHTCTRL){
                        Message(1,"db","ctrl",1000);
                    }else if(event.code == KEY_LEFTALT){
                        Message(1,"db","alt",1000);
                    }else if(event.code == KEY_RIGHTALT){
                        altGrPressed = !altGrPressed;
                    }else if(event.code == KEY_LEFTSHIFT || event.code == KEY_RIGHTSHIFT) {
                        shiftPressed = !shiftPressed;
                    }else if(event.code == KEY_CAPSLOCK) {
                        shiftPressed = !shiftPressed;
                    }else if(event.code == KEY_UP) {

                    }else if(event.code == KEY_DOWN) {

                    }else if(event.code == KEY_RIGHT) {

                    }else if(event.code == KEY_LEFT) {
                    }else{
                        if(event.code == KEY_SPACE){
                            //TODO exists twice
                            key = ' ';
                            FillArea(_currentX,_currentY, _cursorThickness, _textHeight, WHITE);

                            _currentText += key;
                            int textWidth = drawChar(key);

                            FillArea(_currentX,_currentY, _cursorThickness,_textHeight, BLACK);
                            PartialUpdate(_currentX-textWidth,_currentY,textWidth+_cursorThickness,_textHeight);

                        }else{
                            std::map<int,char>::iterator it;

                            if(shiftPressed){
                                it = _keyBindingsShift.find(event.code);
                            }else if(altGrPressed){
                                it = _keyBindingsAltGr.find(event.code);
                            }else{
                                it = _keyBindings.find(event.code);
                            }
                            if(it != _keyBindings.end()){
                                key = it->second;
                                FillArea(_currentX,_currentY, _cursorThickness, _textHeight, WHITE);

                                _currentText += key;
                                int textWidth = drawChar(key);

                                FillArea(_currentX,_currentY, _cursorThickness,_textHeight, BLACK);
                                PartialUpdate(_currentX-textWidth,_currentY,textWidth+_cursorThickness,_textHeight);
                            }else{
                                Message(1,"db",std::to_string(event.code).c_str(),1000);
                            }
                        }


                    }

                }
                else
                {
                    if(event.value == 0){

                        if(event.code == KEY_LEFTSHIFT) {
                            shiftPressed = !shiftPressed;
                        }else if(event.code == KEY_RIGHTALT){
                            altGrPressed = !altGrPressed;
                        }
                    }
                }
            }
            else 
            {
                //Log::writeInfoLog("event type " + std::to_string(event.type));
            }
        }
        eventFile.close();
    }
    else
    {
        DrawTextRect(0, (ScreenHeight() / 3) * 2, ScreenWidth(), 30, strerror(errno), ALIGN_CENTER);
        PartialUpdate(_contentRect->x, _contentRect->y, _contentRect->w, _contentRect->h);
    }

}

void TextView::drawFooter()
{
    //DrawTextRect(_currentX,_currentY,200,_textHeight,"HALLO",ALIGN_CENTER);

    SetFont(_footerFont, WHITE);
    string footer = std::to_string(_shownPage) + "/" + std::to_string(_page);
    FillAreaRect(&_pageIcon, BLACK);

    DrawTextRect2(&_pageIcon, footer.c_str());
    //FillAreaRect(&_firstPageButton, BLACK);
    //DrawTextRect2(&_firstPageButton, "First");
    //FillAreaRect(&_prevPageButton, BLACK);
    //DrawTextRect2(&_prevPageButton, "Prev");
    //FillAreaRect(&_nextPageButton, BLACK);
    //DrawTextRect2(&_nextPageButton, "Next");
    //FillAreaRect(&_lastPageButton, BLACK);
    //DrawTextRect2(&_lastPageButton, "Last");
}


void TextView::removePage()
{
    _shownPage--;
    _page = _shownPage;
    FillAreaRect(_contentRect, WHITE);

    int line = _lineCount;
    _currentY = _textEndY-_textNextLineY;

    auto it = _lineWidth.find(line);
    if(it != _lineWidth.end()){
        _currentX = it->second;
        line--;
    }

    for(int i=_currentText.size()-1; i >= 0;--i)
    {
        if(_currentText.at(i) == '\n'){
            //TODO doppelt
            auto it = _lineWidth.find(line);
            if(it != _lineWidth.end()){
                _currentX = it->second;
                line--;
            }
            _currentY -= _textNextLineY;

            if(_currentY < (_textBeginY-_textNextLineY))
                break;
        }else{

            int charWidth = CharWidth(_currentText.at(i));

            if(_currentX -charWidth < _textBeginX){
                auto it = _lineWidth.find(line);
                if(it != _lineWidth.end()){
                    _currentX = it->second;
                    line--;
                }
                _currentY -= _textNextLineY;
            }

            if(_currentY < (_textBeginY-_textNextLineY))
                break;

            SetFont(_textFont, BLACK);
            _currentX -= charWidth;
            DrawTextRect(_currentX,_currentY,charWidth,_textHeight,&_currentText.at(i),ALIGN_CENTER);

        }

    }
    _currentY = _textEndY-_textNextLineY;
    //needed here?
    _cursorPositionStr = _currentText.size();

    drawFooter();
    PartialUpdate(_contentRect->x, _contentRect->y, _contentRect->w, _contentRect->h);
}

void TextView::addPage()
{
    _shownPage++;
    _page = _shownPage;
    FillAreaRect(_contentRect, WHITE);
    _currentX = _textBeginX;
    _currentY = _textBeginY;
    //needed here?
    _cursorPositionStr = _currentText.size();
    drawFooter();
    FillArea(_currentX,_currentY, _cursorThickness,_textHeight, BLACK);
    PartialUpdate(_contentRect->x, _contentRect->y, _contentRect->w, _contentRect->h);
}
