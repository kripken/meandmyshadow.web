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
#ifndef GAME_OBJECTS_H
#define GAME_OBJECTS_H

#include "Globals.h"
#include <SDL/SDL.h>
#include <string>
#include <vector>
#include <utility>
#include <map>

class Game;
class Player;

//The different gameObject events.
enum GameObjectEventType{
	//Event called when the player walks on the gameObject.
	GameObjectEvent_PlayerWalkOn=1,
	//Event called when the player is on the gameObject.
	GameObjectEvent_PlayerIsOn,
	//Event called when the player leaves the gameObject.
	//Currently unimplemented.
	GameObjectEvent_PlayerLeave, 
	
	//Event called when the player toggles it. (DOWN key)
	GameObjectEvent_OnToggle=0x10000,
	//Event called when the player switches it on. (DOWN key)
	GameObjectEvent_OnSwitchOn=0x10001,
	//Event called when the player switches it off. (DOWN key)
	GameObjectEvent_OnSwitchOff=0x10002,
};

//The different gameObject properties.
enum GameObjectPropertyType{
	//If the player can walk on the gameObject.
	GameObjectProperty_PlayerCanWalkOn=1,
	//If the object is spiked.
	GameObjectProperty_IsSpikes,
	//If the gameObject has some flags.
	GameObjectProperty_Flags,
};

//The different box types that can be requested using the getBox(int boxType) method.
enum GameObjectBoxType{
	//Box of the current position.
	BoxType_Current=0,
	//Box of the base/start position.
	BoxType_Base,
	//Box of the previous position.
	BoxType_Previous,
	//The movement of the block since last position.
	BoxType_Delta,
	//The velocity for when the player is standing on it.
	BoxType_Velocity,
};

//The GameObject class.
class GameObject{
protected:
	//The box of the gameObject.
	//It's used for the location of the gameObject and the size.
	SDL_Rect box;
public:
	//The type of the GameObject.
	int type;
	//Pointer to the Game state.
	Game* parent;

	//Constructor.
	//parent: Pointer to the Game state.
	GameObject(Game* parent);
	//Destructor.
	~GameObject();

	//Method used to retrieve a certain box from the GameObject.
	//boxType: The type of box that is requested. (default=0)
	//Returns: An SDL_Rect.
	virtual SDL_Rect getBox(int boxType=0);
	
	//This method is used to place the location on a given location.
	//x: The x location to place the gameObject.
	//y: The y location to place the gameObject.
	virtual void setPosition(int x,int y);
	
	//Method used to draw the GameObject.
	virtual void show()=0;
	//Save the state of the GameObject, used for moving blocks, etc.
	virtual void saveState();
	//Load the state of the GameObject, used for moving blocks, etc.
	virtual void loadState();
	//Reset the state of the GameObject, used for moving blocks, etc.
	//save: Boolean if the saved state should also be reset.
	virtual void reset(bool save);

	//Play an animation.
	//flags: TODO???
	virtual void playAnimation(int flags);
	//Invoke an event of the GameObject.
	//eventType: The event type.
	virtual void onEvent(int eventType);
	//Method used to request certain properties of the GameObject.
	//propertyType: The property that is requested.
	//obj: Pointer to the player.
	virtual int queryProperties(int propertyType,Player* obj);

	//Method used to retrieve the additional editor data for the GameObject.
	//Used for messages, moving positions, etc...
	//obj: Vector containing the editorData pairs. (key, value)
	virtual void getEditorData(std::vector<std::pair<std::string,std::string> >& obj);
	//Set the editorData.
	//obj: Map containing the key/value for the editor data.
	virtual void setEditorData(std::map<std::string,std::string>& obj);
	
	//Update method for GameObjects, used for moving blocks.
	virtual void move();
};

//We include block.h here because it needs some things defined above.
#include "Block.h"
#endif
