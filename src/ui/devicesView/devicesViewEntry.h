//------------------------------------------------------------------
// devicesViewEntry.h
//
// Author:           JuanJakobo
// Date:             08.09.2021
// Description:      
//-------------------------------------------------------------------

#ifndef DEVICESVIEWENTRY
#define DEVICESVIEWENTRY

#include "listViewEntry.h"
#include "deviceModel.h"

class DevicesViewEntry : public ListViewEntry
{
public:
    /**
        * Creates an DevicesViewEntry
        * 
        * @param Page site of the listView the Entry is shown
        * @param Rect area of the screen the item is positioned
        * @param entry entry that shall be drawn
        */
    DevicesViewEntry(int page, const irect &position, const Device &entry);

    /**
        * draws the DeviceViewEntry to the screen
        * 
        * @param entryFont font for the entry itself
        * @param entryFontBold bold font for the header
        * @param fontHeight height of the font 
        */
    void draw(const ifont *entryFont, const ifont *entryFontBold, int fontHeight) override;

    Device *get() override { return &_entry; };

private:
    Device _entry;
};
#endif
