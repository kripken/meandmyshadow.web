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
#ifndef LEVELSELECT_H
#define LEVELSELECT_H

#include "GameState.h"
#include "GameObjects.h"
#include "Player.h"
#include "GUIObject.h"
#include "GUIScrollBar.h"
#include "GUIListBox.h"
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#include <SDL/SDL_ttf.h>
#include <vector>
#include <string>

//Class that represents a level in the levelselect menu.
class Number{
private:
	//The background image of the number.
	SDL_Surface* background;
	//The background image of the number when it's locked.
	SDL_Surface* backgroundLocked;
	//The (text) image of the number.
	SDL_Surface* image;
	
	//Image containing the three stars a player can earn.
	SDL_Surface* medals;
	
	//The number (or text).
	int number;
	//Integer containing the medal the player got.
	//0 = none, 1 = bronze, 2 = silver, 3 = gold
	int medal;
	
	//Boolean if the number is locked or not.
	bool locked;
public:
	//The location and size of the number.
	SDL_Rect box;

	//If the Number is selected then we draw something indicates it.
	bool selected;

	//Constructor.
	Number();
	//Destructor.
	~Number();

	//Method used for initialising the number.
	//number: The number.
	//box: The location and size of the number.
	void init(int number,SDL_Rect box);
	
	//Method used for initialising the number.
	//text: The caption of the number.
	//box: The location and size of the number.
	void init(std::string text,SDL_Rect box);

	//get current number.
	inline int getNumber(){return number;}
	
	//Method used to set the locked status of the number.
	//locked: Boolean if it should be locked or not.
	void setLocked(bool locked=true);
	//Method used to retrieve the locked status of the number.
	//Returns: True if the number is locked.
	inline bool getLocked(){return locked;}
	
	//Method used to set the medal for this number.
	//medal: The new medal for this number.
	void setMedal(int medal);
	
	//Method that is used to draw the number.
	//dy: The y offset.
	void show(int dy);
};

//This is the LevelSelect state, here you can select levelpacks and levels.
class LevelSelect : public GameState,public GUIEventCallback{
protected:
	//The background image which is drawn before the rest.
	SDL_Surface* background;
	//Surface containing the title.
	SDL_Surface* title;
	
	//Vector containing the numbers.
	std::vector<Number> numbers;
	
	//This hashmap is used to get the path to the levelpack by giving the name of the levelpack.
	std::map<std::string,std::string> levelpackLocations;

	//Contains selected level number (displayed at bottom left corner).
	//If it's NULL then nothing selected.
	Number* selectedNumber;
	
	//Pointer to the scrollbar.
	GUIScrollBar* levelScrollBar;
	//Pointer to the description.
	GUIObject* levelpackDescription;
	
	//Pointer to the levelpack list.
	GUISingleLineListBox* levelpacks;
	
	//Check where and if the mouse clicked on a number.
	//If so select that number.
	virtual void checkMouse();

	//Levels displayed in level select screen without scrollbar (4 rows and 10 levels for each row)
	static const int LEVELS_DISPLAYED_IN_SCREEN=40;
public:
	//Constructor.
	//titleText: The title that is shown at the top of the screen.
	LevelSelect(std::string titleText);
	//Destructor.
	virtual ~LevelSelect();

	//Method used to update the numbers and the scrollbar.
	virtual void refresh()=0;
	
	//Method that is called when a number is selected.
	//number: The selected number.
	//selected: Boolean if the number was already selected.
	virtual void selectNumber(unsigned int number,bool selected)=0;

	//Inherited from GameState.
	void handleEvents();
	void logic();
	void render();
	
	//Method that is called to render the tooltip.
	//number: The number that the tooltip should be drawn for.
	//dy: The y offset of the number, used to draw the tooltip in the right place.
	virtual void renderTooltip(unsigned int number,int dy)=0;

	//GUI events will be handled here.
	virtual void GUIEventCallback_OnEvent(std::string name,GUIObject* obj,int eventType)=0;
};

#endif
