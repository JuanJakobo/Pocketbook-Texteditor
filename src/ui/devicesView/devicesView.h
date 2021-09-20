//------------------------------------------------------------------
// devicesView.h
//
// Author:           JuanJakobo
// Date:             08.09.2021
// Description:      An UI class to display items in a listview
//-------------------------------------------------------------------

#ifndef DEVICESVIEW
#define DEVICESVIEW

#include "deviceModel.h"
#include "listView.h"
#include "devicesViewEntry.h"

#include <vector>
#include <memory>

class DevicesView final : public ListView
{
public:
    /**
        * Displays a list view 
        * 
        * @param ContentRect area of the screen where the list view is placed
        * @param Items items that shall be shown in the listview
        * @param page page that is shown, default is 1
        */
    DevicesView(const irect *contentRect, const std::vector<Device> &devices, int page = 1);

    Device *getCurrentEntry() { return getEntry(_selectedEntry); };

    Device *getEntry(int entryID) { return std::dynamic_pointer_cast<DevicesViewEntry>(_entries.at(entryID))->get(); };
};
#endif
