//------------------------------------------------------------------
// listView.cpp
//
// Author:           JuanJakobo
// Date:             04.08.2020
//------------------------------------------------------------------

#include "inkview.h"
#include "view.h"

#include <string>
#include <vector>

using std::string;
using std::vector;

View::View(const irect &contentRect, int shownPage) : _contentRect(contentRect), _shownPage(shownPage)
{
    _footerHeight = _contentRect.h / 15;
    _footerFontHeight = 0.3 * _footerHeight;
    _footerFont = OpenFont("LiberationMono", _footerFontHeight, 1);

    int footerWidth = contentRect.w/20;

    _pageIcon = iRect(_contentRect.w - footerWidth*2, _contentRect.h + _contentRect.y - _footerHeight, contentRect.w/10, _footerHeight, ALIGN_CENTER);
    _firstPageButton = iRect(_contentRect.x, _contentRect.h + _contentRect.y - _footerHeight, contentRect.w/8, _footerHeight, ALIGN_CENTER);
    _prevPageButton = iRect(_contentRect.x + footerWidth*3, _contentRect.h + _contentRect.y - _footerHeight, contentRect.w/8, _footerHeight, ALIGN_CENTER);
    _nextPageButton = iRect(_contentRect.x + footerWidth*6, _contentRect.h + _contentRect.y - _footerHeight, contentRect.w/8, _footerHeight, ALIGN_CENTER);
    _lastPageButton = iRect(_contentRect.x + footerWidth*9, _contentRect.h + _contentRect.y - _footerHeight, contentRect.w/8, _footerHeight, ALIGN_CENTER);
}

View::~View()
{
    CloseFont(_footerFont);
    //free(_footerFont);
}

void View::drawFooter()
{
    SetFont(_footerFont, WHITE);
    string footer = std::to_string(_shownPage) + "/" + std::to_string(_page);
    FillAreaRect(&_pageIcon, BLACK);

    DrawTextRect2(&_pageIcon, footer.c_str());
    FillAreaRect(&_firstPageButton, BLACK);
    DrawTextRect2(&_firstPageButton, "First");
    FillAreaRect(&_prevPageButton, BLACK);
    DrawTextRect2(&_prevPageButton, "Prev");
    FillAreaRect(&_nextPageButton, BLACK);
    DrawTextRect2(&_nextPageButton, "Next");
    FillAreaRect(&_lastPageButton, BLACK);
    DrawTextRect2(&_lastPageButton, "Last");
}

void View::actualizePage(int pageToShow)
{
    if (pageToShow > _page)
    {
        Message(ICON_INFORMATION, "Info", "You have reached the last page, to return to the first, please click \"first.\"", 1200);
    }
    else if (pageToShow < 1)
    {
        Message(ICON_INFORMATION, "Info", "You are already on the first page.", 1200);
    }
    else
    {
        _shownPage = pageToShow;
        draw();
    }
}
