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
#ifndef GAME_H
#define GAME_H

#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#include <SDL/SDL_ttf.h>
#include <vector>
#include <map>
#include <string>
#include "GameState.h"
#include "GUIObject.h"
#include "GameObjects.h"
#include "Player.h"
#include "Shadow.h"

//This structure contains variables that make a GameObjectEvent.
struct typeGameObjectEvent{
	//The type of event.
	int eventType;
	//The type of object that should react to the event.
	int objectType;
	//Flags, 0x1 means use the id.
	int flags; 
	//Blocks with this id should react to the event.
	std::string id;
};

class ThemeManager;
class ThemeBackground;
class TreeStorageNode;

class Game : public GameState,public GUIEventCallback{
private:
	//Boolean if the game should reset.
	bool isReset;
	
	//contains currently played level.
	TreeStorageNode* currentLevelNode;

protected:
	//Array containing "tooltips" for certain block types.
	//It will be shown in the topleft corner of the screen.
	SDL_Surface* bmTips[TYPE_MAX];
	
	//SDL_Surface containing the action images (record, play, etc..)
	SDL_Surface* action;
	//SDL_Surface containing the medal image.
	SDL_Surface* medals;

	//Vector containing all the levelObjects in the current game.
	std::vector<GameObject*> levelObjects;

	//The name of the current level.
	std::string levelName;
	//The path + file of the current level.
	std::string levelFile;

	//Editor data containing information like name, size, etc...
	std::map<std::string,std::string> editorData;

	//Vector used to queue the gameObjectEvents.
	std::vector<typeGameObjectEvent> eventQueue;

	//The themeManager.
	ThemeManager* customTheme;
	//The themeBackground.
	ThemeBackground* background;

	//Load a level from node.
	//After calling this function the ownership of
	//node will transfer to Game class. So don't delete
	//the node after calling this function!
	virtual void loadLevelFromNode(TreeStorageNode* obj, const std::string& fileName);

public:
	//Array used to convert GameObject type->string.
	static const char* blockName[TYPE_MAX];
	//Map used to convert GameObject string->type.
	static std::map<std::string,int> blockNameMap;
	
	//Boolean that is set to true when a game is won.
	bool won;
	
	//Boolean if the replaying currently done is for the interlevel screen.
	bool interlevel;

	//Integer containing the current tip index.
	int gameTipIndex;
	
	//Integer containing the number of ticks passed since the start of the level.
	int time;
	//Integer containing the stored value of time.
	int timeSaved;
	
	//Integer containing the number of recordings it took to finish.
	int recordings;
	//Integer containing the stored value of recordings.
	int recordingsSaved;
	
	//Boolean if the camera should follow the shadow or not.
	bool shadowCam;

	//The player...
	Player player;
	//... and his shadow.
	Shadow shadow;

	//warning: weak reference only, may point to invalid location
	GameObject *objLastCheckPoint;

	//Constructor.
	//loadLevel: Boolean if the GameState should load the level.
	Game(bool loadLevel=true);
	//If this is not empty then when next Game class is created
	//it will play this record file.
	static std::string recordFile;
	//Destructor.
	//It will call destroy();
	~Game();

	//Method used to clean up the GameState.
	void destroy();

	//Inherited from GameState.
	void handleEvents();
	void logic();
	void render();
	
	//This method will load a level.
	//fileName: The fileName of the level.
	virtual void loadLevel(std::string fileName);

	//Method used to broadcast a GameObjectEvent.
	//eventType: The type of event.
	//objectType: The type of object that should react to the event.
	//id: The id of the blocks that should react.
	void broadcastObjectEvent(int eventType,int objectType=-1,const char* id=NULL);

	//Method used to store the current state.
	//This is used for checkpoints.
	//Returns: True if it succeeds without problems.
	bool saveState();
	//Method used to load the stored state.
	//This is used for checkpoints.
	//Returns: True if it succeeds without problems.
	bool loadState();
	//Method that will reset the GameState to it's initial state.
	//save: Boolean if the saved state should also be delted.
	void reset(bool save);

	//Save current game record to the file.
	//fileName: The filename of the destination file.
	void saveRecord(const char* fileName);
	//Load game record (and its level) from file and play it.
	//fileName: The filename of the recording file.
	void loadRecord(const char* fileName);
	
	//Method called by the player (or shadow) when he finished.
	void replayPlay();
	//Method that gets called when the recording has ended.
	void recordingEnded();

	//get current level's auto-save record path,
	//using current level's MD5, file name and other information.
	void getCurrentLevelAutoSaveRecordPath(std::string &bestTimeFilePath,std::string &bestRecordingFilePath,bool createPath);
	
	//Method that will prepare the gamestate for the next level and start it.
	//If it's the last level it will show the congratulations text and return to the level select screen.
	void gotoNextLevel();
	
	//GUI event handling is done here.
	void GUIEventCallback_OnEvent(std::string name,GUIObject* obj,int eventType);
};

#endif
