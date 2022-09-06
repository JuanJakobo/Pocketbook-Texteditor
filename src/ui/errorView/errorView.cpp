//
// Author:           JuanJakobo
// Date:             30.08.2022
//
//-------------------------------------------------------------------

#include "inkview.h"
#include "errorView.h"

#include <string>

using std::string;

ErrorView::ErrorView(const irect &contentRect, const string &text, int shownPage): View(contentRect,shownPage), _text(text)
{
    _textHeight = _contentRect.h / 30;
    _textFont = OpenFont("LiberationMono", _textHeight, FONT_BOLD);
    draw();
}

ErrorView::~ErrorView()
{
    CloseFont(_textFont);
    //free(_textFont);
}
bool ErrorView::checkIfEntryClicked(int x, int y)
{
    return false;
}

void ErrorView::drawPage()
{
    FillAreaRect(&_contentRect, WHITE);
    SetFont(_textFont, BLACK);
    DrawTextRect2(&_contentRect, _text.c_str());
    PartialUpdate(_contentRect.x, _contentRect.y, _contentRect.w, _contentRect.h);
}
