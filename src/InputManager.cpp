/****************************************************************************
** Copyright (C) 2012 me and my shadow developers
**
**
** This file may be used under the terms of the GNU General Public
** License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
**  You should have received a copy of the GNU General Public License
**  along with this program.  If not, see <http://www.gnu.org/licenses/>.
**
****************************************************************************/

#include "InputManager.h"
#include "Globals.h"
#include "Settings.h"
#include "Functions.h"
#include "GUIObject.h"
#include "GUIListBox.h"
#include <stdlib.h>
#include <stdio.h>
#include <string>
using namespace std;

InputManager inputMgr;

//the order must be the same as InputManagerKeys
static const char* keySettingNames[INPUTMGR_MAX]={
	"key_up","key_down","key_left","key_right","key_jump","key_action","key_space",
	"key_escape","key_restart","key_tab","key_save","key_load","key_swap",
	"key_teleport","key_suicide","key_shift","key_next","key_previous"
};

//the order must be the same as InputManagerKeys
static const char* keySettingDescription[INPUTMGR_MAX]={
	"Up (in menu)","Down (in menu)","Left","Right","Jump","Action","Space (Record)",
	"Escape","Restart","Tab (View shadow/Level prop.)","Save game (in editor)","Load game","Swap (in editor)",
	"Teleport (in editor)","Suicide (in editor)","Shift (in editor)","Next block type","Previous block type"
};

class InputDialogHandler:public GUIEventCallback{
private:
	//the list box which contains keys.
	GUIListBox* listBox;
	//the parent object.
	InputManager* parent;
	//check if it's alternative key
	bool isAlternativeKey;

	//update specified key config item
	void updateConfigItem(int index){
		//get the description
		std::string s=keySettingDescription[index];
		s+=": ";
		
		//get key code name
		int keyCode=parent->getKeyCode((InputManagerKeys)index,isAlternativeKey);
		s+=InputManager::getKeyCodeName(keyCode);

		//show it
		listBox->item[index]=s;
	}
public:
	//Constructor.
	InputDialogHandler(GUIListBox* listBox,InputManager* parent):listBox(listBox),parent(parent),isAlternativeKey(false){
		//load the avaliable keys to the list box.
		for(int i=0;i<INPUTMGR_MAX;i++){
			//get the description
			std::string s=keySettingDescription[i];
			s+=": ";
			
			//get key code name
			int keyCode=parent->getKeyCode((InputManagerKeys)i,false);
			s+=InputManager::getKeyCodeName(keyCode);

			//add item
			listBox->item.push_back(s);
		}
	}
	
	//when a key is pressed call this to set the key to currently-selected item
	void onKeyDown(int keyCode){
		//check if an item is selected.
		int index=listBox->value;
		if(index<0 || index>=INPUTMGR_MAX) return;

		//set it.
		parent->setKeyCode((InputManagerKeys)index,keyCode,isAlternativeKey);
		updateConfigItem(index);
	}

	void GUIEventCallback_OnEvent(std::string name,GUIObject* obj,int eventType){
		//Make sure it's a click event.
		if(eventType==GUIEventClick){
			if(name=="cmdOK"){
				//config is done, exiting
				delete GUIObjectRoot;
				GUIObjectRoot=NULL;
			}else if(name=="cmdUnset"){
				onKeyDown(0);
			}else if(name=="lstType"){
				isAlternativeKey=(obj->value==1);
				for(int i=0;i<INPUTMGR_MAX;i++){
					updateConfigItem(i);
				}
			}
			/*else if(name=="cmdEscape"){
				//set a key to Escape
				//simply call onKeyDown().
				onKeyDown(SDLK_ESCAPE);
			}*/
		}
	}
};

int InputManager::getKeyCode(InputManagerKeys key,bool isAlternativeKey){
	if(isAlternativeKey) return alternativeKeys[key];
	else return keys[key];
}

void InputManager::setKeyCode(InputManagerKeys key,int keyCode,bool isAlternativeKey){
	if(isAlternativeKey) alternativeKeys[key]=keyCode;
	else keys[key]=keyCode;
}

void InputManager::loadConfig(){
	int i;
	for(i=0;i<INPUTMGR_MAX;i++){
		string s=keySettingNames[i];

		keys[i]=atoi(getSettings()->getValue(s).c_str());

		s+="2";
		alternativeKeys[i]=atoi(getSettings()->getValue(s).c_str());
	}
}

void InputManager::saveConfig(){
	int i;
	char c[32];
	for(i=0;i<INPUTMGR_MAX;i++){
		string s=keySettingNames[i];

		sprintf(c,"%d",keys[i]);
		getSettings()->setValue(s,c);

		s+="2";
		sprintf(c,"%d",alternativeKeys[i]);
		getSettings()->setValue(s,c);
	}
}

void InputManager::showConfig(){
	//Save the old GUI.
	GUIObject* tmp=GUIObjectRoot;

	//Create the new GUI.
	GUIObjectRoot=new GUIObject((SCREEN_WIDTH-600)/2,(SCREEN_HEIGHT-400)/2,600,400,GUIObjectFrame,"Config Keys");
	GUIObject* obj;

	obj=new GUIObject(20,36,560,36,GUIObjectLabel,"Select an item and press a key to config it.");
	GUIObjectRoot->childControls.push_back(obj);

	//The list box.
	GUIListBox *listBox=new GUIListBox(20,116,560,220);

	//Event handler.
	InputDialogHandler handler(listBox,this);
	GUIObjectRoot->childControls.push_back(listBox);

	//another box to select key type
	GUISingleLineListBox *listBox0=new GUISingleLineListBox(120,72,360,36);
	listBox0->name="lstType";
	listBox0->item.push_back("Primary key");
	listBox0->item.push_back("Alternative key");
	listBox0->value=0;
	listBox0->eventCallback=&handler;
	GUIObjectRoot->childControls.push_back(listBox0);

	//two buttons
	obj=new GUIObject(20,360,360,36,GUIObjectButton,"Unset the key");
	obj->name="cmdUnset";
	obj->eventCallback=&handler;
	GUIObjectRoot->childControls.push_back(obj);
	/*
	obj=new GUIObject(20,360,360,36,GUIObjectButton,"Set to ESCAPE key");
	obj->name="cmdEscape";
	obj->eventCallback=&handler;
	GUIObjectRoot->childControls.push_back(obj);
	*/
	obj=new GUIObject(460,360,120,36,GUIObjectButton,"OK");
	obj->name="cmdOK";
	obj->eventCallback=&handler;
	GUIObjectRoot->childControls.push_back(obj);

	//Now we keep rendering and updating the GUI.
	SDL_FillRect(tempSurface,NULL,0);
	SDL_SetAlpha(tempSurface,SDL_SRCALPHA,155);
	SDL_BlitSurface(tempSurface,NULL,screen,NULL);
	while(GUIObjectRoot){
		while(SDL_PollEvent(&event)){
			//check if some key is down.
			if(event.type==SDL_KEYDOWN){
				handler.onKeyDown(event.key.keysym.sym);
			}
			//Joystick
			else if(event.type==SDL_JOYAXISMOTION){
				if(event.jaxis.value>3200){
					handler.onKeyDown(0x00010001 | (int(event.jaxis.axis)<<8));
				}else if(event.jaxis.value<-3200){
					handler.onKeyDown(0x000100FF | (int(event.jaxis.axis)<<8));
				}
			}
			else if(event.type==SDL_JOYBUTTONDOWN){
				handler.onKeyDown(0x00020000 | (int(event.jbutton.button)<<8));
			}
			else if(event.type==SDL_JOYHATMOTION){
				if(event.jhat.value & SDL_HAT_LEFT){
					handler.onKeyDown(0x00030000 | (int(event.jhat.hat)<<8) | SDL_HAT_LEFT);
				}else if(event.jhat.value & SDL_HAT_RIGHT){
					handler.onKeyDown(0x00030000 | (int(event.jhat.hat)<<8) | SDL_HAT_RIGHT);
				}else if(event.jhat.value & SDL_HAT_UP){
					handler.onKeyDown(0x00030000 | (int(event.jhat.hat)<<8) | SDL_HAT_UP);
				}else if(event.jhat.value & SDL_HAT_DOWN){
					handler.onKeyDown(0x00030000 | (int(event.jhat.hat)<<8) | SDL_HAT_DOWN);
				}
			}
			//now process GUI events.
			GUIObjectHandleEvents(true);
		}
		if(GUIObjectRoot)
			GUIObjectRoot->render();
		SDL_Flip(screen);
		SDL_Delay(30);
	}

	//Restore the old GUI.
	GUIObjectRoot=tmp;
}


//get key name from key code
std::string InputManager::getKeyCodeName(int keyCode){
	char c[64];
	if(keyCode>0 && keyCode <0x1000){
		//keyboard
		char* s=SDL_GetKeyName((SDLKey)keyCode);
		if(s!=NULL){
			return s;
		}else{
			sprintf(c,"(Key %d)",keyCode);
			return c;
		}
	}else if(keyCode>0x1000){
		//Joystick. first set it to invalid value
		sprintf(c,"(Joystick 0x%08X)",keyCode);
		//check type
		switch((keyCode & 0x00FF0000)>>16){
		case 1:
			//axis
			switch(keyCode & 0xFF){
			case 1:
				sprintf(c,"Joystick axis %d +",(keyCode & 0x0000FF00)>>8);
				break;
			case 0xFF:
				sprintf(c,"Joystick axis %d -",(keyCode & 0x0000FF00)>>8);
				break;
			}
			break;
		case 2:
			//button
			sprintf(c,"Joystick button %d",(keyCode & 0x0000FF00)>>8);
			break;
		case 3:
			//hat
			switch(keyCode & 0xFF){
			case SDL_HAT_LEFT:
				sprintf(c,"Joystick hat %d left",(keyCode & 0x0000FF00)>>8);
				break;
			case SDL_HAT_RIGHT:
				sprintf(c,"Joystick hat %d right",(keyCode & 0x0000FF00)>>8);
				break;
			case SDL_HAT_UP:
				sprintf(c,"Joystick hat %d up",(keyCode & 0x0000FF00)>>8);
				break;
			case SDL_HAT_DOWN:
				sprintf(c,"Joystick hat %d down",(keyCode & 0x0000FF00)>>8);
				break;
			}
			break;
		}
		return c;
	}else{
		//unknown??
		return "(Not set)";
	}
}

InputManager::InputManager(){
	//clear the array.
	for(int i=0;i<INPUTMGR_MAX;i++){
		keys[i]=alternativeKeys[i]=keyFlags[i]=0;
	}
}

InputManager::~InputManager(){
	closeAllJoysticks();
}

int InputManager::getKeyState(int keyCode,int oldState,bool hasEvent){
	int state=0;
	if(keyCode>0 && keyCode<0x1000){
		//keyboard
		if(hasEvent){
			if(event.type==SDL_KEYDOWN && event.key.keysym.sym==keyCode){
				state|=0x2;
			}
			if(event.type==SDL_KEYUP && event.key.keysym.sym==keyCode){
				state|=0x4;
			}
		}
		if(keyCode<SDLK_LAST && SDL_GetKeyState(NULL)[keyCode]){
			state|=0x1;
		}
	}else if(keyCode>0x1000){
		//Joystick
		int index=(keyCode & 0x0000FF00)>>8;
		int value=keyCode & 0xFF;
		int i,v;
		switch((keyCode & 0x00FF0000)>>16){
		case 1:
			//axis
			if(hasEvent){
				if(event.type==SDL_JOYAXISMOTION && event.jaxis.axis==index){
					if((value==1 && event.jaxis.value>3200) || (value==0xFF && event.jaxis.value<-3200)){
						if((oldState & 0x1)==0) state|=0x2;
					}else{
						if(oldState & 0x1) state|=0x4;
					}
				}
			}
			for(i=0;i<(int)joysticks.size();i++){
				v=SDL_JoystickGetAxis(joysticks[i],index);
				if((value==1 && v>3200) || (value==0xFF && v<-3200)){
					state|=0x1;
					break;
				}
			}
			break;
		case 2:
			//button
			if(hasEvent){
				if(event.type==SDL_JOYBUTTONDOWN && event.jbutton.button==index){
					state|=0x2;
				}
				if(event.type==SDL_JOYBUTTONUP && event.jbutton.button==index){
					state|=0x4;
				}
			}
			for(i=0;i<(int)joysticks.size();i++){
				v=SDL_JoystickGetButton(joysticks[i],index);
				if(v){
					state|=0x1;
					break;
				}
			}
			break;
		case 3:
			//hat
			if(hasEvent){
				if(event.type==SDL_JOYHATMOTION && event.jhat.hat==index){
					if(event.jhat.value & value){
						if((oldState & 0x1)==0) state|=0x2;
					}else{
						if(oldState & 0x1) state|=0x4;
					}
				}
			}
			for(i=0;i<(int)joysticks.size();i++){
				v=SDL_JoystickGetHat(joysticks[i],index);
				if(v & value){
					state|=0x1;
					break;
				}
			}
			break;
		}
	}
	return state;
}

//update the key state, according to current SDL event, etc.
void InputManager::updateState(bool hasEvent){
	for(int i=0;i<INPUTMGR_MAX;i++){
		keyFlags[i]=getKeyState(keys[i],keyFlags[i],hasEvent)|getKeyState(alternativeKeys[i],keyFlags[i],hasEvent);
	}
}

//check if there is KeyDown event.
bool InputManager::isKeyDownEvent(InputManagerKeys key){
	return keyFlags[key]&0x2;
}

//check if there is KeyUp event.
bool InputManager::isKeyUpEvent(InputManagerKeys key){
	return keyFlags[key]&0x4;
}

//check if specified key is down.
bool InputManager::isKeyDown(InputManagerKeys key){
	return keyFlags[key]&0x1;
}

//open all joysticks.
void InputManager::openAllJoysitcks(){
	int i,m;
	//First close previous joysticks.
	closeAllJoysticks();

	//open all joysticks.
	m=SDL_NumJoysticks();
	for(i=0;i<m;i++){
		SDL_Joystick *j=SDL_JoystickOpen(i);
		if(j==NULL){
			printf("ERROR: Couldn't open Joystick %d\n",i);
		}else{
			joysticks.push_back(j);
		}
	}
}

//close all joysticks.
void InputManager::closeAllJoysticks(){
	for(int i=0;i<(int)joysticks.size();i++){
		SDL_JoystickClose(joysticks[i]);
	}
	joysticks.clear();
}
