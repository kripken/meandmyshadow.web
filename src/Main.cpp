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
#include "Functions.h"
#include "Timer.h"
#include "Objects.h"
#include "Globals.h"
#include "TitleMenu.h"
#include "GUIObject.h"
#include "InputManager.h"
#include "MD5.h"
#include <SDL/SDL.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <string.h>

//Variables for recording.
//To enable picture recording uncomment the next line:
//#define RECORD_PICUTRE_SEQUENCE
#ifdef RECORD_PICUTRE_SEQUENCE
bool recordPictureSequence=false;
int recordPictureIndex=0;
#endif


int main(int argc, char** argv) {
#ifdef _MSC_VER
	//Fix the non-latin file name bug under Visual Studio
	setlocale(LC_ALL,"");
#endif
	//First parse the comand line arguments.
	if(!parseArguments(argc,argv)){
		printf("Usage: %s [OPTIONS] ...\n",argv[0]);
		printf("Avaliable options:\n");
		printf("    %-20s  %s\n","--data-dir <dir>","Specifies the data directory.");
		printf("    %-20s  %s\n","--user-dir <dir>","Specifies the user preferences directory.");
		printf("    %-20s  %s\n","--version","Display the version and quit.");
		printf("    %-20s  %s\n","--help","Display this help.");
		return 0;
	}

	//Initialise some stuff like SDL, the window, SDL_Mixer.
	if(init()==false) {
		fprintf(stderr,"FATAL ERROR: Failed to initalize game.\n");
		return 1;
	}
	//Try to configure the dataPath, userPath, etc...
	if(configurePaths()==false){
		fprintf(stderr,"FATAL ERROR: Failed to configure paths.\n");
		return 1;
	}
	//Load some important files like the background music, default theme.
	if(loadFiles()==false){
		fprintf(stderr,"FATAL ERROR: Failed to load necessary files.\n");
		return 1;
	}
	//Load the settings.
	if(loadSettings()==false){
		fprintf(stderr,"FATAL ERROR: Failed to load config file.\n");
		return 1;
	}
	//Load key config. Then initalize joystick support.
	inputMgr.loadConfig();
	inputMgr.openAllJoysitcks();
	
	//Load the configured music list.
	getMusicManager()->loadMusicList((getDataPath()+"music/"+getSettings()->getValue("musiclist")+".list"));
	getMusicManager()->setMusicList(getSettings()->getValue("musiclist"));
	
	//Set the currentState id to the main menu and create it.
	stateID=STATE_MENU;
	currentState=new Menu();

	//Seed random.
	srand((unsigned)time(NULL));

	//Check if sound is enabled.
	if(getSettings()->getBoolValue("music"))
		getMusicManager()->setEnabled();

	//Check if we should go fullscreen.
	if(getSettings()->getBoolValue("fullscreen"))
		SDL_SetVideoMode(screen->w,screen->h,screen->format->BitsPerPixel, SDL_FULLSCREEN | SDL_HWSURFACE);
	
	tempSurface=SDL_CreateRGBSurface(SDL_HWSURFACE|SDL_SRCALPHA,
		screen->w,screen->h,screen->format->BitsPerPixel,
		screen->format->Rmask,screen->format->Gmask,screen->format->Bmask,0);
	//int fadeIn=0;

  return 0; // The main loop will be called from postRun
}

extern "C" {

#include <emscripten.h>

void __attribute__((used)) OneMainLoopIteration() {
	if (stateID==STATE_EXIT) emscripten_run_script("throw 'game finished'");

	////Start the game loop.
	//while(stateID!=STATE_EXIT){
		//We start the timer.
		FPS.start();

		//Loop the SDL events.
		while(SDL_PollEvent(&event)){
#ifdef RECORD_PICUTRE_SEQUENCE
			if(event.type==SDL_KEYDOWN && event.key.keysym.sym==SDLK_F10){
				recordPictureSequence=!recordPictureSequence;
				printf("Record Picture Sequence %s\n",recordPictureSequence?"ON":"OFF");
			}
#endif
			//Let the input manager handle the events.
			inputMgr.updateState(true);
			//Let the currentState handle the events.
			currentState->handleEvents();
			//Also pass the events to the GUI.
			GUIObjectHandleEvents();
		}

		//update input state (??)
		inputMgr.updateState(false);
		//Now it's time for the state to do his logic.
		currentState->logic();
		
		currentState->render();
		//TODO: Shouldn't the gamestate take care of rendering the GUI?
		if(GUIObjectRoot) GUIObjectRoot->render();
		/*if(fadeIn>0&&fadeIn<255){
			SDL_BlitSurface(screen,NULL,tempSurface,NULL);
			SDL_FillRect(screen,NULL,0);
			SDL_SetAlpha(tempSurface, SDL_SRCALPHA, fadeIn);
			SDL_BlitSurface(tempSurface,NULL,screen,NULL);
			fadeIn+=17;
		}*/
#ifdef RECORD_PICUTRE_SEQUENCE
		if(recordPictureSequence){
			char s[64];
			recordPictureIndex++;
			sprintf(s,"pic%08d.bmp",recordPictureIndex);
			printf("Save screen to %s\n",s);
			SDL_SaveBMP(screen,(getUserPath(USER_CACHE)+s).c_str());
		}
#endif
		SDL_Flip(screen);

		if(nextState!=STATE_NULL){
			//fadeIn=17;
			changeState();
		}

		/*int t=FPS.getTicks();
		t=(1000/g_FPS)-t;
		if(t>0){
			SDL_Delay(t);
		}*/

	//}
}

int __attribute__((used)) FinalCleanup() { // we don't really do this...
	//close all joysticks.
	inputMgr.closeAllJoysticks();
	
	//The game has ended, save the settings just to be sure.
	saveSettings();

	SDL_FreeSurface(tempSurface);

	levels.saveLevelProgress();
	clean();
	
	//End of program.
	return 0;
}

}

