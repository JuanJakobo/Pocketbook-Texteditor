//------------------------------------------------------------------
// listView.h
//
// Author:           JuanJakobo
// Date:             04.08.2020
// Description:      An UI class to display items in a listview
//-------------------------------------------------------------------

#ifndef LISTVIEW
#define LISTVIEW

#include "inkview.h"
#include "listViewEntry.h"
#include "model.h"
#include "view.h"

#include <vector>
#include <string>
#include <memory>

template <typename T, typename B>
class ListView : public View
{
    public:
        /**
         * Displays a list view
         *
         * @param ContentRect area of the screen where the list view is placed
         * @param Items items that shall be shown in the listview
         */

        ListView(const irect &contentRect, const std::vector<B> &entries, int shownPage = 1) : View(contentRect,shownPage)
    {
        _entries.clear();

        _entryFontHeight = contentRect.h/45;

        _entryFont = OpenFont("LiberationMono", _entryFontHeight, 1);
        _entryFontBold = OpenFont("LiberationMono-Bold", _entryFontHeight, 1);

        SetFont(_entryFont, BLACK);

        setEntries(entries);
    }

        ~ListView()
        {
            CloseFont(_entryFont);
            //free(_entryFont);
            CloseFont(_entryFontBold);
        }

        B &getCurrentEntry() { return getEntry(_selectedEntry);};
        B &getEntry(int entryID) { return _entries.at(entryID)->get();};


        void setEntries(const std::vector<B> &entries)
        {
            auto pageHeight = 0;
            auto contentHeight = _contentRect.h - _footerHeight;
            auto entrycount = entries.size();

            _entries.reserve(entrycount);

            auto i = 0;
            while (i < entrycount)
            {
                auto entrySize = TextRectHeight(_contentRect.w, entries.at(i).name.c_str(), 0) + 2.5 * _entryFontHeight;
                if ((pageHeight + entrySize) > contentHeight)
                {
                    pageHeight = 0;
                    _page++;
                }
                irect rect = iRect(_contentRect.x, _contentRect.y + pageHeight, _contentRect.w, entrySize, 0);

                _entries.emplace_back(std::unique_ptr<T>(new T(_page, rect, entries.at(i))));

                i++;
                pageHeight = pageHeight + entrySize;
            }
            draw();
        }

        /**
         * Draws an single entry to the screen
         */
        void reDrawCurrentEntry()
        {
            FillAreaRect(&_entries.at(_selectedEntry)->getPosition(), WHITE);
            _entries.at(_selectedEntry)->draw(_entryFont, _entryFontBold, _entryFontHeight);
            updateEntry(_selectedEntry);
        }

        /**
         * inverts the color of the currently selected entry
         */
        void invertCurrentEntryColor()
        {
            InvertAreaBW(_entries.at(_selectedEntry)->getPosition().x, _entries.at(_selectedEntry)->getPosition().y, _entries.at(_selectedEntry)->getPosition().w, _entries.at(_selectedEntry)->getPosition().h);
            updateEntry(_selectedEntry);
        }

        /**
         * Checkes if the listview has been clicked and either changes the page or returns item ID
         *
         * @param x x-coordinate
         * @param y y-coordinate
         * @return true if was clicked
         */
        bool checkIfEntryClicked(int x, int y) override
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


        /**
         * Clears the screen and draws entries and footer
         *
         */
        void draw() override
        {
            FillAreaRect(&_contentRect, WHITE);
            drawEntries();
            drawFooter();
            PartialUpdate(_contentRect.x, _contentRect.y, _contentRect.w, _contentRect.h);
        }


    private:
        int _entryFontHeight;
        int _selectedEntry;
        ifont *_entryFont;
        ifont *_entryFontBold;
        std::vector<std::unique_ptr<T>> _entries;

        int getCurrentEntryItertator() const {return _selectedEntry;};

        /**
         * Iterates through the items and sends them to the listViewEntry Class for drawing
         */
        void drawEntries()
        {
            for (unsigned int i = 0; i < _entries.size(); i++)
            {
                if (_entries.at(i)->getPage() == _shownPage)
                    _entries.at(i)->draw(_entryFont, _entryFontBold, _entryFontHeight);
            }
        }

        /**
         * updates an entry
         *
         * @param entryID the id of the item that shall be inverted
         */
        void updateEntry(int entryID)
        {
            PartialUpdate(_entries.at(entryID)->getPosition().x, _entries.at(entryID)->getPosition().y, _entries.at(entryID)->getPosition().w, _entries.at(entryID)->getPosition().h);
        }
};
#endif
