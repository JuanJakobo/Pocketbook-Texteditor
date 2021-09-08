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

#include <string>
#include <map>
#include <vector>

#include <iostream>
#include <fstream>
#include <sstream>

#include <linux/input.h>

using std::string;
using std::vector;

std::unique_ptr<EventHandler> EventHandler::_eventHandlerStatic;

struct input_event event;

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
								//create input event
								case 101:
												{
																createInputEvent();
																break;
												}
								//start input mode
								case 102: 
												{
																startInputMode();
																break;
												}
								//Exit
								case 103:
												{
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
								//if menu is clicked
								if (IsInRect(par1, par2, _menu.getMenuButtonRect()) == 1)
								{
												return _menu.createMenu(EventHandler::mainMenuHandlerStatic);
								}
								else if (_currentView == Views::DEVICEVIEW)
								{
												if (_devicesView->checkIfEntryClicked(par1,par2))
												{
																_devicesView->invertCurrentEntryColor();

																if (iv_access("mnt/secure/su", R_OK) != 0){
																				Message(ICON_ERROR,"Error","No root access available.",2000);
																				Log::writeInfoLog("no root access");
																				_devicesView->invertCurrentEntryColor();
																				return 1;
																}

																_currentDevice = *_devicesView->getCurrentEntry();

																std::ifstream infile("/sys" + _currentDevice.sysfs + "/event" + std::to_string(_currentDevice.eventID) + "/uevent");
																string line;
																string major, minor, devname;

																while(std::getline(infile, line))
																{
																				if(line.find("MAJOR") != std::string::npos)
																								major = line.substr(line.find('=')+1);
																				if(line.find("MINOR") != std::string::npos)
																								minor = line.substr(line.find('=')+1);
																				if(line.find("DEVNAME") != std::string::npos)
																								devname = line.substr(line.find('=')+1);
																}

																//auto z = system ("/mnt/secure/su rm event7");
																string mknod = "/mnt/secure/su mknod -m 664 /dev/input/event" + std::to_string(_currentDevice.eventID) +  " c " + major + " " + minor;
																Log::writeInfoLog(mknod);
																auto i = system(mknod.c_str());
																Log::writeInfoLog("system return code " + std::to_string(i));
																startInputMode();

												}

								}
				}
				return 0;
}

void EventHandler::createInputEvent()
{
				//auto a = GetBluetoothStatus();
				//Log::writeInfoLog("bluetooth status " + std::to_string(a));

				if (GetBluetoothMode() == BLUETOOTH_OFF)
								SetBluetoothOn();

				if(IsBluetoothAwake() == 0)
								BluetoothWakeUp();

				if(IsBluetoothEnabled() == 1){
								std::ifstream infile("/proc/bus/input/devices");
								string line;

								Device temp;
								vector<Device> devices;

								while(std::getline(infile, line))
								{
												if(line.front() == 'N')
																temp.name = line.substr(line.find('=')+1);
												if(line.front() == 'S')
																temp.sysfs = line.substr(line.find('=')+1);
												if(line.front() == 'U'){
																temp.uniq = line.substr(line.find('=')+1);
												}
												if(line.front() == 'H'){
																string handlers =line.substr(line.find('=')+1);
																int t = handlers.find("event");
																if(t != std::string::npos){
																				handlers = handlers.substr(t);
																				handlers = handlers.substr(5,1);
																				temp.eventID = std::stoi(handlers);				
																}
												}
												//TODO filter by type
												//B: --> EV 
												//B: --> KEY
												else if(line.empty() && !temp.name.empty() && temp.eventID > 6){
																devices.push_back(temp);
																temp = {};
												}
								}

								//write devices to list
								if(devices.size() > 0){
												_devicesView.reset(new DevicesView(_menu.getContentRect(),devices,1));
												_devicesView->draw();
												_currentView = Views::DEVICEVIEW;
								}else{
												FillAreaRect(_menu.getContentRect(), WHITE);
												DrawTextRect2(_menu.getContentRect(), "No bluetooth keyboards available. Please pair a new one using bluetoothctl");
												_currentView = Views::DEFAULTVIEW;
												PartialUpdate(_menu.getContentRect()->x, _menu.getContentRect()->y, _menu.getContentRect()->w, _menu.getContentRect()->h);
								}

				}else{
								Message(ICON_ERROR,"Error", "Could not enable Bluetooth",2000); 
				}
}

void EventHandler::startInputMode(){

				if(_currentDevice.name.empty()){
								createInputEvent();
				}else{

								FillArea(_menu.getContentRect()->x, _menu.getContentRect()->y, _menu.getContentRect()->w, _menu.getContentRect()->h,WHITE);
								PartialUpdate(_menu.getContentRect()->x, _menu.getContentRect()->y, _menu.getContentRect()->w, _menu.getContentRect()->h);

								std::ifstream eventFile("/dev/input/event" + std::to_string(_currentDevice.eventID), std::ifstream::in);

								auto textHeight = 30;
								auto textNextLineY = textHeight + 10;
								auto charWidth = 20;
								auto textBeginX = 0;
								auto textEndX = ScreenWidth()-20;
								auto textBeginY = _menu.getContentRect()->y;
								auto textFont = OpenFont("Roboto", textHeight , FONT_STD);
								SetFont(textFont, BLACK);

								if(eventFile.is_open()) {

												auto currentX = textBeginX;
												auto currentY = textBeginY;

												string currentText;

												string textPath = ARTICLE_FOLDER + std::string("/text.txt");

												if (iv_access(textPath.c_str(), W_OK) == 0){
																std::ifstream inFile(textPath);
																string line;
																bool multipleLines = false;

																if(inFile.is_open()){
																				while ( getline(inFile,line)){
																								if(multipleLines){
																												currentX = textBeginX;
																												currentY += textNextLineY;
																												currentText += "\n";
																								}

																								for(const char &c : line)
																								{
																												//here function
																												multipleLines = true;
																												if(currentX > textEndX){
																																currentX = textBeginX;
																																currentY += textNextLineY;
																												}
																												DrawTextRect(currentX,currentY,CharWidth(c),textHeight,&c,ALIGN_CENTER);
																												PartialUpdate(currentX,currentY,CharWidth(c),textHeight);
																												currentX += charWidth; //CharWidth(c);
																								}

																								currentText += line;
																				}
																				inFile.close();
																}

																//PartialUpdate(_menu.getContentRect()->x, _menu.getContentRect()->y, _menu.getContentRect()->w, _menu.getContentRect()->h);

												}

												bool writeToScreen;
												char data[sizeof(event)];
												bool inputSession = true;
												Message(ICON_INFORMATION, "Information", "To cancel input mode press ESC.", 2000);
												while(inputSession) {
																eventFile.read(data,sizeof(event));
																memcpy(&event, data, sizeof(event));

																char key;
																writeToScreen = true;	

																if(event.type == EV_KEY) {
																				if(event.value == EV_KEY)
																				{
																								//https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/tree/include/uapi/linux/input-event-codes.h

																								if(event.code == KEY_SPACE) {
																												key = ' ';
																								}else if(event.code == KEY_BACKSPACE) {
																												writeToScreen = false;
																												if(!currentText.empty()){
																																if(currentText.back() == '\n'){
																																				currentText.pop_back();
																																				int last = currentText.find_last_of('\n');
																																				if(last != std::string::npos){
																																								currentText.pop_back();
																																								currentX = currentText.substr(last).size() * charWidth;
																																				}else{
																																								currentX = currentText.size() * charWidth;
																																								currentText.pop_back();
																																				}
																																				currentY -= textNextLineY;
																																				currentX -= charWidth;
																																				FillArea(currentX,currentY, charWidth, textHeight, WHITE);
																																				PartialUpdate(currentX,currentY,charWidth,textHeight);

																																}else{
																																				if(currentX < textBeginX && currentY > textBeginY){
																																								//TODO other method needed 
																																								currentX = textEndX;
																																								currentY -= textNextLineY;
																																				}
																																				currentX -= charWidth;
																																				currentText.pop_back();
																																				FillArea(currentX,currentY, charWidth, textHeight, WHITE);
																																				PartialUpdate(currentX,currentY,charWidth,textHeight);
																																}

																												}else{
																																Message(ICON_INFORMATION, "Information", "no more characters to delete.", 2000);
																												}
																												//TODO write to text
																								}else if(event.code == KEY_ENTER) {
																												currentX = textBeginX;
																												currentY += textNextLineY;
																												currentText += "\n";
																												writeToScreen = false;
																								}else if(event.code == KEY_DOT) {
																												key = '.';
																								}else if(event.code == KEY_COMMA) {
																												key = ',';
																								}else if(event.code == KEY_A) {
																												key = 'A';
																								}else if(event.code == KEY_B) {
																												key = 'B';
																								}else if(event.code == KEY_C) {
																												key = 'C';
																								}else if(event.code == KEY_D) {
																												key = 'D';
																								}else if(event.code == KEY_E) {
																												key = 'E';
																								}else if(event.code == KEY_F) {
																												key = 'F';
																								}else if(event.code == KEY_G) {
																												key = 'G';
																								}else if(event.code == KEY_H) {
																												key = 'H';
																								}else if(event.code == KEY_I) {
																												key = 'I';
																								}else if(event.code == KEY_J) {
																												key = 'J';
																								}else if(event.code == KEY_K) {
																												key = 'K';
																								}else if(event.code == KEY_L) {
																												key = 'L';
																								}else if(event.code == KEY_M) {
																												key = 'M';
																								}else if(event.code == KEY_N) {
																												key = 'N';
																								}else if(event.code == KEY_O) {
																												key = 'O';
																								}else if(event.code == KEY_P) {
																												key = 'P';
																								}else if(event.code == KEY_Q) {
																												key = 'Q';
																								}else if(event.code == KEY_R) {
																												key = 'R';
																								}else if(event.code == KEY_S) {
																												key = 'S';
																								}else if(event.code == KEY_T) {
																												key = 'T';
																								}else if(event.code == KEY_U) {
																												key = 'U';
																								}else if(event.code == KEY_V) {
																												key = 'V';
																								}else if(event.code == KEY_W) {
																												key = 'W';
																								}else if(event.code == KEY_X) {
																												key = 'X';
																								}else if(event.code == KEY_Y) {
																												key = 'Y';
																								}else if(event.code == KEY_Z) {
																												key = 'Z';
																								}else if(event.code == KEY_0) {
																												key = '0';
																								}else if(event.code == KEY_1) {
																												key = '1';
																								}else if(event.code == KEY_2) {
																												key = '2';
																								}else if(event.code == KEY_3) {
																												key = '3';
																								}else if(event.code == KEY_4) {
																												key = '4';
																								}else if(event.code == KEY_5) {
																												key = '5';
																								}else if(event.code == KEY_6) {
																												key = '6';
																								}else if(event.code == KEY_7) {
																												key = '7';
																								}else if(event.code == KEY_8) {
																												key = '8';
																								}else if(event.code == KEY_9) {
																												key = '9';
																								}else if(event.code == KEY_ESC) {
																												inputSession = false;
																												if(!currentText.empty()){
																																std::ofstream text(textPath);
																																text << currentText;
																																text.close();
																																Message(ICON_INFORMATION,"Information", "Input Mode closed. File saved.",1000);
																												}else{
																																string path = ARTICLE_FOLDER + "/text.txt";
																																remove(path.c_str());
																																Message(ICON_INFORMATION,"Information", "Input Mode closed. File removed.",1000);
																												}


																												writeToScreen = false;
																								}
																								else {
																												Log::writeInfoLog("event code not included yet " + std::to_string(event.code));
																												writeToScreen = false;
																								}

																								if (writeToScreen){
																												//FillArea(currentX+charWidth+10,currentY,currentX+charWidth+10,currentY + textHeight, WHITE);
																												currentText += key;
																												if(currentX  > textEndX){
																																currentX = textBeginX;
																																currentY += textNextLineY;
																												}
																												string text = {key};
																												DrawTextRect(currentX,currentY, charWidth, textHeight, text.c_str(), ALIGN_CENTER);
																												PartialUpdate(currentX,currentY,charWidth,textHeight);
																												currentX += charWidth;//CharWidth(key);
																												//DrawLine(currentX+charWidth+10,currentY,currentX+charWidth+10,currentY + textHeight,BLACK);				
																												//PartialUpdate(currentX+charWidth+10,currentY,currentX+charWidth+10,currentY + textHeight);
																								}
																				}
																}
																else 
																{
																				//Log::writeInfoLog("event type " + std::to_string(event.type));
																}
												}
												eventFile.close();
								}
								else {
												DrawTextRect(0, (ScreenHeight() / 3) * 2, ScreenWidth(), 30, strerror(errno), ALIGN_CENTER);
												PartialUpdate(_menu.getContentRect()->x, _menu.getContentRect()->y, _menu.getContentRect()->w, _menu.getContentRect()->h);
								}

								Log::writeInfoLog("closing input mode");
								CloseFont(textFont);
				}
}

