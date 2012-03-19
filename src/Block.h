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
#ifndef BLOCK_H
#define BLOCK_H

#include "GameObjects.h"
#include "Globals.h"
#include "ThemeManager.h"
#include <vector>
#include <SDL/SDL.h>

class Game;

class Block: public GameObject{
private:
	//The Appearance of the block.
	ThemeBlockInstance appearance;
		
	//Integer that a block can use for all sorts of things.
	int temp;
	//The save for temp when the state of the block is saved.
	int tempSave;
	
	//flags:
	//moving object 0x1=disabled
	//button bit0-1=behavior 0x4=pressed
	//switch bit0-1=behavior
	int flags;
	//The save for flags when the state of the block is saved.
	int flagsSave;

	//Following is for moving blocks.
	//The starting place for moving blocks.
	SDL_Rect boxBase;
	//Vector containing the poisitions of the moving block.
	std::vector<SDL_Rect> movingPos;
	int dx;
	int xSave;
	int dy;
	int ySave;
	//Boolean if the moving block loops his movement.
	//Default value is true.
	bool loop;
	
	//Flags of the block for the editor.
	//moving object 0x1=disabled
	//portal 0x1=automatic
	//fragile =state
	int editorFlags;
public:
	//The id of the block.
	std::string id;
	//String containing the id of the destination for portals.
	std::string destination;
	//String containing the message of the notification block.
	std::string message;
	
	//Constructor.
	//x: The x location of the block.
	//y: The y location of the block.
	//objParent: Pointer to the Game object.
	Block(int x,int y,int type,Game* objParent);
	//Desturctor.
	~Block();

	//Method used to draw the block.
	void show();

	//Returns the box of a given type.
	//boxType: The type of box that should be returned.
	//See GameObjects.h for the types.
	//Returns: The box.
	virtual SDL_Rect getBox(int boxType=BoxType_Current);
	
	//Method used to set the location of the block.
	//It will set the base box x and y location.
	//x: The new x location.
	//y: The new y location.
	virtual void setPosition(int x,int y);
	
	//Save the state of the block so we can load it later on.
	virtual void saveState();
	//Load the saved state of the block so.
	virtual void loadState();
	//Reset the block.
	//save: Boolean if the saved state should also be deleted.
	virtual void reset(bool save);
	
	//Play an animation.
	//flags: TODO???
	virtual void playAnimation(int flags);
	
	//Method called when there's an event.
	//eventType: The type of event.
	//See GameObjects.h for the eventtypes.
	virtual void onEvent(int eventType);
	
	//Method used to retrieve a property from the block.
	//propertyType: The type of property requested.
	//See GameObjects.h for the properties.
	//obj: Pointer to the player.
	//Returns: Integer containing the value of the property.
	virtual int queryProperties(int propertyType,Player* obj);
	
	//Get the editor data of the block.
	//obj: The vector that will be filled with the editorData.
	virtual void getEditorData(std::vector<std::pair<std::string,std::string> >& obj);
	//Set the editor data of the block.
	//obj: The new editor data.
	virtual void setEditorData(std::map<std::string,std::string>& obj);
	
	//Method used for updating moving blocks or elements of blocks.
	virtual void move();
};

#endif
