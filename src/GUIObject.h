/****************************************************************************
** Copyright (C) 2011 Luka Horvat <redreaper132 at gmail.com>
** Copyright (C) 2011 Edward Lii <edward_iii at myway.com>
** Copyright (C) 2011 O. Bahri Gordebak <gordebak at gmail.com>
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
#ifndef GUIOBJECT_H
#define GUIOBJECT_H

#include "Globals.h"
#include "Functions.h"
#include "FileManager.h"
#include <string>
#include <vector>
#include <list>

//Ids for the different GUIObject types.
//None is a special type, it has no visual form.
const int GUIObjectNone=0;
//A label used to dispaly text.
const int GUIObjectLabel=1;
//Button which will invoke an event when pressed.
const int GUIObjectButton=2;
//Checkbox which represents a boolean value and can be toggled.
const int GUIObjectCheckBox=3;
//A text box used to enter text.
const int GUIObjectTextBox=5;
//Frame which is like a container.
const int GUIObjectFrame=6;

//The event id's.
//A click event used for e.g. buttons.
const int GUIEventClick=0;
//A change event used for e.g. textboxes.
const int GUIEventChange=1;


class GUIObject;

//Class that is used as event callback.
class GUIEventCallback{
public:
	//This method is called when an event is fired.
	//name: The name of the event.
	//obj: Pointer to the GUIObject which caused this event.
	//eventType: The type of event as defined above.
	virtual void GUIEventCallback_OnEvent(std::string name,GUIObject* obj,int eventType)=0;
};

//Class containing the 
class GUIObject{
public:
	//The relative x location of the GUIObject.
	int left;
	//The relative y location of the GUIObject.
	int top;
	//The width of the GUIObject.
	int width;
	//The height of the GUIObject.
	int height;
	
	//The type of the GUIObject.
	int type;
	//The value of the GUIObject.
	//It depends on the type of GUIObject what it means.
	int value;

	//The name of the GUIObject.
	std::string name;
	//The caption of the GUIObject.
	//It depends on the type of GUIObject what it is.
	std::string caption;
	
	//Boolean if the GUIObject is enabled.
	bool enabled;
	//Boolean if the GUIObject is visible.
	bool visible;
	
	//Vector containing the children of the GUIObject.
	std::vector<GUIObject*> childControls;
	
	//Event callback used to invoke events.
	GUIEventCallback* eventCallback;
protected:
	//The state of the GUIObject.
	//It depends on the type of GUIObject where it's used for.
	int state;
	
	//Surface containing some gui images.
	SDL_Surface* bmGUI;
public:
	//Constructor.
	//left: The relative x location of the GUIObject.
	//top: The relative y location of the GUIObject.
	//witdh: The width of the GUIObject.
	//height: The height of the GUIObject.
	//type: The type of the GUIObject.
	//caption: The text on the GUIObject.
	//value: The value of the GUIObject.
	//enabled: Boolean if the GUIObject is enabled or not.
	//visible: Boolean if the GUIObject is visisble or not.
	GUIObject(int left=0,int top=0,int width=0,int height=0,int type=0,
		const char* caption=NULL,int value=0,
		bool enabled=true,bool visible=true):
		left(left),top(top),width(width),height(height),
		type(type),value(value),
		enabled(enabled),visible(visible),
		eventCallback(NULL),state(0)
	{
		//Make sure that caption isn't NULL before setting it.
		if(caption) GUIObject::caption=caption;
		
		//Load the gui images.
		bmGUI=loadImage(getDataPath()+"gfx/gui.png");
	}
	//Destructor.
	virtual ~GUIObject();
	
	//Method used to handle mouse and/or key events.
	//x: The x mouse location.
	//y: The y mouse location.
	//enabled: Boolean if the parent is enabled or not.
	//visible: Boolean if the parent is visible or not.
	//processed: Boolean if the event has been processed (by the parent) or not.
	//Returns: Boolean if the event is processed by the child.
	virtual bool handleEvents(int x=0,int y=0,bool enabled=true,bool visible=true,bool processed=false);
	//Method that will render the GUIObject.
	//x: The x location to draw the GUIObject. (x+left)
	//y: The y location to draw the GUIObject. (y+top)
	virtual void render(int x=0,int y=0);
};

//Method used to handle the GUIEvents from the GUIEventQueue.
//kill: Boolean if an SDL_QUIT event may kill the GUIObjectRoot.
void GUIObjectHandleEvents(bool kill=false);

//A structure containing the needed variables to call an event.
struct GUIEvent{
	//Event callback used to invoke the event.
	GUIEventCallback* eventCallback;
	//The name of the event.
	std::string name;
	//Pointer to the object which invoked the event.
	GUIObject* obj;
	//The type of event.
	int eventType;
};

//List used to queue the gui events.
extern std::list<GUIEvent> GUIEventQueue;

#endif
