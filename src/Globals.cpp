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
#include "Globals.h"
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#include <SDL/SDL_ttf.h>

//Set the default value for the level width and height.
int LEVEL_HEIGHT=0;
int LEVEL_WIDTH=0;

//Set both the screen and the tempSurface to NULL.
SDL_Surface* screen=NULL;
SDL_Surface* tempSurface=NULL;

//Font that is used for titles.
//Knewave large.
TTF_Font* fontTitle=NULL;
//Font that is used for captions of buttons and other GUI elements.
//Knewave small.
TTF_Font* fontGUI=NULL;
//Font that is used for (long) text.
//Blokletter-Viltstift small.
TTF_Font* fontText=NULL;

//Set the current stateID and the nextState.
int stateID=STATE_NULL;
int nextState=STATE_NULL;

//The name of the current level.
std::string levelName;

//Initialise the camera.
//Start location is 0, size is the same as the screen size.
SDL_Rect camera={0,0,SCREEN_WIDTH,SCREEN_HEIGHT};

//The SDL_Event object.
SDL_Event event;
