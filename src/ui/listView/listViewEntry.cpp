//------------------------------------------------------------------
// hnCommentViewEntry.cpp
//
// Author:           JuanJakobo
// Date:             04.08.2020
//
//-------------------------------------------------------------------

#include "inkview.h"
#include "listViewEntry.h"

ListViewEntry::ListViewEntry(int page, const irect &rect) : _page(page), _position(rect)
{
}

void ListViewEntry::drawSeperator()
{
    int line = (_position.y + _position.h) - 1;
    DrawLine(0, line, ScreenWidth(), line, BLACK);
}
