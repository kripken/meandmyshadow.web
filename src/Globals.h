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
#ifndef GLOBALS_H
#define GLOBALS_H

#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#include <SDL/SDL_ttf.h>
#include <string>

#ifdef WIN32
//#define DATA_PATH
#else
#include "config.h"
#endif

//Global constants
//The width of the screen.
const int SCREEN_WIDTH=800;
//The height of the screen.
const int SCREEN_HEIGHT=600;
//The depth of the screen.
const int SCREEN_BPP=32;

//Strin containing the 
const std::string version="V0.3";

//The height of the current level.
extern int LEVEL_HEIGHT;
//The width of the current level.
extern int LEVEL_WIDTH;

//The target frames per seconds.
const int g_FPS=40;

//The screen surface, it's used to draw on before it's drawn to the real screen.
extern SDL_Surface* screen;
//SDL_Surface with the same dimensions as screen which can be used for all kinds of (temp) drawing.
extern SDL_Surface* tempSurface;

//Font that is used for titles.
//Knewave large.
extern TTF_Font* fontTitle;
//Font that is used for captions of buttons and other GUI elements.
//Knewave small.
extern TTF_Font* fontGUI;
//Font that is used for (long) text.
//Blokletter-Viltstift small.
extern TTF_Font* fontText;

//Event, used for event handling.
extern SDL_Event event;

//GUI
class GUIObject;
extern GUIObject *GUIObjectRoot;

//The state id of the current state.
extern int stateID;
//Integer containing what the next state will be.
extern int nextState;

//String containing the name of the current level.
extern std::string levelName;

//SDL rectangle used to store the camera.
//x is the x location of the camera.
//y is the y location of the camera.
//w is the width of the camera. (equal to SCREEN_WIDTH)
//h is the height of the camera. (equal to SCREEN_HEIGHT)
extern SDL_Rect camera;

//Enumeration containing the ids of the game states.
enum GameStates
{
	//State null is a special state used to indicate no state.
	//This is used when no next state is defined.
	STATE_NULL,
	
	//This state is before the actual leveleditor used to make levelpacks.
	STATE_LEVEL_EDIT_SELECT,
	//This state is for the level editor.
	STATE_LEVEL_EDITOR,
	//This state is for the main menu.
	STATE_MENU,
	//This state is for the actual game.
	STATE_GAME,
	//Special state used when exiting meandmyshadow.
	
	STATE_EXIT,
	//This state is for the help screen.
	
	STATE_LEVEL_SELECT,
	//This state is for the options screen.
	STATE_OPTIONS,
	//This state is for the addon screen.
	STATE_ADDONS
};

//Enumeration containing the ids of the different block types.
enum GameTileType{
	//The normal solid block.
	TYPE_BLOCK=0,
	//Block representing the start location of the player.
	TYPE_START_PLAYER,
	//Block representing the start location of the shadow.
	TYPE_START_SHADOW,
	//The exit of the level.
	TYPE_EXIT,
	//The shadow block which is only solid for the shadow.
	TYPE_SHADOW_BLOCK,
	//Block that can kill both the player and the shadow.
	TYPE_SPIKES,

	//Special point where the player can save.
	TYPE_CHECKPOINT,
	//Block that will switch the location of the player and the shadow when invoked.
	TYPE_SWAP,
	//Block that will crumble to dust when stepped on it for the third time.
	TYPE_FRAGILE,

	//Normal block that moves along a path.
	TYPE_MOVING_BLOCK,
	//Shadow block that moves along a path.
	TYPE_MOVING_SHADOW_BLOCK,
	//A spike block that moves along a path.
	TYPE_MOVING_SPIKES,

	//Special block which, once entered, moves the player/shadow to a different portal.
	TYPE_PORTAL,
	//A block with a button which can activate or stop moving blocks, converyor belts
	TYPE_BUTTON,
	//A switch which can activate or stop moving blocks, converyor belts
	TYPE_SWITCH,

	//Solid block which works like 
	TYPE_CONVEYOR_BELT,
	TYPE_SHADOW_CONVEYOR_BELT,
	
	//Block that contains a message that can be read.
	TYPE_NOTIFICATION_BLOCK,

	//The (max) number of tiles.
	TYPE_MAX
};

#endif
