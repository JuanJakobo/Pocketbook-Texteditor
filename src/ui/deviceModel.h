//------------------------------------------------------------------
// devicesModel.h
//
// Author:           JuanJakobo
// Date:             23.04.2021
// Description:      
//-------------------------------------------------------------------

#ifndef DEVICESMODEL
#define DEVICESMODEL

#include "model.h"

#include <string>

struct Device : Entry{
				std::string name;
				std::string sysfs;
				std::string uniq;
				int eventID;
};

#endif
