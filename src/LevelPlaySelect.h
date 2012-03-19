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
#ifndef LEVELPLAYSELECT_H
#define LEVELPLAYSELECT_H

#include "GameState.h"
#include "LevelSelect.h"
#include "GameObjects.h"
#include "Player.h"
#include "GUIObject.h"
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#include <SDL/SDL_ttf.h>
#include <vector>
#include <string>

//This is the LevelSelect state, here you can select levelpacks and levels.
class LevelPlaySelect : public LevelSelect{
private:
	//Pointer to the play button, it is only shown when a level is selected.
	GUIObject* play;
	
	//Image of a play icon used as button to start replays.
	SDL_Surface* playButtonImage;
	
	//Image containing the time icon.
	SDL_Surface* timeIcon;
	//Image containing the recordings icon.
	SDL_Surface* recordingsIcon;
	
	//display level info.
	void displayLevelInfo(int number);

	//Check where and if the mouse clicked on a number.
	//If so it will start the level.
	void checkMouse();
public:
	//Constructor.
	LevelPlaySelect();
	//Destructor.
	~LevelPlaySelect();

	//Inherited from LevelSelect.
	void refresh();
	void selectNumber(unsigned int number,bool selected);
	
	//Inherited from GameState.
	void render();

	//Inherited from LevelSelect.
	void renderTooltip(unsigned int number,int dy);
	
	//GUI events will be handled here.
	void GUIEventCallback_OnEvent(std::string name,GUIObject* obj,int eventType);
};

#endif
