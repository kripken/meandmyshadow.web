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
#include "GameState.h"
#include "Functions.h"
#include "FileManager.h"
#include "Globals.h"
#include "Objects.h"
#include "LevelSelect.h"
#include "GUIObject.h"
#include "GUIListBox.h"
#include "GUIScrollBar.h"
#include "InputManager.h"
#include "Game.h"
#include <SDL/SDL_ttf.h>
#include <SDL/SDL.h>
#include <stdio.h>
#include <string>
#include <sstream>
#include <iostream>
using namespace std;

////////////////////NUMBER////////////////////////
Number::Number(){
	image=NULL;
	number=0;
	medal=0;
	selected=false;
	locked=false;
	
	//Set the default dimensions.
	box.x=0;
	box.y=0;
	box.h=50;
	box.w=50;
	
	//Load the medals image.
	background=loadImage(getDataPath()+"gfx/level.png");
	backgroundLocked=loadImage(getDataPath()+"gfx/levellocked.png");
	medals=loadImage(getDataPath()+"gfx/medals.png");
}

Number::~Number(){
	//We only need to free the SDLSurface.
	if(image) SDL_FreeSurface(image);
}

void Number::init(int number,SDL_Rect box){
	//First set the number and update our status.
	this->number=number;

	//Write our text, number+1 since the counting doens't start with 0, but with 1.
	std::stringstream text;
	number++;
	text<<number;

	//Create the text image.
	SDL_Color black={0,0,0};
	if(image) SDL_FreeSurface(image);
	//Create the text image.
	//Also check which font to use, if the number is higher than 100 use the small font.
	image=TTF_RenderText_Blended(fontGUI,text.str().c_str(),black);

	//Set the new location of the number.
	this->box.x=box.x;
	this->box.y=box.y;
}

void Number::init(std::string text,SDL_Rect box){
	//First set the number and update our status.
	this->number=-1;

	//Create the text image.
	SDL_Color black={0,0,0};
	if(image) SDL_FreeSurface(image);
	image=TTF_RenderText_Blended(fontGUI,text.c_str(),black);

	//Set the new location of the number.
	this->box.x=box.x;
	this->box.y=box.y;
}

void Number::show(int dy){
	//First draw the background, also apply the yOffset(dy).
	if(!locked)
		applySurface(box.x,box.y-dy,background,screen,NULL);
	else
		applySurface(box.x,box.y-dy,backgroundLocked,screen,NULL);
	//Now draw the text image over the background.
	//We draw it centered inside the box.
	applySurface((box.x+25-(image->w/2)),box.y+((TTF_FontAscent(fontGUI)+TTF_FontDescent(fontGUI))/2)-dy,image,screen,NULL);

	//Draw the selection mark.
	if(selected){
		drawGUIBox(box.x,box.y-dy,50,50,screen,0xFFFFFF23);
	}
	
	//Draw the medal.
	if(medal>0){
		SDL_Rect r={(medal-1)*30,0,30,30};
		applySurface(box.x+30,(box.y+30)-dy,medals,screen,&r);
	}
}

void Number::setLocked(bool locked){
	this->locked=locked;
}

void Number::setMedal(int medal){
	this->medal=medal;
}


/////////////////////LEVEL SELECT/////////////////////
LevelSelect::LevelSelect(string titleText){
	//clear the selected level
	selectedNumber=NULL;
	
	//Load the background image.
	background=loadImage(getDataPath()+"gfx/menu/background.png");
	
	//Render the title.
	SDL_Color black={0,0,0};
	title=TTF_RenderText_Blended(fontTitle,titleText.c_str(),black);
	
	//create GUI (test only)
	GUIObject* obj;
	if(GUIObjectRoot){
		delete GUIObjectRoot;
		GUIObjectRoot=NULL;
	}

	GUIObjectRoot=new GUIObject(0,0,800,600);

	//the level select scroll bar
	levelScrollBar=new GUIScrollBar(768,184,16,242,ScrollBarVertical,0,0,0,1,4,true,false);
	GUIObjectRoot->childControls.push_back(levelScrollBar);

	//level pack description
	levelpackDescription=new GUIObject(60,140,800,32,GUIObjectLabel);
	GUIObjectRoot->childControls.push_back(levelpackDescription);

	levelpacks=new GUISingleLineListBox(150,104,500,32);
	levelpacks->name="cmdLvlPack";
	levelpacks->eventCallback=this;
	vector<string> v=enumAllDirs(getDataPath()+"levelpacks/");
	for(vector<string>::iterator i=v.begin(); i!=v.end(); ++i){
		levelpackLocations[*i]=getDataPath()+"levelpacks/"+*i;
	}
	vector<string> v2=enumAllDirs(getUserPath(USER_DATA)+"levelpacks/");
	for(vector<string>::iterator i=v2.begin(); i!=v2.end(); ++i){
		levelpackLocations[*i]=getUserPath(USER_DATA)+"levelpacks/"+*i;
	}
	vector<string> v3=enumAllDirs(getUserPath(USER_DATA)+"custom/levelpacks/");
	for(vector<string>::iterator i=v3.begin(); i!=v3.end(); ++i){
		levelpackLocations[*i]=getUserPath(USER_DATA)+"custom/levelpacks/"+*i;
	}
	v.insert(v.end(),v2.begin(),v2.end());
	v.insert(v.end(),v3.begin(),v3.end());
	
	//Now we add a special levelpack that will contain the levels not in a levelpack.
	v.push_back("Levels");
	
	levelpacks->item=v;
	levelpacks->value=0;

	//Check if we can find the lastlevelpack.
	for(vector<string>::iterator i=v.begin(); i!=v.end(); ++i){
		if(*i==getSettings()->getValue("lastlevelpack")){
			levelpacks->value=i-v.begin();
		}
	}

	//Get the name of the selected levelpack.
	string levelpackName=levelpacks->item[levelpacks->value];
	string s1=getUserPath(USER_DATA)+"progress/"+levelpackName+".progress";
	
	//Check if this is the special Levels levelpack.
	if(levelpackName=="Levels"){
		//Clear the current levels.
		levels.clear();
		levels.setCurrentLevel(0);
		
		//List the custom levels and add them one for one.
		vector<string> v=enumAllFiles(getUserPath(USER_DATA)+"custom/levels/");
		for(vector<string>::iterator i=v.begin(); i!=v.end(); ++i){
			levels.addLevel(getUserPath(USER_DATA)+"custom/levels/"+*i);
			levels.setLocked(levels.getLevelCount()-1);
		}
		//List the addon levels and add them one for one.
		v=enumAllFiles(getUserPath(USER_DATA)+"levels/");
		for(vector<string>::iterator i=v.begin(); i!=v.end(); ++i){
			levels.addLevel(getUserPath(USER_DATA)+"levels/"+*i);
			levels.setLocked(levels.getLevelCount()-1);
		}
	}else{
		//This isn't so load the levelpack in the normal way.
		if(!levels.loadLevels(levelpackLocations[levelpackName]+"/levels.lst")){
			msgBox("Can't load level pack:\n"+levelpackName,MsgBoxOKOnly,"Error");
		}
	}
	//Load the progress.
	levels.loadProgress(s1);
	
	//And add the levelpack single line listbox to the GUIObjectRoot.
	GUIObjectRoot->childControls.push_back(levelpacks);
	
	obj=new GUIObject(20,20,100,32,GUIObjectButton,"Back");
	obj->name="cmdBack";
	obj->eventCallback=this;
	GUIObjectRoot->childControls.push_back(obj);
}

LevelSelect::~LevelSelect(){
	if(GUIObjectRoot){
		delete GUIObjectRoot;
		GUIObjectRoot=NULL;
	}
	levelScrollBar=NULL;
	levelpackDescription=NULL;
	
	selectedNumber=NULL;
	
	//Free the rendered title surface.
	SDL_FreeSurface(title);
}

void LevelSelect::handleEvents(){
	//Check for an SDL_QUIT event.
	if(event.type==SDL_QUIT){
		setNextState(STATE_EXIT);
	}
	
	//Check for a mouse click.
	if(event.type==SDL_MOUSEBUTTONUP && event.button.button==SDL_BUTTON_LEFT){
		checkMouse();
	}
	
	//Check if escape is pressed.
	if(inputMgr.isKeyUpEvent(INPUTMGR_ESCAPE)){
		setNextState(STATE_MENU);
	}
	
	//Check for scrolling down and up.
	if(event.type==SDL_MOUSEBUTTONDOWN && event.button.button==SDL_BUTTON_WHEELDOWN && levelScrollBar){
		if(levelScrollBar->value<levelScrollBar->maxValue) levelScrollBar->value++;
		return;
	}else if(event.type==SDL_MOUSEBUTTONDOWN && event.button.button==SDL_BUTTON_WHEELUP && levelScrollBar){
		if(levelScrollBar->value>0) levelScrollBar->value--;
		return;
	}
}

void LevelSelect::checkMouse(){
	int x,y,dy=0,m=numbers.size();
	
	//Get the current mouse location.
	SDL_GetMouseState(&x,&y);

	//Check if there's a scrollbar, if so get the value.
	if(levelScrollBar)
		dy=levelScrollBar->value;
	//Upper bound of levels we'd like to display.
	if(m>dy*10+LEVELS_DISPLAYED_IN_SCREEN)
		m=dy*10+LEVELS_DISPLAYED_IN_SCREEN;
	y+=dy*64;

	SDL_Rect mouse={x,y,0,0};

	for(int n=dy*10; n<m; n++){
		if(!numbers[n].getLocked()){
			if(checkCollision(mouse,numbers[n].box)==true){
				if(numbers[n].selected){
					selectNumber(n,true);
				}else{
					//Select current level
					for(int i=0;i<levels.getLevelCount();i++){
						numbers[i].selected=(i==n);
					}
					selectNumber(n,false);
				}
				break;
			}
		}
	}
}

void LevelSelect::logic(){}

void LevelSelect::render(){
	int x,y,dy=0,m=numbers.size();
	int idx=-1;
	
	//Get the current mouse location.
	SDL_GetMouseState(&x,&y);

	if(levelScrollBar)
		dy=levelScrollBar->value;
	//Upper bound of levels we'd like to display.
	if(m>dy*10+LEVELS_DISPLAYED_IN_SCREEN)
		m=dy*10+LEVELS_DISPLAYED_IN_SCREEN;
	y+=dy*64;

	SDL_Rect mouse={x,y,0,0};

	//Draw the background.
	applySurface(0,0,background,screen,NULL);
	//Draw the title.
	applySurface((800-title->w)/2,40,title,screen,NULL);
	
	//Loop through the level blocks and draw them.
	for(int n=dy*10;n<m;n++){
		numbers[n].show(dy*64);
		if(numbers[n].getLocked()==false && checkCollision(mouse,numbers[n].box)==true)
			idx=n;
	}
	
	//Show the tool tip text.
	if(idx>=0){
		renderTooltip(idx,dy);
	}
}

void LevelSelect::GUIEventCallback_OnEvent(std::string name,GUIObject* obj,int eventType){
	string s;
	if(name=="cmdLvlPack"){
		s=levelpackLocations[((GUISingleLineListBox*)obj)->item[obj->value]];
		getSettings()->setValue("lastlevelpack",((GUISingleLineListBox*)obj)->item[obj->value]);
	}else if(name=="cmdBack"){
		setNextState(STATE_MENU);
		return;
	}else{
		return;
	}

	//new: reset the level list scroll bar
	if(levelScrollBar)
		levelScrollBar->value=0;

	string s1=getUserPath(USER_DATA)+"progress/"+((GUISingleLineListBox*)obj)->item[obj->value]+".progress";
	
	//Check if this is the special Levels levelpack.
	if(((GUISingleLineListBox*)obj)->item[obj->value]=="Levels"){
		//Clear the current levels.
		levels.clear();
		levels.setCurrentLevel(0);
		
		//List the custom levels and add them one for one.
		vector<string> v=enumAllFiles(getUserPath(USER_DATA)+"custom/levels/");
		for(vector<string>::iterator i=v.begin(); i!=v.end(); ++i){
			levels.addLevel(getUserPath(USER_DATA)+"custom/levels/"+*i);
			levels.setLocked(levels.getLevelCount()-1);
		}
		//List the addon levels and add them one for one.
		v=enumAllFiles(getUserPath(USER_DATA)+"levels/");
		for(vector<string>::iterator i=v.begin(); i!=v.end(); ++i){
			levels.addLevel(getUserPath(USER_DATA)+"levels/"+*i);
			levels.setLocked(levels.getLevelCount()-1);
		}
	}else{
		//This isn't so load the levelpack in the normal way.
		if(!levels.loadLevels(levelpackLocations[((GUISingleLineListBox*)obj)->item[obj->value]]+"/levels.lst")){
			msgBox("Can't load level pack:\n"+((GUISingleLineListBox*)obj)->item[obj->value],MsgBoxOKOnly,"Error");
		}
	}
	//Load the progress file.
	levels.loadProgress(s1);
	
	//And refresh the numbers.
	refresh();
}
