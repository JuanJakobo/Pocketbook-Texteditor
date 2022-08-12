//------------------------------------------------------------------
// listView.cpp
//
// Author:           JuanJakobo
// Date:             04.08.2020
//
//-------------------------------------------------------------------

#include "inkview.h"
#include "listView.h"
#include "listViewEntry.h"

#include <string>
#include <vector>

#include "log.h"

using std::string;
using std::vector;

ListView::ListView(const irect &contentRect, int shownPage) : View(contentRect,shownPage)
{
    _entries.clear();

    _entryFontHeight = contentRect.h/45;

    _entryFont = OpenFont("LiberationMono", _entryFontHeight, 1);
    _entryFontBold = OpenFont("LiberationMono-Bold", _entryFontHeight, 1);

    SetFont(_entryFont, BLACK);
}

ListView::~ListView()
{
    CloseFont(_entryFont);
    //free(_entryFont);
    CloseFont(_entryFontBold);
}

void ListView::draw()
{
    FillAreaRect(&_contentRect, WHITE);
    drawEntries();
    drawFooter();
    PartialUpdate(_contentRect.x, _contentRect.y, _contentRect.w, _contentRect.h);
}

void ListView::reDrawCurrentEntry()
{
    FillAreaRect(&_entries.at(_selectedEntry)->getPosition(), WHITE);
    _entries.at(_selectedEntry)->draw(_entryFont, _entryFontBold, _entryFontHeight);
    updateEntry(_selectedEntry);
}

void ListView::invertCurrentEntryColor()
{
    InvertAreaBW(_entries.at(_selectedEntry)->getPosition().x, _entries.at(_selectedEntry)->getPosition().y, _entries.at(_selectedEntry)->getPosition().w, _entries.at(_selectedEntry)->getPosition().h);
    updateEntry(_selectedEntry);
}

void ListView::drawEntries()
{
    for (unsigned int i = 0; i < _entries.size(); i++)
    {
        if (_entries.at(i)->getPage() == _shownPage)
            _entries.at(i)->draw(_entryFont, _entryFontBold, _entryFontHeight);
    }
}

//TODO name
bool ListView::checkIfEntryClicked(int x, int y)
{
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
    else
    {
        for (unsigned int i = 0; i < _entries.size(); i++)
        {
            if (_entries.at(i)->getPage() == _shownPage && IsInRect(x, y, &_entries.at(i)->getPosition()) == 1)
            {
                _selectedEntry = i;
                invertCurrentEntryColor();
                return true;
            }
        }
    }
    return false;
}


void ListView::updateEntry(int entryID)
{
    PartialUpdate(_entries.at(entryID)->getPosition().x, _entries.at(entryID)->getPosition().y, _entries.at(entryID)->getPosition().w, _entries.at(entryID)->getPosition().h);
}
