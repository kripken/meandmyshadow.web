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

#include <stdio.h>
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h> 
#include <SDL/SDL_gfxPrimitives.h>
#include <string>
#include "Globals.h"
#include "Functions.h"
#include "FileManager.h"
#include "Objects.h"
#include "Player.h"
#include "GameObjects.h"
#include "Levels.h"
#include "TitleMenu.h"
#include "LevelEditSelect.h"
#include "LevelEditor.h"
#include "Game.h"
#include "LevelPlaySelect.h"
#include "Addons.h"
#include "ImageManager.h"
#include "MusicManager.h"
#include "ThemeManager.h"
#include "GUIListBox.h"
using namespace std;

#ifdef WIN32
#include <windows.h>
#include <shlobj.h>
#else
#include <strings.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#endif

//Initialise the imagemanager.
//The ImageManager is used to prevent loading images multiple times.
ImageManager imageManager;

//Initialise the musicManager.
//The MusicManager is used to prevent loading music files multiple times and for playing/fading music.
MusicManager musicManager;

//Pointer to the settings object.
//It is used to load and save the settings file and change the settings.
Settings* settings=0;


SDL_Surface* loadImage(string file){
	//We use the imageManager to load the file.
	return imageManager.loadImage(file);
}

void applySurface(int x,int y,SDL_Surface* source,SDL_Surface* dest,SDL_Rect* clip){
	//The offset is needed to draw at the right location.
	SDL_Rect offset;
	offset.x=x;
	offset.y=y;
	
	//Let SDL do the drawing of the surface.
	SDL_BlitSurface(source,clip,dest,&offset);
}

void drawRect(int x,int y,int w,int h,SDL_Surface* dest,Uint32 color){
	//NOTE: We let SDL_gfx render it.
	rectangleRGBA(dest,x,y,x+w,y+h,color >> 24,color >> 16,color >> 8,255);
}

//Draw a box with anti-aliased borders using SDL_gfx.
void drawGUIBox(int x,int y,int w,int h,SDL_Surface* dest,Uint32 color){
	//Fill content's background color from function parameter
	boxRGBA(screen,x+1,y+1,x+w-2,y+h-2,color >> 24,color >> 16,color >> 8,color >> 0);

	//Draw first black borders around content and leave 1 pixel in every corner
	lineRGBA(screen,x+1,y,x+w-2,y,0,0,0,255);
	lineRGBA(screen,x+1,y+h-1,x+w-2,y+h-1,0,0,0,255);
	lineRGBA(screen,x,y+1,x,y+h-2,0,0,0,255);
	lineRGBA(screen,x+w-1,y+1,x+w-1,y+h-2,0,0,0,255);
	
	//Fill the corners with transperent color to create anti-aliased borders
	pixelRGBA(screen,x,y,0,0,0,160);
	pixelRGBA(screen,x,y+h-1,0,0,0,160);
	pixelRGBA(screen,x+w-1,y,0,0,0,160);
	pixelRGBA(screen,x+w-1,y+h-1,0,0,0,160);

	//Draw second lighter border around content
	rectangleRGBA(screen,x+1,y+1,x+w-2,y+h-2,0,0,0,64);
	
	//Create anti-aliasing in corners of second border
	pixelRGBA(screen,x+1,y+1,0,0,0,50);
	pixelRGBA(screen,x+1,y+h-2,0,0,0,50);
	pixelRGBA(screen,x+w-2,y+1,0,0,0,50);
	pixelRGBA(screen,x+w-2,y+h-2,0,0,0,50);
}

void drawLine(int x1,int y1,int x2,int y2,SDL_Surface* dest,Uint32 color){
	//NOTE: We let SDL_gfx render it.
	lineRGBA(dest,x1,y1,x2,y2,color >> 24,color >> 16,color >> 8,255);
}

void drawLineWithArrow(int x1,int y1,int x2,int y2,SDL_Surface* dest,Uint32 color,int spacing,int offset,int xsize,int ysize){
	//Draw line first
	drawLine(x1,y1,x2,y2,dest,color);

	//calc delta and length
	double dx=x2-x1;
	double dy=y2-y1;
	double length=sqrt(dx*dx+dy*dy);
	if(length<0.001) return;

	//calc the unit vector
	dx/=length; dy/=length;

	//Now draw arrows on it
	for(double p=offset;p<length;p+=spacing){
		drawLine(int(x1+p*dx+0.5),int(y1+p*dy+0.5),
			int(x1+(p-xsize)*dx-ysize*dy+0.5),int(y1+(p-xsize)*dy+ysize*dx+0.5),dest,color);
		drawLine(int(x1+p*dx+0.5),int(y1+p*dy+0.5),
			int(x1+(p-xsize)*dx+ysize*dy+0.5),int(y1+(p-xsize)*dy-ysize*dx+0.5),dest,color);
	}
}

bool init(){
	//Initialze SDL.
	if(SDL_Init(SDL_INIT_EVERYTHING)==-1) {
		fprintf(stderr,"FATAL ERROR: SDL_Init failed\n");
		return false;
	}

	//Initialze SDL_mixer (audio).
	if(Mix_OpenAudio(22050,MIX_DEFAULT_FORMAT,2,512)==-1){
		fprintf(stderr,"FATAL ERROR: Mix_OpenAudio failed\n");
		return false;
	}

	//Initialze SDL_ttf (fonts).
	if(TTF_Init()==-1){
		fprintf(stderr,"FATAL ERROR: TTF_Init failed\n");
		return false;
	}

	//Initialise the screen.
	screen=SDL_SetVideoMode(SCREEN_WIDTH,SCREEN_HEIGHT,SCREEN_BPP,SDL_HWSURFACE | SDL_DOUBLEBUF /*|SDL_FULLSCREEN*/ );
	if(screen==NULL){
		fprintf(stderr,"FATAL ERROR: SDL_SetVideoMode failed\n");
		return false;
	}

	//Set the the window caption.
	SDL_WM_SetCaption(("Me and my shadow "+version).c_str(),NULL);
	SDL_EnableUNICODE(1);

	//Create the types of blocks.
	for(int i=0;i<TYPE_MAX;i++){
		Game::blockNameMap[Game::blockName[i]]=i;
	}

	//Nothing went wrong so we return true.
	return true;
}

bool loadFiles(){
	//Load the music and play it.
	if(musicManager.loadMusic((getDataPath()+"music/menu.music")).empty()){
		printf("WARNING: Unable to load background music! \n");
	}
	musicManager.playMusic("menu",false);
	//Always load the default music list for fallback.
	musicManager.loadMusicList((getDataPath()+"music/default.list"));
	
	//Load the fonts.
	fontTitle=TTF_OpenFont((getDataPath()+"font/knewave.ttf").c_str(),55);
	fontGUI=TTF_OpenFont((getDataPath()+"font/knewave.ttf").c_str(),32);
	fontText=TTF_OpenFont((getDataPath()+"font/Blokletters-Viltstift.ttf").c_str(),16);
	if(fontTitle==NULL || fontGUI==NULL || fontText==NULL){
		printf("ERROR: Unable to load fonts! \n");
		return false;
	}

	//Load the default theme.
	if(objThemes.appendThemeFromFile(getDataPath()+"themes/Cloudscape/theme.mnmstheme")==NULL){
		printf("ERROR: Can't load default theme file\n");
		return false;
	}
	
	//Nothing failed so return true.
	return true;
}

bool loadSettings(){
	settings=new Settings(getUserPath(USER_CONFIG)+"meandmyshadow.cfg");
	settings->parseFile();
  
	//Always return true?
	return true;
}

bool saveSettings(){
	settings->save();

	//Always return true?
	return true;
}

Settings* getSettings(){
	return settings;
}

MusicManager* getMusicManager(){
	return &musicManager;
}

void clean(){
	//We delete the settings.
	if(settings){
		delete settings;
		settings=NULL;
	}

	//Get rid of the currentstate/
	//NOTE: The state is probably already deleted by the changeState function.
	if(currentState)
		delete currentState;

	//Destroy the GUI if present.
	if(GUIObjectRoot){
		delete GUIObjectRoot;
		GUIObjectRoot=NULL;
	}
	
	//Destroy the imageManager.
	imageManager.destroy();
	
	//Destroy the musicManager.
	musicManager.destroy();
	
	//Close the fonts and quit SDL_ttf.
	TTF_CloseFont(fontTitle);
	TTF_CloseFont(fontGUI);
	TTF_CloseFont(fontText);
	TTF_Quit();
	
	//Quit SDL.
	SDL_Quit();
	
	//And finally stop audio.
	Mix_CloseAudio();
}

void setNextState(int newstate){
	//Only change the state when we aren't already exiting.
	if(nextState!=STATE_EXIT){
		nextState=newstate;
	}
}

void changeState(){
	//Check if there's a nextState.
	if(nextState!=STATE_NULL){
		//Delete the currentState.
		delete currentState;
		currentState=NULL;

		//Set the currentState to the nextState.
		stateID=nextState;
		nextState=STATE_NULL;

		//Init the state.
		switch(stateID){
		case STATE_GAME:
			currentState=new Game();
			break;
		case STATE_MENU:
			levels.clear();
			currentState=new Menu();
			break;
		case STATE_LEVEL_SELECT:
			currentState=new LevelPlaySelect();
			break;
		case STATE_LEVEL_EDIT_SELECT:
			currentState=new LevelEditSelect();
			break;
		case STATE_LEVEL_EDITOR:
			currentState=new LevelEditor();
			break;
		case STATE_OPTIONS:
			currentState=new Options();
			break;
		case STATE_ADDONS:
			currentState=new Addons();
			break;  
		}
		//NOTE: STATE_EXIT isn't mentioned, meaning that currentState is null.
		//This way the game loop will break and the program will exit.
		
		//Fade out.
		int fade=255;
		SDL_BlitSurface(screen,NULL,tempSurface,NULL);
		while(fade>0){
			fade-=17;
			if(fade<0)
				fade=0;
			
			SDL_FillRect(screen,NULL,0);
			SDL_SetAlpha(tempSurface, SDL_SRCALPHA, fade);
			SDL_BlitSurface(tempSurface,NULL,screen,NULL);
			SDL_Flip(screen);
			SDL_Delay(25);
		}
	}
}

void musicStoppedHook(){
	//We just call the musicStopped method of the MusicManager.
	musicManager.musicStopped();
}

bool checkCollision(const SDL_Rect& a,const SDL_Rect& b){
	//Check if the left side of box a isn't past the right side of b.
	if(a.x>=b.x+b.w){
		return false;
	}
	//Check if the right side of box a isn't left of the left side of b.
	if(a.x+a.w<=b.x){
		return false;
	}
	//Check if the top side of box a isn't under the bottom side of b.
	if(a.y>=b.y+b.h){
		return false;
	}
	//Check if the bottom side of box a isn't above the top side of b.
	if(a.y+a.h<=b.y){
		return false;
	}

	//We have collision.
	return true;
}

void setCamera(){
	//SetCamera only works in the Level editor.
	if(stateID==STATE_LEVEL_EDITOR){
		//Get the mouse coordinates.
		int x,y;
		SDL_GetMouseState(&x,&y);
		
		//Make sure we avoid the toolbar.
		SDL_Rect mouse={x,y,0,0};
		SDL_Rect toolbar={155,550,510,50};
		if(checkCollision(mouse,toolbar))
			return;

		//Check if the mouse is near the left edge of the screen.
		//Else check if the mouse is near the right edge.
		if(x<50){
			//We're near the left edge so move the camera.
			camera.x-=5;
		}else if(x>SCREEN_WIDTH-50){
			//We're near the right edge so move the camera.
			camera.x+=5;
		}

		//Check if the mouse is near the top edge of the screen.
		//Else check if the mouse is near the bottom edge.
		if(y<50){
			//We're near the top edge so move the camera.
			camera.y-=5;
		}else if(y>SCREEN_HEIGHT-50){
			//We're near the bottom edge so move the camera.
			camera.y+=5;
		}
	}
}

bool parseArguments(int argc, char** argv){
	//Loop through all arguments.
	//We start at one since 0 is the command itself.
	for(int i=1;i<argc;i++){
		string argument=argv[i];
		
		//Check if the argument is the data-dir.
		if(argument=="--data-dir"){
			//We need a second argument so we increase i.
			i++;
			if(i>=argc){
				printf("ERROR: Missing argument for command '%s'\n\n",argument.c_str());
				return false;
			}
			
			//Configure the dataPath with the given path.
			dataPath=argv[i];
			if(!getDataPath().empty()){
				char c=dataPath[dataPath.size()-1];
				if(c!='/'&&c!='\\') dataPath+="/";
			}
		}else if(argument=="--user-dir"){
			//We need a second argument so we increase i.
			i++;
			if(i>=argc){
				printf("ERROR: Missing argument for command '%s'\n\n",argument.c_str());
				return false;
			}
			
			//Configure the userPath with the given path.
			userPath=argv[i];
			if(!userPath.empty()){
				char c=userPath[userPath.size()-1];
				if(c!='/'&&c!='\\') userPath+="/";
			}
		}else if(argument=="-v" || argument=="-version" || argument=="--version"){
			//Print the version.
			printf("Version: '%s'\n\n",version.c_str());
		}else if(argument=="-h" || argument=="-help" || argument=="--help"){
			//If the help is requested we'll return false without printing an error.
			//This way the usage/help text will be printed.
			return false;
		}else{
			//Any other argument is unknow so we return false.
			printf("ERROR: Unknown argument %s\n\n",argument.c_str());
			return false;
		}
	}
	
	//If everything went well we can return true.
	return true;
}

//Special structure that will recieve the GUIEventCallbacks of the messagebox.
struct msgBoxHandler:public GUIEventCallback{
public:
	//Integer containing the ret(urn) value of the messageBox.
	int ret;
public:
	//Constructor.
	msgBoxHandler():ret(0){}
	
	void GUIEventCallback_OnEvent(std::string name,GUIObject* obj,int eventType){
		//Make sure it's a click event.
		if(eventType==GUIEventClick){
			//Set the return value.
			ret=obj->value;
			
			//After a click event we can delete the GUI.
			if(GUIObjectRoot){
				delete GUIObjectRoot;
				GUIObjectRoot=NULL;
			}
		}
	}
};

msgBoxResult msgBox(string prompt,msgBoxButtons buttons,const string& title){
	//Create the event handler.
	msgBoxHandler objHandler;
	//The GUI objects.
	GUIObject* obj;
	//We keep a pointer to the original GUIObjectRoot for later.
	GUIObject* tmp=GUIObjectRoot;
	
	//Create the GUIObjectRoot, the height and y location is temp.
	//It depends on the content what it will be.
	GUIObjectRoot=new GUIObject(100,200,600,200,GUIObjectFrame,title.c_str());
	
	//Integer containing the current y location used to grow dynamic depending on the content.
	int y=50;
	
	//Now process the prompt.
	{
		//Pointer to the string.
		char* lps=(char*)prompt.c_str();
		//Pointer to a character.
		char* lp=NULL;
		
		//We keep looping forever.
		//The only way out is with the break statement.
		for(;;){
			//As long as it's still the same sentence we continue.
			//It will stop when there's a newline or end of line.
			for(lp=lps;*lp!='\n'&&*lp!='\r'&&*lp!=0;lp++);
			
			//Store the character we stopped on. (End or newline)
			char c=*lp;
			//Set the character in the string to 0, making lps a string containing one sentence.
			*lp=0;
			
			//Integer used to center the sentence horizontally.
			int x;
			TTF_SizeText(fontText,lps,&x,NULL);
			x=(600-x)/2;
			
			//Add a GUIObjectLabel with the sentence.
			GUIObjectRoot->childControls.push_back(new GUIObject(x,y,584,25,GUIObjectLabel,lps));
			//Increase y with 25, about the height of the text.
			y+=25;
			
			//Check the stored character if it was a stop.
			if(c==0){
				//It was so break out of the for loop.
				lps=lp;
				break;
			}
			//It wasn't meaning more will follow.
			//We set lps to point after the "newline" forming a new string.
			lps=lp+1;
		}
	}
	//Add 70 to y to leave some space between the content and the buttons.
	y+=70;
	//Recalc the size of the message box.
	GUIObjectRoot->top=(SCREEN_HEIGHT-y)/2;
	GUIObjectRoot->height=y;
	
	//Now we need to add the buttons.
	//Integer containing the number of buttons to add.
	int count=0;
	//Array with the return codes for the buttons.
	int value[3]={0};
	//Array containing the captation for the buttons.
	string button[3]={"","",""};
	switch(buttons){
	case MsgBoxOKCancel:
		count=2;
		button[0]="OK";value[0]=MsgBoxOK;
		button[1]="Cancel";value[1]=MsgBoxCancel;
		break;
	case MsgBoxAbortRetryIgnore:
		count=3;
		button[0]="Abort";value[0]=MsgBoxAbort;
		button[1]="Retry";value[1]=MsgBoxRetry;
		button[2]="Ignore";value[2]=MsgBoxIgnore;
		break;
	case MsgBoxYesNoCancel:
		count=3;
		button[0]="Yes";value[0]=MsgBoxYes;
		button[1]="No";value[1]=MsgBoxNo;
		button[2]="Cancel";value[2]=MsgBoxCancel;
		break;
	case MsgBoxYesNo:
		count=2;
		button[0]="Yes";value[0]=MsgBoxYes;
		button[1]="No";value[1]=MsgBoxNo;
		break;
	case MsgBoxRetryCancel:
		count=2;
		button[0]="Retry";value[0]=MsgBoxRetry;
		button[1]="Cancel";value[1]=MsgBoxCancel;
		break;
	default:
		count=1;
		button[0]="OK";value[0]=MsgBoxOK;
		break;
	}
	
	//Now we start making the buttons.
	{
		//Calculate the x location (centered).
		int x=302-count*50;
		//Reduce y so that the buttons fit inside the frame.
		y-=40;
		
		//Loop to add the buttons.
		for(int i=0;i<count;i++,x+=100){
			obj=new GUIObject(x,y,96,36,GUIObjectButton,button[i].c_str(),value[i]);
			obj->eventCallback=&objHandler;
			GUIObjectRoot->childControls.push_back(obj);
		}
	}
	
	//Now we dim the screen and keep the GUI rendering/updating.
	SDL_FillRect(tempSurface,NULL,0);
	SDL_SetAlpha(tempSurface,SDL_SRCALPHA,155);
	SDL_BlitSurface(tempSurface,NULL,screen,NULL);
	while(GUIObjectRoot){
		while(SDL_PollEvent(&event)){
			GUIObjectHandleEvents(true);
			
			//Also check for the return, escape or backspace button.
			//escape = KEYUP.
			//backspace and return = KEYDOWN.
			if(count==1 && ((event.type==SDL_KEYUP && event.key.keysym.sym==SDLK_ESCAPE) ||
				(event.type==SDL_KEYDOWN && (event.key.keysym.sym==SDLK_RETURN || event.key.keysym.sym==SDLK_BACKSPACE)))){
				delete GUIObjectRoot;
				GUIObjectRoot=NULL;
			}
			
		}
		//Render the gui.
		if(GUIObjectRoot)
			GUIObjectRoot->render();
		SDL_Flip(screen);
		SDL_Delay(30);
	}
	
	//We're done so set the original GUIObjectRoot back.
	GUIObjectRoot=tmp;
	//And return the result.
	return (msgBoxResult)objHandler.ret;
}

struct fileDialogHandler:public GUIEventCallback{
public:
	//The ret(urn) value, true=ok and false=cancel
	bool ret;
	//Boolean if it's a save dialog.
	bool isSave;
	//Boolean if the file should be verified.
	bool verifyFile;
	//Boolean if files should be listed instead of directories.
	bool files;
	
	//Pointer to the textfield containing the filename.
	GUIObject* txtName;
	//Pointer to the listbox containing the different files.
	GUIListBox* lstFile;
	
	//The extension the files listed should have.
	const char* extension;
	//The current filename.
	string fileName;
	//The current search path.
	string path;
	
	//Vector containing the search paths.
	vector<string> searchPath;
public:
	//Constructor.
	fileDialogHandler(bool isSave=false,bool verifyFile=false, bool files=true):ret(false),
		isSave(isSave),verifyFile(verifyFile),
		files(files),txtName(NULL){}
	
	void GUIEventCallback_OnEvent(std::string name,GUIObject* obj,int eventType){
		//Check for the ok event.
		if(name=="cmdOK"){
			//Get the entered fileName from the text field.
			std::string s=txtName->caption;

			//If it doesn't contain a slash we need to add the path to the fileName.
			if(s.find_first_of("/")==string::npos)
				s=path+s;
			
			//If the string empty we return.
			if(s.empty() || s.find_first_of("*?")!=string::npos)
				return;
			
			//We only need to check for extensions if it isn't a folder dialog.
			if(files){
				//If there isn't right extension add it.
				size_t found=s.find_first_of(".");
				if(found!=string::npos)
					s.replace(s.begin()+found+1,s.end(),extension);
				else if (s.substr(found+1)!=extension)
					s.append(string(".")+extension);
			}
			
			//Check if we should save or load the file.
			//
			if(isSave){
				//Open the file with read permission to check if it already exists.
				FILE* f;
				f=fopen(processFileName(s).c_str(),"rb");
				
				//Check if it exists.
				if(f){
					//Close the file.
					fclose(f);
					
					//Let the currentState render once to prevent multiple GUI overlapping and prevent the screen from going black.
					currentState->render();
					
					//Prompt the user with a Yes or No question.
					if(msgBox(s+" already exists.\nDo you want to overwrite it?",MsgBoxYesNo,"Overwrite Prompt")!=MsgBoxYes){
						//He answered no, so we return.
						return;
					}
				}
				
				//Check if we should verify the file.
				//Verifying only applies to files not to directories.
				if(verifyFile && files){
					//Open the file with write permission.
					f=fopen(processFileName(s).c_str(),"wb");
					
					//Check if their aren't problems.
					if(f){
						//Close the file.
						fclose(f);
					}else{
						//Let the currentState render once to prevent multiple GUI overlapping and prevent the screen from going black.
						currentState->render();
						
						//The file can't be opened so tell the user.
						msgBox("Can't open file "+s+".",MsgBoxOKOnly,"Error");
						return;
					}
				}
			}else if(verifyFile && files){
				//We need to verify a file for opening.
				FILE *f;
				f=fopen(processFileName(s).c_str(),"rb");
				
				//Check if it didn't fail.
				if(f){
					//Succes, so close the file.
					fclose(f);
				}else{
					//Let the currentState render once to prevent multiple GUI overlapping and prevent the screen from going black.
					currentState->render();
					
					//Unable to open file so tell the user.
					msgBox("Can't open file "+s+".",MsgBoxOKOnly,"Error");
					return;
				}
			}
			
			//If we haven't returned then it's fine.
			//Set the fileName to the chosen file.
			fileName=s;
			
			//Delete the GUI.
			if(GUIObjectRoot){
				delete GUIObjectRoot;
				GUIObjectRoot=NULL;
			}
			
			//Set return to true.
			ret=true;
		}else if(name=="cmdCancel"){
			//Cancel means we can kill the gui.
			if(GUIObjectRoot){
				delete GUIObjectRoot;
				GUIObjectRoot=NULL;
			}
		}else if(name=="lstFile"){
			//Get a pointer to the listbox.
			GUIListBox* obj1=lstFile;
			
			//Make sure the option exist and change textfield to it.
			if(obj1!=NULL && txtName!=NULL && obj1->value>=0 && obj1->value<(int)obj1->item.size()){
				txtName->caption=obj1->item[obj1->value];
			}
		}else if(name=="lstSearchIn"){
			//Get the searchpath listbox.
			GUISingleLineListBox *obj1=dynamic_cast<GUISingleLineListBox*>(obj);
			
			//Check if the entry exists.
			if(obj1!=NULL && lstFile!=NULL && obj1->value>=0 && obj1->value<(int)searchPath.size()){
				//Temp string.
				string s;
				
				//Get the new search path.
				path=searchPath[obj1->value];
				
				//Make sure it isn't empty.
				if(!path.empty()){
					//Process the filename.
					s=processFileName(path);
				}else{
					//It's empty so we give the userpath.
					s=getUserPath();
				}
				
				//Fill the list with files or directories.
				if(files) {
					lstFile->item=enumAllFiles(s,extension);
				}else
					lstFile->item=enumAllDirs(s);
				
				//Remove any selection from the list.
				lstFile->value=-1;
			}
		}
	}
};

bool fileDialog(string& fileName,const char* title,const char* extension,const char* path,bool isSave,bool verifyFile,bool files){
	//Pointer to GUIObject to make the GUI with.
	GUIObject* obj;
	//Pointer to the current GUIObjectRoot.
	//We keep it so we can put it back after closing the fileDialog.
	GUIObject* tmp=GUIObjectRoot;
	
	//Create the fileDialogHandler, used for event handling.
	fileDialogHandler objHandler(isSave,verifyFile,files);
	//Vector containing the pathNames.
	vector<string> pathNames;

	//Set the extension of the objHandler.
	objHandler.extension=extension;
	
	//We now need to splits the given path into multiple path names.
	if(path && path[0]){
		//The string isn't empty.
		//Pointer to the paths string.
		char* lp=(char*)path;
		//Pointer to the first newline.
		char* lps=strchr(lp,'\n');
		//Pointer used for checking if their's another newline.
		//It will indicate if it's the last set or not.
		char* lpe;
		
		//Check for a newline.
		if(lps){
			//We have newline(s) so loop forever.
			//We can only break out of the loop when the string ends.
			for(;;){
				//Add the first searchpath.
				//This is the beginning of the string (lp) to the first newline. (lps)
				objHandler.searchPath.push_back(string(lp,lps-lp));
				
				//We should have another newline so search for it.
				lpe=strchr(lps+1,'\n');
				if(lpe){
					//We found it so we add that to the pathname.
					pathNames.push_back(string(lps+1,lpe-lps-1));
					//And start over again by setting lp to the start of a new set of searchPath/pathName.
					lp=lpe+1;
				}else{
					//There is no newline anymore, meaning the last entry, the rest of the string must be the pathName.
					pathNames.push_back(string(lps+1));
					//And break out of the loop.
					break;
				}
				
				//We haven't broken out so search for a newline.
				lps=strchr(lp,'\n');
				//If there isn't a newline break.
				if(!lps) 
					break;
			}
		}else{
			//There is no newline thus the whole string is the searchPath.
			objHandler.searchPath.push_back(path);
		}
	}else{
		//Empty so put an empty string as searchPath.
		objHandler.searchPath.push_back(string());
	}
	
	//It's time to create the GUI.
	//If there are more than one pathNames we need to add a GUISingleLineListBox.
	int base_y=pathNames.size()>0?60:20;
	
	//Create the frame.
	GUIObjectRoot=new GUIObject(100,100-base_y/2,600,400+base_y,GUIObjectFrame,title?title:(isSave?"Save File":"Load File"));
	
	//Create the search path list box if needed.
	if(pathNames.size()>0){
		GUIObjectRoot->childControls.push_back(new GUIObject(8,40,184,36,GUIObjectLabel,"Search In"));
		GUISingleLineListBox* obj1=new GUISingleLineListBox(160,40,432,36);
		obj1->item=pathNames;
		obj1->value=0;
		obj1->name="lstSearchIn";
		obj1->eventCallback=&objHandler;
		GUIObjectRoot->childControls.push_back(obj1);
	}
	
	//Add the FileName label and textfield.
	GUIObjectRoot->childControls.push_back(new GUIObject(8,20+base_y,184,36,GUIObjectLabel,"File Name"));
	{
		//Fill the textbox with the given fileName.
		string s=fileName;
		
		if(!isSave){
			//But only if it isn't empty.
			if(s.empty() && extension && extension[0])
				s=string("*.")+string(extension);
		}
		
		//Create the textbox and add it to the GUI.
		objHandler.txtName=new GUIObject(160,20+base_y,432,36,GUIObjectTextBox,s.c_str());
		GUIObjectRoot->childControls.push_back(objHandler.txtName);
	}
	
	//Now we add the ListBox containing the files or directories.
	{
		GUIListBox* obj1=new GUIListBox(8,60+base_y,584,292);
		
		//Get the searchPath.
		string s=objHandler.searchPath[0];
		//Make sure it isn't empty.
		if(!s.empty()){
			objHandler.path=s;
			s=processFileName(s);
		}else{
			s=getUserPath();
		}
		
		//Check if we should list files or directories.
		if(files){
			//Fill the list with files.
			obj1->item=enumAllFiles(s,extension);
		}else{
			//Fill the list with directories.
			obj1->item=enumAllDirs(s);
		}
		obj1->name="lstFile";
		obj1->eventCallback=&objHandler;
		GUIObjectRoot->childControls.push_back(obj1);
		objHandler.lstFile=obj1;
	}
	
	//Now create the OK and Cancel buttons.
	obj=new GUIObject(200,360+base_y,192,36,GUIObjectButton,"OK");
	obj->name="cmdOK";
	obj->eventCallback=&objHandler;
	GUIObjectRoot->childControls.push_back(obj);
	obj=new GUIObject(400,360+base_y,192,36,GUIObjectButton,"Cancel");
	obj->name="cmdCancel";
	obj->eventCallback=&objHandler;
	GUIObjectRoot->childControls.push_back(obj);

	//Now we keep rendering and updating the GUI.
	SDL_FillRect(tempSurface,NULL,0);
	SDL_SetAlpha(tempSurface,SDL_SRCALPHA,155);
	SDL_BlitSurface(tempSurface,NULL,screen,NULL);
	while(GUIObjectRoot){
		while(SDL_PollEvent(&event)) 
			GUIObjectHandleEvents(true);
		if(GUIObjectRoot)
			GUIObjectRoot->render();
		SDL_Flip(screen);
		SDL_Delay(30);
	}
	
	//The while loop ended meaning we can restore the previous GUI.
	GUIObjectRoot=tmp;
	
	//Now determine what the return value is (and if there is one).
	if(objHandler.ret) 
		fileName=objHandler.fileName;
	return objHandler.ret;
}
