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
#include "LevelEditSelect.h"
#include "GameState.h"
#include "Functions.h"
#include "FileManager.h"
#include "Globals.h"
#include "Objects.h"
#include "GUIObject.h"
#include "GUIListBox.h"
#include "GUIScrollBar.h"
#include "InputManager.h"
#include "Game.h"
#include <SDL/SDL_ttf.h>
#include <SDL/SDL.h>
#include <stdio.h>
#include <algorithm>
#include <string>
#include <iostream>
using namespace std;

LevelEditSelect::LevelEditSelect():LevelSelect("Map Editor"){
	//The levelpack name text field.
	levelpackName=new GUIObject(280,104,240,32,GUIObjectTextBox);
	levelpackName->eventCallback=this;
	levelpackName->visible=false;
	GUIObjectRoot->childControls.push_back(levelpackName);
	
	//Create the six buttons at the bottom of the screen.
	GUIObject* obj=new GUIObject(20,480,260,32,GUIObjectButton,"New Levelpack");
	obj->name="cmdNewLvlpack";
	obj->eventCallback=this;
	GUIObjectRoot->childControls.push_back(obj);
	
	propertiesPack=new GUIObject(280,480,280,32,GUIObjectButton,"Pack Properties");
	propertiesPack->name="cmdLvlpackProp";
	propertiesPack->eventCallback=this;
	GUIObjectRoot->childControls.push_back(propertiesPack);
	
	removePack=new GUIObject(540,480,240,32,GUIObjectButton,"Remove Pack");
	removePack->name="cmdRmLvlpack";
	removePack->eventCallback=this;
	GUIObjectRoot->childControls.push_back(removePack);
	
	move=new GUIObject(20,540,240,32,GUIObjectButton,"Move Map");
	move->name="cmdMoveMap";
	move->eventCallback=this;
	move->enabled=false;
	GUIObjectRoot->childControls.push_back(move);
	
	remove=new GUIObject(280,540,240,32,GUIObjectButton,"Remove Map");
	remove->name="cmdRmMap";
	remove->eventCallback=this;
	remove->enabled=false;
	GUIObjectRoot->childControls.push_back(remove);
	
	edit=new GUIObject(540,540,240,32,GUIObjectButton,"Edit Map");
	edit->name="cmdEdit";
	edit->eventCallback=this;
	edit->enabled=false;
	GUIObjectRoot->childControls.push_back(edit);
	
	//Set the levelEditGUIObjectRoot.
	levelEditGUIObjectRoot=GUIObjectRoot;
	
	//NOTE: We are changing the available list of levelpacks to prevent editing the main/addons levelpacks.
	listPacks();
	
	//show level list
	refresh();
}

LevelEditSelect::~LevelEditSelect(){
	selectedNumber=NULL;
}

void LevelEditSelect::listPacks(){
	levelpackLocations.clear();
	levelpacks->item.clear();
	levelpacks->value=0;
	
	vector<string> v=enumAllDirs(getUserPath(USER_DATA)+"custom/levelpacks/");
	v.push_back("Levels");
	for(vector<string>::iterator i=v.begin(); i!=v.end(); ++i){
		levelpackLocations[*i]=getUserPath(USER_DATA)+"custom/levelpacks/"+*i;
		
		//Check if we can find the lastlevelpack.
		if(*i==getSettings()->getValue("lastlevelpack")){
			levelpacks->value=i-v.begin();
			packName=*i;
		}
	}
	levelpacks->item=v;
	
	//And call changePack since we changed the levelpack.
	changePack();
}

void LevelEditSelect::changePack(){
	packName=levelpacks->item[levelpacks->value];
	if(packName=="Levels"){
		//Clear the current levels.
		levels.clear();
		levels.setCurrentLevel(0);
		levels.levelpackPath="";
		
		//List the custom levels and add them one for one.
		vector<string> v=enumAllFiles(getUserPath(USER_DATA)+"custom/levels/");
		for(vector<string>::iterator i=v.begin(); i!=v.end(); ++i){
			levels.addLevel(getUserPath(USER_DATA)+"custom/levels/"+*i);
			levels.setLocked(levels.getLevelCount()-1);
		}
		
		//Disable some levelpack buttons.
		propertiesPack->enabled=false;
		removePack->enabled=false;
	}else{
		//Load the levelpack in the normal way.
		if(!levels.loadLevels(levelpackLocations[packName]+"/levels.lst")){
			msgBox("Can't load level pack:\n"+packName,MsgBoxOKOnly,"Error");
		}
		
		//Enable some levelpack buttons.
		propertiesPack->enabled=true;
		removePack->enabled=true;
	}
}

void LevelEditSelect::packProperties(){
	//Open a message popup.
	
	//NOTE: We can always point GUIObjectRoot to the main gui by using levelEditGUIObjectRoot.
	GUIObjectRoot=new GUIObject(100,(SCREEN_HEIGHT-320)/2,600,320,GUIObjectFrame,"Properties");
	GUIObject* obj;
	
	obj=new GUIObject(40,50,240,36,GUIObjectLabel,"Name:");
	GUIObjectRoot->childControls.push_back(obj);
	
	obj=new GUIObject(60,80,480,36,GUIObjectTextBox,packName.c_str());
	obj->name="LvlpackName";
	GUIObjectRoot->childControls.push_back(obj);
	
	obj=new GUIObject(40,120,240,36,GUIObjectLabel,"Description:");
	GUIObjectRoot->childControls.push_back(obj);
	
	obj=new GUIObject(60,150,480,36,GUIObjectTextBox,levels.levelpackDescription.c_str());
	obj->name="LvlpackDescription";
	GUIObjectRoot->childControls.push_back(obj);
	
	obj=new GUIObject(40,190,240,36,GUIObjectLabel,"Congratulation text:");
	GUIObjectRoot->childControls.push_back(obj);
	
	obj=new GUIObject(60,220,480,36,GUIObjectTextBox,levels.congratulationText.c_str());
	obj->name="LvlpackCongratulation";
	GUIObjectRoot->childControls.push_back(obj);
	
	obj=new GUIObject(100,320-44,150,36,GUIObjectButton,"OK");
	obj->name="cfgOK";
	obj->eventCallback=this;
	GUIObjectRoot->childControls.push_back(obj);
	obj=new GUIObject(350,320-44,150,36,GUIObjectButton,"Cancel");
	obj->name="cfgCancel";
	obj->eventCallback=this;
	GUIObjectRoot->childControls.push_back(obj);
	
	//Dim the screen using the tempSurface.
	SDL_FillRect(tempSurface,NULL,0);
	SDL_SetAlpha(tempSurface,SDL_SRCALPHA,155);
	SDL_BlitSurface(tempSurface,NULL,screen,NULL);
	while(GUIObjectRoot){
		while(SDL_PollEvent(&event)) GUIObjectHandleEvents(true);
		if(GUIObjectRoot) GUIObjectRoot->render();
		SDL_Flip(screen);
		SDL_Delay(30);
	}
	
	//We're done so set the original GUIObjectRoot back.
	GUIObjectRoot=levelEditGUIObjectRoot;
}

void LevelEditSelect::addLevel(){
	//Open a message popup.
	
	//NOTE: We can always point GUIObjectRoot to the main gui by using levelEditGUIObjectRoot.
	GUIObjectRoot=new GUIObject(100,(SCREEN_HEIGHT-200)/2,600,200,GUIObjectFrame,"Add level");
	GUIObject* obj;
	
	obj=new GUIObject(40,80,240,36,GUIObjectLabel,"File name:");
	GUIObjectRoot->childControls.push_back(obj);
	
	char s[64];
	sprintf(s,"map%02d.map",levels.getLevelCount()+1);
	obj=new GUIObject(300,80,240,36,GUIObjectTextBox,s);
	obj->name="LvlFile";
	GUIObjectRoot->childControls.push_back(obj);

	obj=new GUIObject(100,200-44,150,36,GUIObjectButton,"OK");
	obj->name="cfgAddOK";
	obj->eventCallback=this;
	GUIObjectRoot->childControls.push_back(obj);
	obj=new GUIObject(350,200-44,150,36,GUIObjectButton,"Cancel");
	obj->name="cfgAddCancel";
	obj->eventCallback=this;
	GUIObjectRoot->childControls.push_back(obj);
	
	//Dim the screen using the tempSurface.
	SDL_FillRect(tempSurface,NULL,0);
	SDL_SetAlpha(tempSurface,SDL_SRCALPHA,155);
	SDL_BlitSurface(tempSurface,NULL,screen,NULL);
	while(GUIObjectRoot){
		while(SDL_PollEvent(&event)) GUIObjectHandleEvents(true);
		if(GUIObjectRoot) GUIObjectRoot->render();
		SDL_Flip(screen);
		SDL_Delay(30);
	}
	
	//We're done so set the original GUIObjectRoot back.
	GUIObjectRoot=levelEditGUIObjectRoot;
}

void LevelEditSelect::moveLevel(){
	//Open a message popup.
	
	//NOTE: We can always point GUIObjectRoot to the main gui by using levelEditGUIObjectRoot.
	GUIObjectRoot=new GUIObject(100,(SCREEN_HEIGHT-200)/2,600,200,GUIObjectFrame,"Move level");
	GUIObject* obj;
	
	obj=new GUIObject(40,80,240,36,GUIObjectLabel,"Level: ");
	GUIObjectRoot->childControls.push_back(obj);
	
	obj=new GUIObject(300,80,240,36,GUIObjectTextBox,"1");
	obj->name="MoveLevel";
	GUIObjectRoot->childControls.push_back(obj);
	
	obj=new GUISingleLineListBox(40,120,240,36);
	obj->name="lstPlacement";
	vector<string> v;
	v.push_back("Before");
	v.push_back("After");
	v.push_back("Swap");
	(dynamic_cast<GUISingleLineListBox*>(obj))->item=v;
	obj->value=0;
	GUIObjectRoot->childControls.push_back(obj);
	
	obj=new GUIObject(100,200-44,150,36,GUIObjectButton,"OK");
	obj->name="cfgMoveOK";
	obj->eventCallback=this;
	GUIObjectRoot->childControls.push_back(obj);
	obj=new GUIObject(350,200-44,150,36,GUIObjectButton,"Cancel");
	obj->name="cfgMoveCancel";
	obj->eventCallback=this;
	GUIObjectRoot->childControls.push_back(obj);
	
	//Dim the screen using the tempSurface.
	SDL_FillRect(tempSurface,NULL,0);
	SDL_SetAlpha(tempSurface,SDL_SRCALPHA,155);
	SDL_BlitSurface(tempSurface,NULL,screen,NULL);
	while(GUIObjectRoot){
		while(SDL_PollEvent(&event)) GUIObjectHandleEvents(true);
		if(GUIObjectRoot) GUIObjectRoot->render();
		SDL_Flip(screen);
		SDL_Delay(30);
	}
	
	//We're done so set the original GUIObjectRoot back.
	GUIObjectRoot=levelEditGUIObjectRoot;
}
void LevelEditSelect::refresh(){
	int m=levels.getLevelCount();
	numbers.clear();
	
	//clear the selected level
	if(selectedNumber!=NULL){
		selectedNumber=NULL;
	}
	
	//Disable the level specific buttons.
	move->enabled=false;
	remove->enabled=false;
	edit->enabled=false;
	
	for(int n=0;n<=m;n++){
		numbers.push_back(Number());
	}
	
	for(int n=0;n<m;n++){
		SDL_Rect box={(n%10)*64+80,(n/10)*64+184,0,0};
		numbers[n].init(n,box);
	}
	SDL_Rect box={(m%10)*64+80,(m/10)*64+184,0,0};
	numbers[m].init("+",box);
	
	m++; //including the "+" button
	if(m>LEVELS_DISPLAYED_IN_SCREEN){
		levelScrollBar->maxValue=(m-LEVELS_DISPLAYED_IN_SCREEN+9)/10;
		levelScrollBar->visible=true;
	}else{
		levelScrollBar->maxValue=0;
		levelScrollBar->visible=false;
	}
	levelpackDescription->caption=levels.levelpackDescription;
	int width,height;
	TTF_SizeText(fontText,levels.levelpackDescription.c_str(),&width,&height);
	levelpackDescription->left=(800-width)/2;
}

void LevelEditSelect::selectNumber(unsigned int number,bool selected){
	if(selected){
		levels.setCurrentLevel(number);
		setNextState(STATE_LEVEL_EDITOR);
		
		//Pick music from the current music list.
		getMusicManager()->pickMusic();
	}else{
		if(number==numbers.size()-1){
			addLevel();
		}else if(number>=0 && number<numbers.size()){
			selectedNumber=&numbers[number];
			
			//Enable the level specific buttons.
			//NOTE: We check if 'remove levelpack' is enabled, if not then it's the Levels levelpack.
			if(removePack->enabled)
				move->enabled=true;
			remove->enabled=true;
			edit->enabled=true;
		}
	}
}

void LevelEditSelect::render(){
	//Let the levelselect render.
	LevelSelect::render();
}

void LevelEditSelect::renderTooltip(unsigned int number,int dy){
	SDL_Color fg={0,0,0};
	SDL_Surface* name;
	
	if(number==(unsigned)levels.getLevelCount()){
		//Render the name of the level.
		name=TTF_RenderText_Blended(fontText,"Add level",fg);
	}else{
		//Render the name of the level.
		name=TTF_RenderText_Blended(fontText,levels.getLevelName(number).c_str(),fg);
	}
	
	//Check if name isn't null.
	if(name==NULL)
		return;
	
	//Now draw a square the size of the three texts combined.
	SDL_Rect r=numbers[number].box;
	r.y-=dy*64;
	r.w=name->w;
	r.h=name->h;
	
	//Make sure the tooltip doesn't go outside the window.
	if(r.y>SCREEN_HEIGHT-200){
		r.y-=name->h+4;
	}else{
		r.y+=numbers[number].box.h+2;
	}
	if(r.x+r.w>SCREEN_WIDTH-50)
		r.x=SCREEN_WIDTH-50-r.w;
	
	//Draw a rectange
	Uint32 color=0xFFFFFF00|240;
	drawGUIBox(r.x-5,r.y-5,r.w+10,r.h+10,screen,color);
	
	//Calc the position to draw.
	SDL_Rect r2=r;
	
	//Now we render the name if the surface isn't null.
	if(name!=NULL){
		//Draw the name.
		SDL_BlitSurface(name,NULL,screen,&r2);
	}
	
	//And free the surfaces.
	SDL_FreeSurface(name);
}

void LevelEditSelect::GUIEventCallback_OnEvent(std::string name,GUIObject* obj,int eventType){
	//NOTE: We check for the levelpack change to enable/disable some levelpack buttons.
	if(name=="cmdLvlPack"){
		//Set the name of the levelpack.
		packName=((GUISingleLineListBox*)obj)->item[obj->value];
		
		//Check if it matches the Levels levelpack.
		if(((GUISingleLineListBox*)obj)->item[obj->value]=="Levels"){
			//Disable some levelpack buttons.
			propertiesPack->enabled=false;
			removePack->enabled=false;
			getSettings()->setValue("lastlevelpack","Levels");
			
			//We call changepack and return to prevent the LevelSelect to undo what we did.
			changePack();
			refresh();
			return;
		}else{
			//Enable some levelpack buttons.
			propertiesPack->enabled=true;
			removePack->enabled=true;
		}
	}
	
	//Let the level select handle his GUI events.
	LevelSelect::GUIEventCallback_OnEvent(name,obj,eventType);
	
	//Check for the edit button.
	if(name=="cmdNewLvlpack"){
		//Clear the current pack.
		packName.clear();
		levels.clear();
		
		//Create a new pack.
		packProperties();
	}else if(name=="cmdLvlpackProp"){
		//Show the pack properties.
		packProperties();
	}else if(name=="cmdRmLvlpack"){
		//Show an "are you sure" message.
		if(msgBox("Are you sure?",MsgBoxYesNo,"Remove prompt")==MsgBoxYes){
			//Remove the directory.
			if(!removeDirectory(levelpackLocations[packName].c_str())){
				cerr<<"ERROR: Unable to remove levelpack directory "<<levelpackLocations[packName]<<endl;
			}
			
			//Remove it from the vector (levelpack list).
			vector<string>::iterator it;
			it=find(levelpacks->item.begin(),levelpacks->item.end(),packName);
			if(it!=levelpacks->item.end()){
				levelpacks->item.erase(it);
			}
			
			//And call changePack.
			levelpacks->value=levelpacks->item.size()-1;
			changePack();
			refresh();
		}
	}else if(name=="cmdMoveMap"){
		if(selectedNumber!=NULL){
			moveLevel();
		}
	}else if(name=="cmdRmMap"){
		if(selectedNumber!=NULL){
			if(packName!="Levels"){
				if(!removeFile((levelpackLocations[packName]+"/"+levels.getLevel(selectedNumber->getNumber())->file).c_str())){
					cerr<<"ERROR: Unable to remove level "<<(levelpackLocations[packName]+"/"+levels.getLevel(selectedNumber->getNumber())->file).c_str()<<endl;
				}
				levels.removeLevel(selectedNumber->getNumber());
				levels.saveLevels(levelpackLocations[packName]+"/levels.lst");
			}else{
				//This is the levels levelpack so we just remove the file.
				if(!removeFile(levels.getLevel(selectedNumber->getNumber())->file.c_str())){
					cerr<<"ERROR: Unable to remove level "<<(levelpackLocations[packName]+"/"+levels.getLevel(selectedNumber->getNumber())->file).c_str()<<endl;
				}
				changePack();
			}
			
			//And refresh the selection screen.
			refresh();
		}
	}else if(name=="cmdEdit"){
		if(selectedNumber!=NULL){
			levels.setCurrentLevel(selectedNumber->getNumber());
			setNextState(STATE_LEVEL_EDITOR);
			
			//Pick music from the current music list.
			getMusicManager()->pickMusic();
		}
	}
	
	//Check for levelpack properties events.
	if(name=="cfgOK"){
		//Now loop throught the children of the GUIObjectRoot in search of the fields.
		for(unsigned int i=0;i<GUIObjectRoot->childControls.size();i++){
			if(GUIObjectRoot->childControls[i]->name=="LvlpackName"){
				//Check if the name changed.
				if(packName!=GUIObjectRoot->childControls[i]->caption){
					//Delete the old one.
					if(!packName.empty()){
						if(!renameDirectory((getUserPath(USER_DATA)+"custom/levelpacks/"+packName).c_str(),(getUserPath(USER_DATA)+"custom/levelpacks/"+GUIObjectRoot->childControls[i]->caption).c_str())){
							cerr<<"ERROR: Unable to move levelpack directory "<<(getUserPath(USER_DATA)+"custom/levelpacks/"+packName)<<" to "<<(getUserPath(USER_DATA)+"custom/levelpacks/"+GUIObjectRoot->childControls[i]->caption)<<endl;
						}
						
						//Also change the levelpack location.
						map<string,string>::iterator it;
						it=levelpackLocations.find(packName);
						
						if(it!=levelpackLocations.end()){
							levelpackLocations.erase(it);
						}
						
						//And the levelpack list.
						vector<string>::iterator it1;
						it1=find(levelpacks->item.begin(),levelpacks->item.end(),packName);
							if(it1!=levelpacks->item.end()){
								levelpacks->item.erase(it1);
							if((unsigned)levelpacks->value>levelpacks->item.size())
								levelpacks->value=levelpacks->item.size()-1;
						}
						
						//Also add the levelpack location
						levelpackLocations[GUIObjectRoot->childControls[i]->caption]=(getUserPath(USER_DATA)+"custom/levelpacks/"+GUIObjectRoot->childControls[i]->caption);
						levelpacks->item.push_back(GUIObjectRoot->childControls[i]->caption);
						levelpacks->value=levelpacks->item.size()-1;
					}else{
						if(!createDirectory((getUserPath(USER_DATA)+"custom/levelpacks/"+GUIObjectRoot->childControls[i]->caption).c_str())){
							cerr<<"ERROR: Unable to create levelpack directory "<<(getUserPath(USER_DATA)+"custom/levelpacks/"+GUIObjectRoot->childControls[i]->caption)<<endl;
						}
						if(!createFile((getUserPath(USER_DATA)+"custom/levelpacks/"+GUIObjectRoot->childControls[i]->caption+"/levels.lst").c_str())){
							cerr<<"ERROR: Unable to create levelpack file "<<(getUserPath(USER_DATA)+"custom/levelpacks/"+GUIObjectRoot->childControls[i]->caption+"/levels.lst")<<endl;
						}
						
						//Also add the levelpack location.
						levelpackLocations[GUIObjectRoot->childControls[i]->caption]=(getUserPath(USER_DATA)+"custom/levelpacks/"+GUIObjectRoot->childControls[i]->caption);
						levelpacks->item.push_back(GUIObjectRoot->childControls[i]->caption);
						levelpacks->value=levelpacks->item.size()-1;
					}
					//And set the new name.
					packName=GUIObjectRoot->childControls[i]->caption;
					changePack();
				}
			}
			if(GUIObjectRoot->childControls[i]->name=="LvlpackDescription"){
				levels.levelpackDescription=GUIObjectRoot->childControls[i]->caption;
			}
			if(GUIObjectRoot->childControls[i]->name=="LvlpackCongratulation"){
				levels.congratulationText=GUIObjectRoot->childControls[i]->caption;
			}
		}
		//Refresh the leveleditselect to show the correct information.
		refresh();
		
		//Save the configuration.
		levels.saveLevels(getUserPath(USER_DATA)+"custom/levelpacks/"+packName+"/levels.lst");
		getSettings()->setValue("lastlevelpack",packName);
		
		//Clear the gui.
		if(GUIObjectRoot){
			delete GUIObjectRoot;
			GUIObjectRoot=NULL;
		}
	}else if(name=="cfgCancel"){
		//Check if packName is empty, if so it was a new levelpack and we need to revert to an existing one.
		if(packName.empty()){
			packName=levelpacks->item[levelpacks->value];
			changePack();
		}
		
		//Clear the gui.
		if(GUIObjectRoot){
			delete GUIObjectRoot;
			GUIObjectRoot=NULL;
		}
	}
	
	//Check for add level events.
	if(name=="cfgAddOK"){
		//Check if the file name isn't null.
		//Now loop throught the children of the GUIObjectRoot in search of the fields.
		for(unsigned int i=0;i<GUIObjectRoot->childControls.size();i++){
			if(GUIObjectRoot->childControls[i]->name=="LvlFile"){
				if(GUIObjectRoot->childControls[i]->caption.empty()){
					msgBox("No file name given for the new level.",MsgBoxOKOnly,"Missing file name");
					return;
				}else{
					string tmp_caption = GUIObjectRoot->childControls[i]->caption;
					
					//Replace all spaces with a underline.
					size_t j;
					for(;(j=tmp_caption.find(" "))!=string::npos;){
						tmp_caption.replace(j,1,"_");
					}
					
					//If there isn't ".map" extension add it.
					size_t found=tmp_caption.find_first_of(".");
					if(found!=string::npos)
						tmp_caption.replace(tmp_caption.begin()+found+1,tmp_caption.end(),"map");
					else if (tmp_caption.substr(found+1)!="map")
						tmp_caption.append(".map");
					
					/* Create path and file in it */
					string path=(levelpackLocations[packName]+"/"+tmp_caption);
					if(packName=="Levels"){
						path=(getUserPath(USER_DATA)+"/custom/levels/"+tmp_caption);
					}
					
					//First check if the file doesn't exist already.
					FILE* f;
					f=fopen(path.c_str(),"rb");
					
					//Check if it exists.
					if(f){
						//Close the file.
						fclose(f);
						
						//Let the currentState render once to prevent multiple GUI overlapping and prevent the screen from going black.
						currentState->render();
						levelEditGUIObjectRoot->render();
						
						//Notify the user.
						msgBox(("The file "+tmp_caption+" already exists.").c_str(),MsgBoxOKOnly,"Error");
						return;
					}
					
					if(!createFile(path.c_str())){
						cerr<<"ERROR: Unable to create level file "<<path<<endl;
					}
					levels.addLevel(path);
					if(packName!="Levels")
						levels.saveLevels(getUserPath(USER_DATA)+"custom/levelpacks/"+packName+"/levels.lst");
					refresh();
					
					//Clear the gui.
					if(GUIObjectRoot){
						delete GUIObjectRoot;
						GUIObjectRoot=NULL;
						return;
					}
				}
			}
		}
	}else if(name=="cfgAddCancel"){
		//Clear the gui.
		if(GUIObjectRoot){
			delete GUIObjectRoot;
			GUIObjectRoot=NULL;
		}
	}
	
	//Check for move level events.
	if(name=="cfgMoveOK"){
		//Check if the entered level number is valid.
		//Now loop throught the children of the GUIObjectRoot in search of the fields.
		int level=0;
		int placement=0;
		for(unsigned int i=0;i<GUIObjectRoot->childControls.size();i++){
			if(GUIObjectRoot->childControls[i]->name=="MoveLevel"){
				level=atoi(GUIObjectRoot->childControls[i]->caption.c_str());
				if(level<=0 || level>levels.getLevelCount()){
					msgBox("The entered level number isn't valid!",MsgBoxOKOnly,"Illegal number");
					return;
				}
			}
			if(GUIObjectRoot->childControls[i]->name=="lstPlacement"){
				placement=GUIObjectRoot->childControls[i]->value;
			}
		}
		
		//Now we execute the swap/move.
		//Check for the place before.
		if(placement==0){
			//We place the selected level before the entered level.
			levels.moveLevel(selectedNumber->getNumber(),level-1);
		}else if(placement==1){
			//We place the selected level after the entered level.
			if(level<selectedNumber->getNumber())
				levels.moveLevel(selectedNumber->getNumber(),level);
			else
				levels.moveLevel(selectedNumber->getNumber(),level+1);
		}else if(placement==2){
			//We swap the selected level with the entered level.
			levels.swapLevel(selectedNumber->getNumber(),level-1);
		}
		
		//And save the change.
		if(packName!="Levels")
			levels.saveLevels(getUserPath(USER_DATA)+"custom/levelpacks/"+packName+"/levels.lst");
			
		refresh();
		
		//Clear the gui.
		if(GUIObjectRoot){
			delete GUIObjectRoot;
			GUIObjectRoot=NULL;
		}
	}else if(name=="cfgMoveCancel"){
		//Clear the gui.
		if(GUIObjectRoot){
			delete GUIObjectRoot;
			GUIObjectRoot=NULL;
		}
	}
}
