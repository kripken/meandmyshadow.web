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
#ifndef LEVELS_H
#define LEVELS_H

#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#include <SDL/SDL_ttf.h>
#include <vector>
#include <string>
#include "GameObjects.h"
#include "Player.h"


class Levels{
public:
	//A level entry structure.
	struct Level{
		//The name of the level.
		string name;
		//The filename of the level.
		string file;
		
		//Boolean if the level is locked.
		bool locked;
		//Boolean if the level is won.
		bool won;
		
		//Integer containing the number of ticks (40 = 1s) it took to finish the level.
		//If there's no time the value will be -1.
		int time;
		//Integer containing the target time to get a medal.
		int targetTime;
		
		//Integer containing the number of recordings used to finish the level.
		//When not won the value is -1.
		int recordings;
		//Integer containing the target recordings to get a medal.
		int targetRecordings;

		//MD5 of level node. :/
		unsigned char md5Digest[16];
	};
private:
	//Index of the current level.
	int currentLevel;
	
	//Boolean if the levels are loaded.
	bool loaded;

	//Vector containing the filenames of the levels.
	std::vector<Level> levels;
	
	//The file name of the level progress.
	std::string levelProgressFile;

public:
	//The location the levelpack is stored.
	std::string levelpackPath;
	//A description of the levelpack.
	std::string levelpackDescription;
	
	//The text that will be displayed when the levels are finished.
	std::string congratulationText;
	
	//Constructor.
	Levels():currentLevel(0),loaded(false),levels(){};

	//Adds a level to the levels.
	//levelFileName: The filename of the level to add.
	//level: The index of the level to add.
	void addLevel(const std::string& levelFileName,int levelno=-1);
	//Removes a level from the levels.
	//level: The index of the level to remove.
	void removeLevel(unsigned int level);
	//Moves the level to a given index.
	//level1: The level to move.
	//level2: The destination.
	void moveLevel(unsigned int level1,unsigned int level2);
	//Swaps two level.
	//level1: The first level to swap.
	//level2: The second level to swap.
	void swapLevel(unsigned int level1,unsigned int level2);

	//Get the levelFile for a given level.
	//level: The level index to get the levelFileName from.
	//Returns: String containing the levelFileName.
	const std::string& getLevelFile(int level=-1);
	//Get the levelpackPath of the levels.
	//Returns: String containing the levelpackPath.
	const std::string& getLevelpackPath();
	//Get the levelName for a given level.
	//level: The level index to get the levelName from.
	//Returns: String containing the levelName.
	const std::string& getLevelName(int level=-1);
	//Sets the levelName for a given level.
	//level: The level index to get the levelName from.
	//name: The new name of the level.
	void setLevelName(unsigned int level,const std::string& name);
	//Get the MD5 for a given level.
	//level: The level index.
	//Returns: const unsigned char[16] contains the digest.
	const unsigned char* getLevelMD5(int level=-1);

	//get level's auto-save record path,
	//using level's MD5, file name and other information.
	void getLevelAutoSaveRecordPath(int level,std::string &bestTimeFilePath,std::string &bestRecordingFilePath,bool createPath);

	//Set the currentLevel.
	//level: The new current level.
	void setCurrentLevel(unsigned int level);
	//Get the currentLevel.
	//Returns: The currentLevel.
	inline int getCurrentLevel(){return currentLevel;}
	//Get the levelCount.
	//Returns: The level count.
	inline int getLevelCount(){return levels.size();}
	
	//Method that will return the requested level.
	//level: The index of the level, default is the current level.
	//Returns: Pointer to the requested level structure.
	struct Level* getLevel(int level=-1);
	
	//Method that will reset any progress/statistics for a given level.
	//level: The index of the level to reset, default is currentLevel.
	void resetLevel(int level=-1);
	
	//Check if a certain level is locked.
	//level: The index of the level to check.
	//Returns: True if the level is locked.
	bool getLocked(unsigned int level);
	//Set a level locked or not.
	//level: The level to (un)lock.
	//locked: The new status of the level, default is unlocked (false).
	void setLocked(unsigned int level,bool locked=false);

	//Empties the levels.
	void clear();
	
	
	bool loadLevels(const std::string& levelListFile);
	void loadProgress(const std::string& levelProgressFile);
	void saveLevels(const std::string& levelListFile);
	void saveLevelProgress();

	void nextLevel();

};
#endif
