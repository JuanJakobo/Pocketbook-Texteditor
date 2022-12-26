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

TextView::TextView(const irect &contentRect, Device device, const string &filePath, int shownPage) : View(contentRect,shownPage), _filePath(filePath)
{
    draw();
    handleKeyEvents(device.eventID,filePath);
}

TextView::~TextView()
{
    CloseFont(_textFont);
    //free(_textFont);
}
bool TextView::checkIfEntryClicked(int x, int y)
{
    //TODO lock
    if (IsInRect(x, y, &_firstPageButton))
    {
        firstPage();
    }
    else if (IsInRect(x, y, &_nextPageButton))
    {
        nextPage();
    }
    else if (IsInRect(x, y, &_prevPageButton))
    {
        prevPage();
    }
    else if (IsInRect(x, y, &_lastPageButton))
    {
        actualizePage(_page);
    }

    return false;
}

void TextView::draw()
{
    FillAreaRect(&_contentRect, WHITE);

    _textHeight = (GetOrientation() == 0 || GetOrientation() == 3) ? ScreenHeight()/35 : ScreenWidth()/30;
    _textFont = OpenFont("Roboto", _textHeight , FONT_STD);
    _textNextLineY = _textHeight + 10;
    int margin = 20; //ScreenWidth()/50;
    _textBeginX = margin;
    _textEndX = _contentRect.w - _textBeginX - margin;
    _textBeginY = _contentRect.y; //+ margin;
    _textEndY = _contentRect.h -_footerHeight - _textHeight; // - margin;

    _currentX = _textBeginX;
    _currentY = _textBeginY;
    _lineCount = 0;
    _currentText.clear();

    //map containing page number and the first char?!
    //map or vector?
    _lastPageLineCount.insert(std::pair<int,int>(_page,0));
    _pageCharPos.insert(std::pair<int,int>(_page,0));

    SetFont(_textFont, BLACK);
    if (iv_access(_filePath.c_str(), W_OK) == 0){
        std::ifstream inFile(_filePath);
        string line;
        bool multipleLines = false;

        if (inFile.is_open()){
            while (getline(inFile,line)){
                if (multipleLines){
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

                    int charWidth = CharWidth(c);

                    if (_currentX + charWidth > _textEndX){
                        _lineCount++;
                        _lineWidth.insert(std::pair<int,int>(_lineCount, _currentX));
                        _currentX = _textBeginX;
                        _currentY += _textNextLineY;
                    }

                    if (_currentY >= _textEndY)
                    {
                        _shownPage++;
                        _page = _shownPage;
                        _lastPageLineCount.insert(std::pair<int,int>(_page,_lineCount));
                        _pageCharPos.insert(std::pair<int,int>(_page,_currentText.size()-1));
                        _currentX = _textBeginX;
                        _currentY = _textBeginY;
                    }
                    _currentX += charWidth;
                }

            }
            inFile.close();
        }
        drawPage();
    }
    else
    {
        Message(ICON_ERROR,"Error","File not found.",1000);
    }
}

int TextView::drawChar(const char &c)
{
    int charWidth = CharWidth(c);

    if (_currentX + charWidth > _textEndX){
        FillArea(_currentX,_currentY, _cursorThickness, _textHeight, WHITE);
        PartialUpdate(_currentX,_currentY, _cursorThickness,_textHeight);
        _lineCount++;
        _lineWidth.insert(std::pair<int,int>(_lineCount, _currentX));
        _currentX = _textBeginX;
        _currentY += _textNextLineY;
    }

    if (_currentY >= _textEndY)
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

bool TextView::loadKeyMaps()
{
    string keyBindingsPath = CONFIG_FOLDER + "/map.keys";

    if (iv_access(keyBindingsPath.c_str(), W_OK) == 0){
        std::ifstream inFile(keyBindingsPath);

        if (inFile.is_open()){
            std::string line;
            auto shift = false;
            auto altGr = false;
            int key;
            char value;

            while(getline(inFile,line))
            {
                if (line.find("shift") != std::string::npos){
                    shift = true;
                    altGr = false;
                }else if (line.find("altgr") != std::string::npos){
                    shift = false;
                    altGr = true;
                }else{
                    std::stringstream ss(line);

                    ss >> key >> value;

                    if (shift)
                        _keyBindingsShift[key] = value;
                    else if (altGr)
                        _keyBindingsAltGr[key] = value;
                    else
                        _keyBindings[key] = value;
                }

            }
            inFile.close();
        }
    }else{
        Message(ICON_INFORMATION,"Info", "No keymap file found. Please place a key file into \"/system/config/texteditor\".",2000);
        return false;
    }
    return true;
}

void TextView::handleKeyEvents(int eventID, const string &path)
{
    if(loadKeyMaps())
    {
        std::ifstream eventFile("/dev/input/event" + std::to_string(eventID), std::ifstream::in);

        if (_currentText.empty())
        {
            _currentX = _textBeginX;
            _currentY = _textBeginY;
            _lineCount = 0;
            _cursorPositionStr = 0;
        }

        if (eventFile.is_open())
        {
            auto shiftPressed = false;
            auto altGrPressed = false;

            char data[sizeof(event)];
            bool inputSession = true;

            Message(ICON_INFORMATION, "Information", "To cancel input mode press ESC.", 2000);

            //TODO make ü avialble
            unsigned char key;

            //TODO Lock for menu button
            //TODO do in thread
            while(inputSession)
            {
                key = 0;
                SetFont(_textFont, BLACK);
                eventFile.read(data,sizeof(event));
                memcpy(&event, data, sizeof(event));

                if (event.type == EV_KEY)
                {
                    if (event.value == EV_KEY)
                    {
                        switch (event.code)
                        {
                            case KEY_ESC:
                                {
                                    inputSession = false;
                                    if (!_currentText.empty()){
                                        std::ofstream text(path);
                                        text << _currentText;
                                        text.close();
                                        Message(ICON_INFORMATION,"Information", "Input Mode closed. File saved.",1000);
                                    }else{
                                        remove(path.c_str());
                                        Message(ICON_INFORMATION,"Information", "Input Mode closed. File removed.",1000);
                                    }
                                    break;
                                }
                            case KEY_BACKSPACE:
                                {
                                    if (!_currentText.empty()){

                                        if (_currentX <= _textBeginX){
                                            if (_currentText.at(_cursorPositionStr-1) == '\n'){
                                                _currentText.erase(_cursorPositionStr-1);
                                                _cursorPositionStr = _currentText.size();
                                            }
                                            FillArea(_currentX,_currentY, _currentX+_cursorThickness,_textHeight+5, WHITE);
                                            PartialUpdate(_currentX,_currentY,_currentX+_cursorThickness,_textHeight+5);

                                            _currentY -= _textNextLineY;
                                            if (_currentY < _textBeginY)
                                                removePage();
                                            //TODO twice
                                            if (auto it = _lineWidth.find(_lineCount); it != _lineWidth.end())
                                                _currentX = it->second;

                                            _lineWidth.erase(_lineCount);
                                            _lineCount--;
                                            FillArea(_currentX,_currentY, _cursorThickness,_textHeight, BLACK);
                                            PartialUpdate(_currentX,_currentY,_cursorThickness,_textHeight+5);
                                        }else{

                                            //TODO twice
                                            if (!_currentText.empty()){

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
                                        }
                                    }else{
                                        Message(ICON_INFORMATION, "Information", "No more characters to delete.", 2000);
                                    }
                                    break;
                                }
                            case KEY_TAB:
                                break;
                            case KEY_ENTER:
                                {
                                    _lineCount++;
                                    _lineWidth.insert(std::pair<int,int>(_lineCount, _currentX));
                                    _currentText += "\n";

                                    if ((_currentY+_textNextLineY) >= _textEndY){
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

                                    break;
                                }
                            case KEY_LEFTCTRL:
                            case KEY_RIGHTCTRL:
                                Message(1,"db","ctrl",1000);
                                break;
                            case KEY_LEFTALT:
                                Message(1,"db","alt",1000);
                                break;
                            case KEY_RIGHTALT:
                                altGrPressed = !altGrPressed;
                                break;
                            case KEY_LEFTSHIFT:
                            case KEY_RIGHTSHIFT:
                                shiftPressed = !shiftPressed;
                                break;
                            case KEY_CAPSLOCK:
                                shiftPressed = !shiftPressed;
                                break;
                            case KEY_UP:
                            case KEY_DOWN:
                            case KEY_RIGHT:
                            case KEY_LEFT:
                                Message(1,"db","arrow keys",1000);
                                break;
                            case KEY_SPACE:
                                key = ' ';
                                break;
                            default:
                                {
                                    std::map<int,char>::iterator it;
                                    if (shiftPressed){
                                        it = _keyBindingsShift.find(event.code);
                                    }else if (altGrPressed){
                                        it = _keyBindingsAltGr.find(event.code);
                                    }else{
                                        it = _keyBindings.find(event.code);
                                    }
                                    key = it->second;

                                    break;
                                }
                        }
                        if(key != 0)
                        {
                            //Log::writeInfoLog("eventcode " + std::to_string(event.code) + " key " + std::to_string(key));
                            FillArea(_currentX,_currentY, _cursorThickness, _textHeight, WHITE);

                            _currentText += key;
                            int textWidth = drawChar(key);

                            //TODO when fast typing hide, otherwise show cursor
                            //FillArea(_currentX,_currentY, _cursorThickness,_textHeight, BLACK);
                            //PartialUpdate(_currentX-textWidth,_currentY,textWidth+_cursorThickness,_textHeight);
                            PartialUpdate(_currentX-textWidth,_currentY,textWidth,_textHeight);
                        }else{
                            //Message(1,"db",std::to_string(event.code).c_str(),1000);
                        }
                    }
                    else if (event.value == 0)
                    {
                        switch (event.code)
                        {
                            case KEY_LEFTSHIFT:
                                shiftPressed = !shiftPressed;
                                break;
                            case KEY_RIGHTALT:
                                altGrPressed = !altGrPressed;
                                break;
                            default:
                               break;
                        }
                    }
                }
            }
            eventFile.close();
        }
        else
        {
            DrawTextRect(0, (_contentRect.h / 3) * 2, _contentRect.w, 30, strerror(errno), ALIGN_CENTER);
            PartialUpdate(_contentRect.x, _contentRect.y, _contentRect.w, _contentRect.h);
        }

    }
}

void TextView::removePage()
{
    _shownPage--;
    _page = _shownPage;
    drawPage();
    //TODO do not subistute in other method for remove?
    _lineCount++;
}

void TextView::drawPage()
{
    FillAreaRect(&_contentRect, WHITE);

    if(!_currentText.empty())
    {
        _currentX = _textBeginX;
        _currentY = _textBeginY;

        if (auto it = _lastPageLineCount.find(_shownPage); it != _lastPageLineCount.end())
            _lineCount = it->second;

        int lastPageCharPosition = 0;
        if (auto it = _pageCharPos.find(_shownPage); it != _pageCharPos.end())
            lastPageCharPosition = it->second;

        for(int i=lastPageCharPosition; i < _currentText.size();++i)
        {
            if (_currentY >= _textEndY)
                break;

            if (_currentText.at(i) == '\n'){
                _lineCount++;
                _lineWidth.insert(std::pair<int,int>(_lineCount, _currentX));
                _currentX = _textBeginX;
                _currentY += _textNextLineY;
            }else{
                int charWidth = CharWidth(_currentText.at(i));

                if (_currentX + charWidth > _textEndX){
                    _lineCount++;
                    _lineWidth.insert(std::pair<int,int>(_lineCount, _currentX));
                    _currentX = _textBeginX;
                    _currentY += _textNextLineY;
                }

                _cursorPositionStr = _currentText.size();
                SetFont(_textFont, BLACK);
                DrawTextRect(_currentX,_currentY,charWidth,_textHeight,&_currentText.at(i),ALIGN_CENTER);
                _currentX += charWidth;
            }
        }
        _cursorPositionStr = _currentText.size();
    }
    drawFooter();
    //TODO draw cursor on first enter
    //FillArea(_currentX,_currentY, _cursorThickness,_textHeight, BLACK);
    PartialUpdate(_contentRect.x, _contentRect.y, _contentRect.w, _contentRect.h);
}

void TextView::addPage()
{
    _shownPage++;
    _page = _shownPage;
    _lastPageLineCount.insert(std::pair<int,int>(_page,_lineCount));
    _pageCharPos.insert(std::pair<int,int>(_page,_currentText.size()-1));
    _currentX = _textBeginX;
    _currentY = _textBeginY;
    _cursorPositionStr = _currentText.size();

    FillAreaRect(&_contentRect, WHITE);
    drawFooter();
    FillArea(_currentX,_currentY, _cursorThickness,_textHeight, BLACK);
    PartialUpdate(_contentRect.x, _contentRect.y, _contentRect.w, _contentRect.h);
}
