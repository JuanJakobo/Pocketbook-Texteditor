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

#include "deviceModel.h"
#include "devicesView.h"

#include "fileModel.h"
#include "fileView.h"

#include "textView.h"

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
                int dialogResult = DialogSynchro(ICON_QUESTION, "Action", "How to turn?", "Portrait", "Landscape 270", "Cancel");
                switch (dialogResult)
                {
                    case 1:
                        {
                            SetOrientation(0);
                            break;
                        }
                    case 2:
                        {
                            SetOrientation(2);
                            break;
                        }
                    default:
                        break;
                }
                //TODO use views and redraw
                Message(ICON_INFORMATION,"Error", "Open menu and select what to do",3000);
                break;
            }
            //Exit
        case 103:
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
        //TODO switch view
        if (_currentView == Views::DEVICEVIEW)
        {
            if (_devicesView->checkIfEntryClicked(par1,par2))
            {
                _devicesView->invertCurrentEntryColor();
                if (createDevice(_devicesView->getCurrentEntry()))
                {
                    getLocalFiles(ARTICLE_FOLDER);
                }
                else
                {
                    _devicesView->invertCurrentEntryColor();
                    return 1;
                }
            }
        }
        else if (_currentView == Views::FILVIEW)
        {
            if (_fileView->checkIfEntryClicked(par1, par2))
            {
                _fileView->invertCurrentEntryColor();

                if (_fileView->getCurrentEntry().type == Type::FFOLDER)
                {
                    //TODO just once? and in getLocalFiles?
                    FileBrowser fileBrowser = FileBrowser(true);
                    vector<FileItem> currentFolder = fileBrowser.getFileStructure(_fileView->getCurrentEntry().path);

                    _fileView.reset(new FileView(_menu.getContentRect(), currentFolder,1));
                }
                else
                {
                    _currentView = Views::TXVIEW;
                    TextView text = TextView(_menu.getContentRect(),1, _currentDevice,_fileView->getCurrentEntry().path);

                }
                //TODO add new file
                if (false)
                {
                    string _temp = "Filename";
                    _temp.resize(60);
                    OpenKeyboard(_temp.c_str(), &_temp[0], 60 - 1, KBD_NORMAL, &keyboardHandlerStatic);
                }

            }
        }
    }
    return 0;
}

int EventHandler::keyHandler(const int type, const int par1, const int par2)
{
    //menu button
    if (type == EVT_KEYPRESS)
    {
        if (par1 == 23)
        {
            return _menu.createMenu(EventHandler::mainMenuHandlerStatic);
        }
        //left button -> pre page
        else if (par1 == 24)
        {
            return 1;
        }
        //right button -> next page
        else if (par1 == 25)
        {
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
            //TODO test
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
            //TODO filter by type
            //B: --> EV
            //B: --> KEY
            if (line.empty() && !temp.name.empty() && temp.eventID > 6){
                devices.push_back(temp);
                temp = {};
            }
        }

        //TODO test
        switch (devices.size())
        {
            case 1:
                if(createDevice(devices.at(0)))
                {
                    getLocalFiles(ARTICLE_FOLDER);
                    break;
                }
            case 2 ... INT_MAX :
                {
                    _devicesView.reset(new DevicesView(_menu.getContentRect(),devices,1));
                    _devicesView->draw();
                    _currentView = Views::DEVICEVIEW;
                    break;
                }
            default:
                {
                    FillAreaRect(&_menu.getContentRect(), WHITE);
                    auto textHeight = ScreenHeight() / 45;
                    auto startscreenFont = OpenFont("LiberationMono", textHeight, FONT_BOLD);
                    SetFont(startscreenFont, BLACK);
                    DrawTextRect2(&_menu.getContentRect(), "No bluetooth keyboards available. Please pair a new one using bluetoothctl");
                    CloseFont(startscreenFont);
                    _currentView = Views::DEFAULTVIEW;
                    PartialUpdate(_menu.getContentRect().x, _menu.getContentRect().y, _menu.getContentRect().w, _menu.getContentRect().h);
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

    FileBrowser fb = FileBrowser(true);
    vector<FileItem> files = fb.getFileStructure(path);



    //TODO add entry "add new file"
    /*
    if (files.size() <= 0)
    {
        FillAreaRect(&_menu.getContentRect(), WHITE);
        std::ofstream output(ARTICLE_FOLDER + "/HelloWorld.txt");
        Message(ICON_INFORMATION,"Info", "No files available. Will create an new one. You can add further files to the TextEditor folder.",2000);
        getLocalFiles(ARTICLE_FOLDER);
    }
    else
    {
    */
        _fileView.reset(new FileView(_menu.getContentRect(),files, 1));
        _currentView = Views::FILVIEW;
}

void EventHandler::keyboardHandlerStatic(char *text)
{
    _eventHandlerStatic->keyboardHandler(text);
}
void EventHandler::keyboardHandler(char *text)
{
    if (!text)
        return;

    string s(text);
    if (s.empty())
        return;

    Log::writeErrorLog(s);
}
