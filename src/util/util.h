//------------------------------------------------------------------
// util.h
//
// Author:           JuanJakobo
// Date:             04.08.2020
// Description:      Various utility methods
//-------------------------------------------------------------------

#ifndef UTIL
#define UTIL

#include "inkview.h"

#include "eventHandler.h"

#include <string>

enum class Action
{
    IWriteSecret,
    IReadSecret,
    IWriteString,
    IReadString,
    IWriteInt,
    IReadInt
};

const std::string CONFIG_PATH = CONFIG_FOLDER + "/config.cfg";

class Util
{
public:

    template <typename T>
    static T accessConfig(const Action &action, const std::string &name, T value)
    {
        iconfigedit *temp = nullptr;
        iconfig *config = OpenConfig(CONFIG_PATH.c_str(), temp);
        T returnValue;

        if constexpr(std::is_same<T, Entry>::value)
        {
        }
        if constexpr(std::is_same<T, std::string>::value)
        {
            switch (action)
            {
                case Action::IWriteSecret:
                    WriteSecret(config, name.c_str(), value.c_str());
                    returnValue = {};
                    break;
                case Action::IReadSecret:
                    returnValue = ReadSecret(config, name.c_str(), "");
                    break;
                case Action::IWriteString:
                    WriteString(config, name.c_str(), value.c_str());
                    returnValue = {};
                    break;
                case Action::IReadString:
                    returnValue = ReadString(config, name.c_str(), "");
                    break;
                default:
                    break;
            }
        }
        else if constexpr(std::is_same<T, int>::value)
        {
            switch(action)
            {
                case Action::IWriteInt:
                    WriteInt(config, name.c_str(), value);
                    returnValue = 0;
                    break;
                case Action::IReadInt:
                    returnValue = ReadInt(config, name.c_str(), 0);
                    break;
                default:
                    break;
            }
        }
        CloseConfig(config);

        return returnValue;
    }

private:
    Util() {}
};
#endif
