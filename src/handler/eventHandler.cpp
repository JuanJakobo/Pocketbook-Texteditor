//------------------------------------------------------------------
// eventHandler.cpp
//
// Author:           JuanJakobo
// Date:             22.04.2021
//
//-------------------------------------------------------------------

#include "eventHandler.h"
#include "inkview.h"

#include "mainMenu.h"

#include "log.h"
#include "util.h"

#include "view.h"

#include "textView.h"
#include "errorView.h"

#include "fileBrowser.h"
#include "fileModel.h"

#include <string>
#include <fstream>


using std::string;
using std::vector;

std::unique_ptr<EventHandler> EventHandler::_eventHandlerStatic;


EventHandler::EventHandler()
{
    //create an copy of the eventhandler to handle methods that require static functions
    _eventHandlerStatic = std::unique_ptr<EventHandler>(this);

    if (iv_access(CONFIG_FOLDER.c_str(), W_OK) != 0)
        iv_mkdir(CONFIG_FOLDER.c_str(), 0777);
    if (iv_access(ARTICLE_FOLDER.c_str(), W_OK) != 0)
        iv_mkdir(ARTICLE_FOLDER.c_str(), 0777);

    SetOrientation(Util::accessConfig(Action::IReadInt, "orientation",0));
    createInputEvent();
}

EventHandler::~EventHandler()
{
}

int EventHandler::eventDistributor(const int type, const int par1, const int par2)
{
    if (ISPOINTEREVENT(type))
        return EventHandler::pointerHandler(type, par1, par2);
    else if (ISKEYEVENT(type))
        return EventHandler::keyHandler(type,par1,par2);
    return 0;
}

void EventHandler::mainMenuHandlerStatic(const int index)
{
    _eventHandlerStatic->mainMenuHandler(index);
}

void EventHandler::mainMenuHandler(const int index)
{
    switch (index)
    {
        //start input mode
        case 101:
            {
                if (_currentDevice.name.empty()){
                    createInputEvent();
                }else{
                    getLocalFiles(ARTICLE_FOLDER);
                }
                break;
            }
            //set orientation
        case 102:
            {
                // Set screen orientation: 0=portrait, 1=landscape 90, 2=landscape 270, 3=portrait 180
                int orientation = []{
                    switch(DialogSynchro(ICON_QUESTION, "Action", "How to turn?", "Portrait", "Landscape 270", "Cancel"))
                    {
                        case 1:
                            return 0;
                            break;
                        case 2:
                            return 2;
                            break;
                        default:
                            return -1;
                            break;
                    }
                }();
                if (GetOrientation() != orientation && orientation != -1)
                {
                    SetOrientation(orientation);
                    Util::accessConfig(Action::IWriteInt, "orientation",orientation);
                    _currentViews->setContentRect(_menu.getContentRect());
                    _currentViews->draw();
                }
                break;
            }
            //create File
        case 103:
            {
                _tempKeyboard = "Filename";
                _tempKeyboard.resize(60);
                if (iv_access(_currentPath.c_str(), W_OK) == 0)
                    OpenKeyboard(_tempKeyboard.c_str(), &_tempKeyboard[0], 60 - 1, KBD_NORMAL, &keyboardHandlerStatic);
                else
                    Message(ICON_INFORMATION,"Error", "Can not write to current location",2000);
                break;
            }
            //Exit
        case 104:
            {
                if (IsBluetoothEnabled() == 1){
                    Message(ICON_INFORMATION,"Information","Disabling Bluetooth",2000);
                    SetBluetoothOff();
                }
                CloseApp();
                break;
            }
        default:
            break;
    }
}

int EventHandler::pointerHandler(const int type, const int par1, const int par2)
{
    if (type == EVT_POINTERUP)
    {
        if (_currentViews->checkIfEntryClicked(par1,par2))
        {

            switch(_currentView)
            {
                case Views::DEVICE:
                    {
                        if (auto view = dynamic_cast<ListView<DeviceViewEntry,Device>*>(_currentViews.get()))
                        {
                            if (createDevice(view->getCurrentEntry()))
                            {
                                getLocalFiles(ARTICLE_FOLDER);
                            }
                            else
                            {
                                view->invertCurrentEntryColor();
                            }
                        }
                        break;
                    }
                case Views::FIL:
                    {
                        if (auto view = dynamic_cast<ListView<FileViewEntry,FileItem>*>(_currentViews.get()))
                        {
                            if (view->getCurrentEntry().type == Type::FFOLDER)
                            {
                                auto path = view->getCurrentEntry().path;
                                getLocalFiles(path);
                            }
                            else
                            {
                                _currentViews.reset(new TextView(_menu.getContentRect(), _currentDevice,view->getCurrentEntry().path));
                                _currentView = Views::TEXT;

                            }
                        }
                        break;
                    }
                default:
                    break;
            }
            return 0;
        }
    }
    return 1;
}

int EventHandler::keyHandler(const int type, const int par1, const int par2)
{
    //TODO lock
    if (type == EVT_KEYPRESS)
    {
        switch(par1)
        {
            case 23: //menu button
                return _menu.createMenu(EventHandler::mainMenuHandlerStatic, _currentView);
            case 24: //left button
                (GetOrientation() == 0 || GetOrientation() == 3) ? _currentViews->prevPage() : _currentViews->nextPage();
                break;
            case 25: //right button
                (GetOrientation() == 0 || GetOrientation() == 3) ? _currentViews->nextPage() : _currentViews->prevPage();
                break;
            default:
                return 1;
        }
    }

    return 0;
}

void EventHandler::createInputEvent()
{
    if (IsBluetoothEnabled() == 0)
        SetBluetoothOn();
    if (IsBluetoothAwake() == 0)
        BluetoothWakeUp();

    if (IsBluetoothEnabled() == 1){
        std::ifstream infile("/proc/bus/input/devices");
        string line;

        Device temp;
        vector<Device> devices;

        while(std::getline(infile, line))
        {
            switch (line.front())
            {
                case 'N':
                    temp.name = line.substr(line.find('=')+1);
                    break;
                case 'S':
                    temp.sysfs = line.substr(line.find('=')+1);
                    break;
                case 'U':
                    temp.uniq = line.substr(line.find('=')+1);
                    break;
                case 'H':
                    {
                        string handlers =line.substr(line.find('=')+1);
                        int t = handlers.find("event");
                        if (t != std::string::npos){
                            handlers = handlers.substr(t);
                            handlers = handlers.substr(5,1);
                            temp.eventID = std::stoi(handlers);
                        }
                        break;
                    }
            }
            if (line.empty() && !temp.name.empty() && temp.eventID > 6){
                devices.push_back(temp);
                temp = {};
            }
        }

        switch (devices.size())
        {
            case 1:
                if (createDevice(devices.at(0)))
                {
                    getLocalFiles(ARTICLE_FOLDER);
                    break;
                }
            case 2 ... INT_MAX :
                {
                    _currentViews.reset(new ListView<DeviceViewEntry,Device>(_menu.getContentRect(),devices));
                    _currentView = Views::DEVICE;
                    break;
                }
            default:
                {
                    _currentViews.reset(new ErrorView(_menu.getContentRect(), "No bluetooth keyboards avialable. Please pair a new one using bluetoothctl or connect a registered one. To refresh click the menu button and select \"Start input mode\"."));
                    _currentView = Views::ERROR;
                }
        }

    }else{
        Message(ICON_ERROR,"Error", "Could not enable Bluetooth",2000);
    }
}

bool EventHandler::createDevice(const Device &device)
{
    _currentDevice = device;
    if (_currentDevice.name.empty())
        createInputEvent();

    if (iv_access("mnt/secure/su", R_OK) != 0)
    {
        Message(ICON_ERROR,"Error","No root access available.",2000);
        Log::writeInfoLog("no root access");
        return false;
    }

    std::ifstream infile("/sys" + _currentDevice.sysfs + "/event" + std::to_string(_currentDevice.eventID) + "/uevent");
    string line;
    string major, minor, devname;

    while(std::getline(infile, line))
    {
        if (line.find("MAJOR") != std::string::npos)
            major = line.substr(line.find('=')+1);
        if (line.find("MINOR") != std::string::npos)
            minor = line.substr(line.find('=')+1);
        if (line.find("DEVNAME") != std::string::npos)
            devname = line.substr(line.find('=')+1);
    }

    string systemCommand ="/mnt/secure/su rm /dev/input/event" + std::to_string(_currentDevice.eventID);
    auto i = system(systemCommand.c_str());
    systemCommand = "/mnt/secure/su mknod -m 664 /dev/input/event" + std::to_string(_currentDevice.eventID) +  " c " + major + " " + minor;
    if (system(systemCommand.c_str()) != 0)
    {
        Message(ICON_ERROR,"Error","Could not create link to input.",2000);
        Log::writeInfoLog("Could not create link to input. System return code " + std::to_string(i));
        return false;
    }

    return true;
}

void EventHandler::getLocalFiles(const string &path)
{

    _currentPath = path;
    FileBrowser fb = FileBrowser(true);
    vector<FileItem> files = fb.getFileStructure(_currentPath);

    _currentViews.reset(new ListView<FileViewEntry,FileItem>(_menu.getContentRect(),files));
    _currentView = Views::FIL;
}

void EventHandler::keyboardHandlerStatic(char *text)
{
    _eventHandlerStatic->keyboardHandler(text);
}
void EventHandler::keyboardHandler(char *text)
{
    if (!text)
        return;

    string path(text);
    if (path.empty())
        return;

    path = _currentPath + '/' + path;
    std::ofstream output(path);
    getLocalFiles(_currentPath);
}
