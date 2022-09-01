//------------------------------------------------------------------
// devicesViewEntry.cpp
//
// Author:           JuanJakobo
// Date:             08.09.2021
//
//-------------------------------------------------------------------

#include "deviceViewEntry.h"
#include "deviceModel.h"

#include <string>

void DeviceViewEntry::draw(const ifont *entryFont, const ifont *entryFontBold, int fontHeight)
{
    SetFont(entryFontBold, BLACK);
    int heightOfTitle = TextRectHeight(_position.w, _entry.name.c_str(), 0);
    DrawTextRect(_position.x, _position.y, _position.w, heightOfTitle, _entry.name.c_str(), ALIGN_LEFT);

    SetFont(entryFont, BLACK);

    DrawTextRect(_position.x, _position.y + heightOfTitle, _position.w, fontHeight, _entry.sysfs.c_str(), ALIGN_LEFT);
    DrawTextRect(_position.x, _position.y + heightOfTitle + fontHeight, _position.w, fontHeight, _entry.uniq.c_str(), ALIGN_LEFT);
    DrawTextRect(_position.x, _position.y + heightOfTitle + fontHeight, _position.w, fontHeight, std::to_string(_entry.eventID).c_str(), ALIGN_RIGHT);

    drawSeperator();
}
