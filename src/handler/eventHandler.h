//------------------------------------------------------------------
// eventHandler.h
//
// Author:           JuanJakob
// Date:             22.04.2021
// Description:      Handles all events and directs them
//-------------------------------------------------------------------

#ifndef EVENT_HANDLER
#define EVENT_HANDLER

#include "inkview.h"

#include "mainMenu.h"

#include "devicesView.h"
#include "deviceModel.h"

#include "fileView.h"

#include <string>
#include <memory>
#include <map>
#include <vector>

enum class Views
{
    DEFAULTVIEW,
    FILVIEW,
    DEVICEVIEW,
    TXVIEW
};

const std::string CONFIG_FOLDER = "/mnt/ext1/system/config/textEditor";
const std::string ARTICLE_FOLDER = "/mnt/ext1/textEditor";

class EventHandler
{
    public:
        /**
         * Defines fonds, sets global Event Handler and starts new content 
         */
        EventHandler();

        ~EventHandler();

        /**
         * Handles events and redirects them
         * 
         * @param type event type
         * @param par1 first argument of the event
         * @param par2 second argument of the event
         * @return int returns if the event was handled
         */
        int eventDistributor(const int type, const int par1, const int par2);

    private:
        static std::unique_ptr<EventHandler> _eventHandlerStatic;
        MainMenu _menu = MainMenu("Text Editor");
        std::unique_ptr<DevicesView> _devicesView;
        std::unique_ptr<FileView> _fileView;
        Views _currentView;
        Device _currentDevice;
        std::string _currentPath;
        std::string _tempKeyboard;

        /**
         * Function needed to call C function, redirects to real function
         * 
         *  @param index int of the menu that is set
         */
        static void mainMenuHandlerStatic(const int index);

        /**
         * Handles menu events and redirects them
         * 
         * @param index int of the menu that is set
         */
        void mainMenuHandler(const int index);

        /**
         * Handles pointer Events
         * 
         * @param type event type
         * @param par1 first argument of the event
         * @param par2 second argument of the event
         * @return int returns if the event was handled
         */
        int pointerHandler(const int type, const int par1, const int par2);

        int keyHandler(const int type, const int par1, const int par2);

        bool createDevice(const Device &device);

        void createInputEvent();

        void getLocalFiles(const std::string &path);

    /**
     * Function needed to call C function, redirects to real function
     *
     *  @param  text typed in by the user to the keyboard
     */
    static void keyboardHandlerStatic(char *text);

    /**
     *  function to handle input of user and create new file
     *
     *  @param  text typed in by the user to the keyboard
     */
    void keyboardHandler(char *text);

};
#endif
