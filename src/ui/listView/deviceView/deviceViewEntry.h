//------------------------------------------------------------------
// deviceViewEntry.h
//
// Author:           JuanJakobo
// Date:             08.09.2021
// Description:      
//-------------------------------------------------------------------

#ifndef DEVICESVIEWENTRY
#define DEVICESVIEWENTRY

#include "listViewEntry.h"
#include "deviceModel.h"

class DeviceViewEntry : public ListViewEntry
{
public:
    /**
        * Creates an DevicesViewEntry
        * 
        * @param Page site of the listView the Entry is shown
        * @param Rect area of the screen the item is positioned
        * @param entry entry that shall be drawn
        */
    DeviceViewEntry(int page, const irect &position, const Device &entry) : ListViewEntry(page, position), _entry(entry){}

    /**
        * draws the DeviceViewEntry to the screen
        * 
        * @param entryFont font for the entry itself
        * @param entryFontBold bold font for the header
        * @param fontHeight height of the font 
        */
    void draw(const ifont *entryFont, const ifont *entryFontBold, int fontHeight) override;

    Device &get() { return _entry; };

private:
    Device _entry;
};
#endif
