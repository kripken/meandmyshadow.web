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
#include "Globals.h"
#include "Functions.h"
#include "FileManager.h"
#include "GameObjects.h"
#include "ThemeManager.h"
#include "Objects.h"
#include "Game.h"
#include "TreeStorageNode.h"
#include "POASerializer.h"
#include "InputManager.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
using namespace std;

const char* Game::blockName[TYPE_MAX]={"Block","PlayerStart","ShadowStart",
"Exit","ShadowBlock","Spikes",
"Checkpoint","Swap","Fragile",
"MovingBlock","MovingShadowBlock","MovingSpikes",
"Teleporter","Button","Switch",
"ConveyorBelt","ShadowConveyorBelt","NotificationBlock",
};

map<string,int> Game::blockNameMap;
string Game::recordFile;

Game::Game(bool loadLevel):isReset(false)
	,currentLevelNode(NULL)
	,customTheme(NULL)
	,background(NULL)
	,won(false)
	,interlevel(false)
	,gameTipIndex(0)
	,time(0),timeSaved(0)
	,recordings(0),recordingsSaved(0)
	,shadowCam(false)
	,player(this),shadow(this),objLastCheckPoint(NULL){
	
	//Reserve the memory for the GameObject tips.
	memset(bmTips,0,sizeof(bmTips));
	
	action=loadImage(getDataPath()+"gfx/actions.png");
	medals=loadImage(getDataPath()+"gfx/medals.png");

	//Check if we should load record file.
	if(!recordFile.empty()){
		loadRecord(recordFile.c_str());
		recordFile.clear();
		return;
	}

	//If we should load the level then load it.
	if(loadLevel){
		this->loadLevel(levels.getLevelpackPath()+levels.getLevelFile());
		levels.saveLevelProgress();
	}
}

Game::~Game(){
	//Simply call our destroy method.
	destroy();
}

void Game::destroy(){
	//Loop through the levelObjects and delete them.
	for(unsigned int i=0;i<levelObjects.size();i++)
		delete levelObjects[i];
	//Done now clear the levelObjects vector.
	levelObjects.clear();
	
	//Clear the name and the editor data.
	levelName.clear();
	levelFile.clear();
	editorData.clear();
	
	//Loop through the tips.
	for(int i=0;i<TYPE_MAX;i++){
		//If it exist free the SDL_Surface.
		if(bmTips[i])
			SDL_FreeSurface(bmTips[i]);
	}
	memset(bmTips,0,sizeof(bmTips));
	
	//Remove everything from the themeManager.
	background=NULL;
	if(customTheme)
		objThemes.removeTheme();
	customTheme=NULL;

	//delete current level (if any)
	if(currentLevelNode){
		delete currentLevelNode;
		currentLevelNode=NULL;
	}
	
	//Reset the time.
	time=timeSaved=0;
	recordings=recordingsSaved=0;
}

void Game::loadLevelFromNode(TreeStorageNode* obj,const string& fileName){
	//Make sure there's nothing left from any previous levels.
	destroy();

	//set current level to loaded one.
	currentLevelNode=obj;
	
	//Temp var used for block locations.
	SDL_Rect box;

	//Set the level dimensions to the default, it will probably be changed by the editorData,
	//but 800x600 is a fallback.
	LEVEL_WIDTH=800;
	LEVEL_HEIGHT=600;

	//Load the additional data.
	for(map<string,vector<string> >::iterator i=obj->attributes.begin();i!=obj->attributes.end();i++){
		if(i->first=="size"){
			//We found the size attribute.
			if(i->second.size()>=2){
				//Set the dimensions of the level.
				LEVEL_WIDTH=atoi(i->second[0].c_str());
				LEVEL_HEIGHT=atoi(i->second[1].c_str());
			}
		}else if(i->second.size()>0){
			//Any other data will be put into the editorData.
			editorData[i->first]=i->second[0];
		}
	}

	//Get the theme.
	{
		//If a theme is configured then load it.
		string theme=processFileName(getSettings()->getValue("theme"));
		
		//Check if it isn't the default theme, because if it is it's already loaded.
		if(fileNameFromPath(theme)!="Cloudscape") {
			customTheme=objThemes.appendThemeFromFile(theme+"/theme.mnmstheme");
		}
			  
		//Check if level themes are enabled.
		if(getSettings()->getBoolValue("leveltheme")) {
			string &s=editorData["theme"];
			if(!s.empty()){
				customTheme=objThemes.appendThemeFromFile(processFileName(theme)+"/theme.mnmstheme");
			}
		}
		
		//Set the Appearance of the player and the shadow.
		objThemes.getCharacter(false)->createInstance(&player.appearance);
		objThemes.getCharacter(true)->createInstance(&shadow.appearance);
	}

	
	for(unsigned int i=0;i<obj->subNodes.size();i++){
		TreeStorageNode* obj1=obj->subNodes[i];
		if(obj1==NULL) continue;
		if(obj1->name=="tile" && obj1->value.size()>=3){
			int objectType=blockNameMap[obj1->value[0]];

			box.x=atoi(obj1->value[1].c_str());
			box.y=atoi(obj1->value[2].c_str());

			map<string,string> obj;

			for(map<string,vector<string> >::iterator i=obj1->attributes.begin();i!=obj1->attributes.end();i++){
				if(i->second.size()>0) obj[i->first]=i->second[0];
			}

			levelObjects.push_back( new Block ( box.x, box.y, objectType, this) );
			levelObjects.back()->setEditorData(obj);
		}
	}

	//Set the levelName to the name of the current level.
	levelName=editorData["name"];
	levelFile=fileName;

	//Some extra stuff only needed when not in the levelEditor.
	if(stateID!=STATE_LEVEL_EDITOR){
		//We create a text with the text "Level <levelno> <levelName>".
		//It will be shown in the left bottom corner of the screen.
		stringstream s;
		if(levels.getLevelCount()>1){
			s<<"Level "<<(levels.getCurrentLevel()+1)<<" ";
		}
		s<<editorData["name"];
		
		SDL_Color fg={0,0,0,0};
		SDL_Color bg={255,255,255,0};
		bmTips[0]=TTF_RenderText_Shaded(fontText,s.str().c_str(),fg,bg);
		if(bmTips[0])
			SDL_SetAlpha(bmTips[0],SDL_SRCALPHA,160);
	}

	//Get the background
	background=objThemes.getBackground();
	if(background)
		background->resetAnimation(true);
}

void Game::loadLevel(string fileName){
	//Create a TreeStorageNode that will hold the loaded data.
	TreeStorageNode *obj=new TreeStorageNode();
	{
		POASerializer objSerializer;
		string s=fileName;
		
		//Parse the file.
		if(!objSerializer.loadNodeFromFile(s.c_str(),obj,true)){
			cout<<"Can't load level file "<<s<<endl;
			delete obj;
			return;
		}
	}

	//Now call another function.
	loadLevelFromNode(obj,fileName);
}

void Game::saveRecord(const char* fileName){
	//check if current level is NULL (which should be impossible)
	if(currentLevelNode==NULL) return;

	TreeStorageNode obj;
	POASerializer objSerializer;

	//put current level to the node.
	currentLevelNode->name="map";
	obj.subNodes.push_back(currentLevelNode);

	//serialize the game record using RLE compression.
#define PUSH_BACK \
			if(j>0){ \
				if(j>1){ \
					sprintf(c,"%d*%d",last,j); \
				}else{ \
					sprintf(c,"%d",last); \
				} \
				v.push_back(c); \
			}
	vector<string> &v=obj.attributes["record"];
	vector<int> *record=player.getRecord();
	char c[64];
	int i,j=0,last;
	for(i=0;i<(int)record->size();i++){
		int currentKey=(*record)[i];
		if(j==0 || currentKey!=last){
			PUSH_BACK;
			last=currentKey;
			j=1;
		}else{
			j++;
		}
	}
	PUSH_BACK;
#undef PUSH_BACK

#ifdef RECORD_FILE_DEBUG
	//add record file debug data.
	{
		obj.attributes["recordKeyPressLog"].push_back(player.keyPressLog());

		vector<SDL_Rect> &playerPosition=player.playerPosition();
		string s;
		char c[32];

		sprintf(c,"%d\n",int(playerPosition.size()));
		s=c;

		for(unsigned int i=0;i<playerPosition.size();i++){
			SDL_Rect& r=playerPosition[i];
			sprintf(c,"%d %d\n",r.x,r.y);
			s+=c;
		}

		obj.attributes["recordPlayerPosition"].push_back(s);
	}
#endif

	//save it
	objSerializer.saveNodeToFile(fileName,&obj,true,true);

	//remove current level from node to prevent delete it.
	obj.subNodes.clear();
}

void Game::loadRecord(const char* fileName){
	//Create a TreeStorageNode that will hold the loaded data.
	TreeStorageNode obj;
	{
		POASerializer objSerializer;
		string s=fileName;
		
		//Parse the file.
		if(!objSerializer.loadNodeFromFile(s.c_str(),&obj,true)){
			cout<<"Can't load record file "<<s<<endl;
			return;
		}
	}

	//find the node named 'map'.
	bool loaded=false;
	for(unsigned int i=0;i<obj.subNodes.size();i++){
		if(obj.subNodes[i]->name=="map"){
			//load the level. (fileName=???)
			loadLevelFromNode(obj.subNodes[i],"???");
			//remove this node to prevent delete it.
			obj.subNodes[i]=NULL;
			//over
			loaded=true;
			break;
		}
	}

	if(!loaded){
		cout<<"ERROR: Can't find subnode named 'map' from record file"<<endl;
		return;
	}

	//load the record.
	{
		vector<int> *record=player.getRecord();
		record->clear();
		vector<string> &v=obj.attributes["record"];
		for(unsigned int i=0;i<v.size();i++){
			string &s=v[i];
			string::size_type pos=s.find_first_of('*');
			if(pos==string::npos){
				//1 item only.
				int i=atoi(s.c_str());
				record->push_back(i);
			}else{
				//contains many items.
				int i=atoi(s.substr(0,pos).c_str());
				int j=atoi(s.substr(pos+1).c_str());
				for(;j>0;j--){
					record->push_back(i);
				}
			}
		}
	}

#ifdef RECORD_FILE_DEBUG
	//load the debug data
	{
		vector<string> &v=obj.attributes["recordPlayerPosition"];
		vector<SDL_Rect> &playerPosition=player.playerPosition();
		playerPosition.clear();
		if(!v.empty()){
			if(!v[0].empty()){
				stringstream st(v[0]);
				int m;
				st>>m;
				for(int i=0;i<m;i++){
					SDL_Rect r;
					st>>r.x>>r.y;
					r.w=0;
					r.h=0;
					playerPosition.push_back(r);
				}
			}
		}
	}
#endif

	//play the record.
	//TODO: tell the level manager don't save the level progress.
	player.playRecord();
	shadow.playRecord(); //???
}

/////////////EVENT///////////////
void Game::handleEvents(){
	//First of all let the player handle input.
	player.handleInput(&shadow);

	//Check for an SDL_QUIT event.
	if(event.type==SDL_QUIT){
		//We need to quit so enter STATE_EXIT.
		setNextState(STATE_EXIT);
	}
	
	//Check for the escape key.
	if(inputMgr.isKeyUpEvent(INPUTMGR_ESCAPE)){
		//Escape means we go one level up, to the level select state.
		setNextState(STATE_LEVEL_SELECT);
		//Save the progress.
		levels.saveLevelProgress();
		
		//And change the music back to the menu music.
		getMusicManager()->playMusic("menu");
	}
	
	//Check if 'r' is pressed.
	if(inputMgr.isKeyDownEvent(INPUTMGR_RESTART)){
		//Check if it isn't a replay.
		if(!player.isPlayFromRecord()){
			//Set reset true.
			isReset=true;
		}else if(interlevel){
			//We can also reset during the inter level screen.
			if(GUIObjectRoot){
				delete GUIObjectRoot;
				GUIObjectRoot=NULL;
			}
			
			isReset=true;
		}
	}
	
	//Check for the next level buttons when in the interlevel popup.
	if(inputMgr.isKeyDownEvent(INPUTMGR_SPACE) || (event.type==SDL_KEYDOWN && (event.key.keysym.sym==SDLK_RETURN || event.key.keysym.sym==SDLK_RCTRL))){
		if(interlevel){
			//The interlevel popup is shown so we need to delete it.
			if(GUIObjectRoot){
				delete GUIObjectRoot;
				GUIObjectRoot=NULL;
			}
			
			//Now goto the next level.
			gotoNextLevel();
		}
	}
	
	//Check if tab is pressed.
	if(inputMgr.isKeyDownEvent(INPUTMGR_TAB)){
		shadowCam=!shadowCam;
	}
}

/////////////////LOGIC///////////////////
void Game::logic(){
	//Add one tick to the time.
	time++;
	
	//Let the player store his move, if recording.
	player.shadowSetState();
	//Let the player give his recording to the shadow, if configured.
	player.shadowGiveState(&shadow);
	//Let the player jump.
	player.jump();
	//Let him move.
	player.move(levelObjects);
	//And let the camera follow him.
	if(!shadowCam){
		player.setMyCamera();
	}else{
		shadow.setMyCamera();
	}

	//Now let the shadow decide his move, if he's playing a recording.
	shadow.moveLogic();
	//Let the shadow jump.
	shadow.jump();
	//Let the shadow move.
	shadow.move(levelObjects);

	//Some levelObjects can move so update them.
	for(unsigned int i=0;i<levelObjects.size();i++){
		levelObjects[i]->move();
	}

	//Process any event in the queue.
	for(unsigned int idx=0;idx<eventQueue.size();idx++){
		//Get the event from the queue.
		typeGameObjectEvent &e=eventQueue[idx];
		
		//Check if the it has an id attached to it.
		if(e.flags|1){
			//Loop through the levelObjects and give them the event if they have the right id.
			for(unsigned int i=0;i<levelObjects.size();i++){
				if(e.objectType<0 || levelObjects[i]->type==e.objectType){
					Block *obj=dynamic_cast<Block*>(levelObjects[i]);
					if(obj!=NULL && obj->id==e.id){
						levelObjects[i]->onEvent(e.eventType);
					}
				}
			}
		}else{
			//Loop through the levelObjects and give them the event.
			for(unsigned int i=0;i<levelObjects.size();i++){
				if(e.objectType<0 || levelObjects[i]->type==e.objectType){
					levelObjects[i]->onEvent(e.eventType);
				}
			}
		}
	}
	//Done processing the events so clear the queue.
	eventQueue.clear();
	
	//Check collision and stuff for the shadow and player.
	player.otherCheck(&shadow);
	
	//Check if we won.
	if(won){
		//the string to store auto-save record path.
		string bestTimeFilePath,bestRecordingFilePath;
		//and if we can't get test path.
		bool filePathError=false;
		
		//Set the current level won.
		levels.getLevel()->won=true;
		if(levels.getLevel()->time==-1 || levels.getLevel()->time>time){
			levels.getLevel()->time=time;
			//save the best-time game record.
			if(bestTimeFilePath.empty()){
				getCurrentLevelAutoSaveRecordPath(bestTimeFilePath,bestRecordingFilePath,true);
			}
			if(bestTimeFilePath.empty()){
				cout<<"ERROR: Couldn't get auto-save record file path"<<endl;
				filePathError=true;
			}else{
				saveRecord(bestTimeFilePath.c_str());
			}
		}
		if(levels.getLevel()->recordings==-1 || levels.getLevel()->recordings>recordings){
			levels.getLevel()->recordings=recordings;
			//save the best-recordings game record.
			if(bestRecordingFilePath.empty() && !filePathError){
				getCurrentLevelAutoSaveRecordPath(bestTimeFilePath,bestRecordingFilePath,true);
			}
			if(bestRecordingFilePath.empty()){
				cout<<"ERROR: Couldn't get auto-save record file path"<<endl;
				filePathError=true;
			}else{
				saveRecord(bestRecordingFilePath.c_str());
			}
		}
		
		//Set the next level unlocked if it exists.
		if(levels.getCurrentLevel()+1<levels.getLevelCount()){
			levels.setLocked(levels.getCurrentLevel()+1);
		}
		//And save the progress.
		levels.saveLevelProgress();
		
		//Now go to the interlevel screen.
		replayPlay();
		
		//NOTE: We set isReset false to prevent the user from getting a best time of 0.00s and 0 recordings.
	}
	won=false;
	
	//Check if we should reset.
	if(isReset)
		reset(false);
	isReset=false;
}

/////////////////RENDER//////////////////
void Game::render(){
	//First of all render the background.
	{
		//Get a pointer to the background.
		ThemeBackground* bg=background;
		
		//Check if the background is null, but there are themes.
		if(bg==NULL && objThemes.themeCount()>0){
			//Get the background from the first theme in the stack.
			bg=objThemes[0]->getBackground();
		}
		
		//Check if the background isn't null.
		if(bg){
			//It isn't so draw it.
			bg->draw(screen);
			
			//And if it's the loaded background then also update the animation.
			//FIXME: Updating the animation in the render method?
			if(bg==background)
				bg->updateAnimation();
		}else{
			//There's no background so fill the screen with white.
			SDL_Rect r={0,0,SCREEN_WIDTH,SCREEN_HEIGHT};
			SDL_FillRect(screen,&r,-1);
		}
	}

	//Now we draw the levelObjects.
	for(unsigned int o=0; o<levelObjects.size(); o++){
		levelObjects[o]->show();
	}

	//Followed by the player and the shadow.
	player.show();
	shadow.show();

	//Show the levelName if it isn't the level editor.
	if(stateID!=STATE_LEVEL_EDITOR && bmTips[0]!=NULL && !interlevel){
		applySurface(0,SCREEN_HEIGHT-bmTips[0]->h,bmTips[0],screen,NULL);
	}
	
	//Check if there's a tooltip.
	//NOTE: gameTipIndex 0 is used for the levelName, 1 for shadow death, 2 for restart text, 3 for restart+checkpoint.
	if(gameTipIndex>3 && gameTipIndex<TYPE_MAX){
		//Check if there's a tooltip for the type.
		if(bmTips[gameTipIndex]==NULL){
			//There isn't thus make it.
			const char* s=NULL;
			switch(gameTipIndex){
			case TYPE_CHECKPOINT:
				s="Press DOWN key to save the game.";
				break;
			case TYPE_SWAP:
				s="Press DOWN key to swap the position of player and shadow.";
				break;
			case TYPE_SWITCH:
				s="Press DOWN key to activate the switch.";
				break;
			case TYPE_PORTAL:
				s="Press DOWN key to teleport.";
				break;
			}
			
			//If we have a string then it's a supported GameObject type.
			if(s!=NULL){
				SDL_Color fg={0,0,0,0},bg={255,255,255,0};
				bmTips[gameTipIndex]=TTF_RenderText_Shaded(fontText,s,fg,bg);
				SDL_SetAlpha(bmTips[gameTipIndex],SDL_SRCALPHA,160);
			}
		}
		
		//We already have a gameTip for this type so draw it.
		if(bmTips[gameTipIndex]!=NULL){
			applySurface(0,0,bmTips[gameTipIndex],screen,NULL);
		}
	}
	//Set the gameTip to 0.
	gameTipIndex=0;
	
	//Pointer to the sdl surface that will contain a message, if any.
	SDL_Surface* bm=NULL;
	
	//Check if the player is dead, meaning we draw a message.
	if(player.dead){
		//The player is dead, check if there's a state that can be loaded.
		if(player.canLoadState()){
			//Now check if the tip is already made, if not make it.
			if(bmTips[3]==NULL){
				SDL_Color fg={0,0,0,0},bg={255,255,255,0};
				bmTips[3]=TTF_RenderText_Shaded(fontText,
					"Press R to restart current level or press F3 to load the game.",
					fg,bg);
				SDL_SetAlpha(bmTips[3],SDL_SRCALPHA,160);
			}
			bm=bmTips[3];
		}else{
			//Now check if the tip is already made, if not make it.
			if(bmTips[2]==NULL){
				SDL_Color fg={0,0,0,0},bg={255,255,255,0};
				bmTips[2]=TTF_RenderText_Shaded(fontText,
					"Press R to restart current level.",
					fg,bg);
				SDL_SetAlpha(bmTips[2],SDL_SRCALPHA,160);
			}
			bm=bmTips[2];
		}
	}
	
	//Check if the shadow has died (and there's no other notification).
	//NOTE: We use the shadow's jumptime as countdown, this variable isn't used when the shadow is dead.
	if(shadow.dead && bm==NULL && shadow.jumpTime>0){
		//Now check if the tip is already made, if not make it.
		if(bmTips[1]==NULL){
			SDL_Color fg={0,0,0,0},bg={255,255,255,0};
			bmTips[1]=TTF_RenderText_Shaded(fontText,
				"Your shadow has died.",
				fg,bg);
			SDL_SetAlpha(bmTips[1],SDL_SRCALPHA,160);
		}
		bm=bmTips[1];
		
		//NOTE: Logic in the render loop, we substract the shadow's jumptime by one.
		shadow.jumpTime--;
	}
	
	//Draw the tip.
	if(bm!=NULL)
		applySurface(0,0,bm,screen,NULL);

	//show time and records used in level editor.
	if(stateID==STATE_LEVEL_EDITOR && time>0){
		char c[32];
		SDL_Color fg={0,0,0,0},bg={255,255,255,0};
		SDL_Surface *bm;
		int y=SCREEN_HEIGHT;

		sprintf(c,"%d recordings",recordings);
		bm=TTF_RenderText_Shaded(fontText,c,fg,bg);
		SDL_SetAlpha(bm,SDL_SRCALPHA,160);
		y-=bm->h;
		applySurface(0,y,bm,screen,NULL);
		SDL_FreeSurface(bm);

		sprintf(c,"%-.2fs",time/40.0f);
		bm=TTF_RenderText_Shaded(fontText,c,fg,bg);
		SDL_SetAlpha(bm,SDL_SRCALPHA,160);
		y-=bm->h;
		applySurface(0,y,bm,screen,NULL);
		SDL_FreeSurface(bm);
	}
	
	//Draw the current action in the upper right corner.
	if(player.record){
		applySurface(750,0,action,screen,NULL);
	}else if(shadow.state!=0){
		SDL_Rect r={50,0,50,50};
		applySurface(750,0,action,screen,&r);
	}

	//if the game is play from record then draw something indicates it
	if(player.isPlayFromRecord()){
		//Dim the screen if interlevel is true.
		if(interlevel){
			SDL_BlitSurface(screen,NULL,tempSurface,NULL);
			SDL_FillRect(screen,NULL,0);
			SDL_SetAlpha(tempSurface, SDL_SRCALPHA,220);
			SDL_BlitSurface(tempSurface,NULL,screen,NULL);
			
			//Check if the GUI isn't null.
			if(GUIObjectRoot){
				//==Create first box==
				
				//Create the title
				SDL_Color black={0,0,0,0};
				SDL_Rect r;
				SDL_Surface* bm=TTF_RenderText_Blended(fontGUI,"You've finished:",black);
				
				//Recreate the level string.
				stringstream s;
				if(levels.getLevelCount()>0){
					s<<"Level "<<(levels.getCurrentLevel()+1)<<" ";
				}
				s<<levelName;
				
				SDL_Surface* bm2=TTF_RenderText_Blended(fontText,s.str().c_str(),black);
				
				//Now draw the first gui box so that it's bigger than longer text.
				int width;
				if(bm->w>bm2->w)
					width=bm->w+32;
				else
					width=bm2->w+32;
				drawGUIBox((SCREEN_WIDTH-width)/2,4,width,68,screen,0xDDDDDDA1);
				
				// Now draw title.
				r.x=(800-bm->w)/2;
				r.y=8;
				SDL_BlitSurface(bm,NULL,screen,&r);
				
				// And then level name.
				r.x=(800-bm2->w)/2;
				r.y=44;
				SDL_BlitSurface(bm2,NULL,screen,&r);
				
				//Free drawed texts
				SDL_FreeSurface(bm);
				SDL_FreeSurface(bm2);
				
				//==Create second box==
				
				//Now draw the second gui box.
				drawGUIBox(GUIObjectRoot->left,GUIObjectRoot->top,GUIObjectRoot->width,GUIObjectRoot->height,screen,0xDDDDDDA1);
				
				//Draw the medal.
				int medal=GUIObjectRoot->value;
				r.x=(medal-1)*30;
				r.y=0;
				r.w=30;
				r.h=30;
				applySurface(GUIObjectRoot->left+16,GUIObjectRoot->top+92,medals,screen,&r);
				applySurface(GUIObjectRoot->left+370,GUIObjectRoot->top+92,medals,screen,&r);
			}
		}else if((time & 0x10)==0x10){
			SDL_Rect r={50,0,50,50};
			applySurface(0,0,action,screen,&r);
			applySurface(0,SCREEN_HEIGHT-50,action,screen,&r);
			applySurface(SCREEN_WIDTH-50,SCREEN_HEIGHT-50,action,screen,&r);
		}
	}else if(player.objNotificationBlock){
		//If the player is in front of a notification block show the message.
		//And it isn't a replay.
		std::vector<char> string_data((dynamic_cast<Block*>(player.objNotificationBlock))->message.begin(), (dynamic_cast<Block*>(player.objNotificationBlock))->message.end());
		string_data.push_back('\0');
		
		int y = 20;
		vector<SDL_Surface*> lines;
		
		//Now process the prompt.
		{
			//Pointer to the string.
			char* lps=&string_data[0];
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
				
				//Color the text will be: black.
				SDL_Color black={0,0,0,0};
				lines.push_back(TTF_RenderText_Blended(fontText,lps,black));
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
		drawGUIBox(100,SCREEN_HEIGHT-y-25,600,y+20,screen,0xDDDDDDA1);
		while(!lines.empty()){
			SDL_Surface* bm=lines[0];
			
			if(bm!=NULL){
				applySurface(100+((600-bm->w)/2),SCREEN_HEIGHT-y,bm,screen,NULL);
				SDL_FreeSurface(bm);
			}
			
			y-=25;
			lines.erase(lines.begin());
			
		}
	}
}

void Game::replayPlay(){
	interlevel=true;

	//cout<<"Game::replayPlay()"<<endl;
	
	//Fix the bug that press "r" to restart a level just before finishing it
	//will cause inter-level popup appeares and the game restarts.
	//Now when the game is about to finish, we don't allow to restart.
	isReset=false;
	
	//Create the gui if it isn't already done.
	if(!GUIObjectRoot){
		GUIObjectRoot=new GUIObject(125,460,570,135,GUIObjectNone);
		//NOTE: We put the medal in the value of the GUIObjectRoot.
		
		//The different values.
		int bestTime=levels.getLevel()->time;
		int targetTime=levels.getLevel()->targetTime;
		int bestRecordings=levels.getLevel()->recordings;
		int targetRecordings=levels.getLevel()->targetRecordings;
		
		int medal=1;
		if(targetTime<0){
			medal=3;
		}else{
			if(targetTime<0 || bestTime<=targetTime)
				medal++;
			if(targetRecordings<0 || bestRecordings<=targetRecordings)
				medal++;
		}
		//Add it to the GUIObjectRoot.
		GUIObjectRoot->value=medal;
		
		//Create the labels with the time and best time.
		char s1[64];
		sprintf(s1,"Time: %-.2fs",time/40.0f);
		GUIObject* obj=new GUIObject(20,10,150,36,GUIObjectLabel,s1);
		GUIObjectRoot->childControls.push_back(obj);
		
		sprintf(s1,"Best time: %-.2fs",bestTime/40.0f);
		obj=new GUIObject(20,34,150,36,GUIObjectLabel,s1);
		GUIObjectRoot->childControls.push_back(obj);
		
		if(targetTime>=0){
			sprintf(s1,"Target time: %-.2fs",targetTime/40.0f);
			obj=new GUIObject(20,58,150,36,GUIObjectLabel,s1);
			GUIObjectRoot->childControls.push_back(obj);
		}
		
		//Now the ones for the recordings.
		sprintf(s1,"Recordings: %d",recordings);
		obj=new GUIObject(210,10,150,36,GUIObjectLabel,s1);
		GUIObjectRoot->childControls.push_back(obj);
		
		sprintf(s1,"Best recordings: %d",bestRecordings);
		obj=new GUIObject(210,34,150,36,GUIObjectLabel,s1);
		GUIObjectRoot->childControls.push_back(obj);
		
		if(targetRecordings>=0){
			sprintf(s1,"Target recordings: %d",targetRecordings);
			obj=new GUIObject(210,58,150,36,GUIObjectLabel,s1);
			GUIObjectRoot->childControls.push_back(obj);
		}
		
		//The medal that is earned.
		sprintf(s1,"You earned the %s medal",(medal>1)?(medal==3)?"GOLD":"SILVER":"BRONZE");
		obj=new GUIObject(48,92,150,36,GUIObjectLabel,s1);
		//Center it horizontally.
		int width;
		TTF_SizeText(fontText,s1,&width,NULL);
		obj->width=width;
		obj->left=(416-width)/2;
		GUIObjectRoot->childControls.push_back(obj);
		
		//Create the three buttons, Menu, Restart, Next.
		obj=new GUIObject(420,10,128,36,GUIObjectButton,"Menu");
		obj->name="cmdMenu";
		obj->eventCallback=this;
		GUIObjectRoot->childControls.push_back(obj);
		
		obj=new GUIObject(409,50,150,36,GUIObjectButton,"Restart");
		obj->name="cmdRestart";
		obj->eventCallback=this;
		GUIObjectRoot->childControls.push_back(obj);
		
		obj=new GUIObject(420,90,128,36,GUIObjectButton,"Next");
		obj->name="cmdNext";
		obj->eventCallback=this;
		GUIObjectRoot->childControls.push_back(obj);
	}
	
	//We only need to reset a few things so we don't call reset().
	for(unsigned int i=0;i<levelObjects.size();i++){
		levelObjects[i]->reset(true);
	}
	//Also reset the background animation, if any.
	if(background)
		background->resetAnimation(true);
	
	//Make a copy of the playerButtons.
	vector<int> recordCopy=player.recordButton;
	player.reset(true);
	shadow.reset(true);
	player.recordButton=recordCopy;
	
	//Now play the recording.
	player.playRecord();
}

void Game::recordingEnded(){
	//Check if it's a normal replay, if so just stop.
	if(!interlevel){
		msgBox("Game replay is done.",MsgBoxOKOnly,"Game Replay");
		//Go to the level select menu.
		setNextState(STATE_LEVEL_SELECT);
		
		//And change the music back to the menu music.
		getMusicManager()->playMusic("menu");
	}else{
		//Instead of directly replaying we set won true to let the Game handle the replaying at the end of the update cycle.
		won=true;
	}
}

bool Game::saveState(){
	//Check if the player and shadow can save the current state.
	if(player.canSaveState() && shadow.canSaveState()){
		//Let the player and the shadow save their state.
		player.saveState();
		shadow.saveState();
		
		//Save the stats.
		timeSaved=time;
		recordingsSaved=recordings;
		
		//Save other state, for example moving blocks.
		for(unsigned int i=0;i<levelObjects.size();i++){
			levelObjects[i]->saveState();
		}
		
		//Also save the background animation, if any.
		if(background)
			background->saveAnimation();
		
		//Return true.
		return true;
	}
	
	//We can't save the state so return false.
	return false;
}

bool Game::loadState(){
	//Check if there's a state that can be loaded.
	if(player.canLoadState() && shadow.canLoadState()){
		//Let the player and the shadow load their state.
		player.loadState();
		shadow.loadState();
		
		//Load the stats.
		time=timeSaved;
		recordings=recordingsSaved;
		
		//Load other state, for example moving blocks.
		for(unsigned int i=0;i<levelObjects.size();i++){
			levelObjects[i]->loadState();
		}
		
		//Also load the background animation, if any.
		if(background)
			background->loadAnimation();
		
		//Return true.
		return true;
	}
	
	//We can't load the state so return false.
	return false;
}

void Game::reset(bool save){
	//We need to reset the game so we also reset the player and the shadow.
	player.reset(save);
	shadow.reset(save);
	
	//Reset the stats.
	time=0;
	recordings=0;
	
	//There is no last checkpoint so set it to NULL.
	if(save)
		objLastCheckPoint=NULL;
	
	//Reset other state, for example moving blocks.
	for(unsigned int i=0;i<levelObjects.size();i++){
		levelObjects[i]->reset(save);
	}
	//Also reset the background animation, if any.
	if(background)
		background->resetAnimation(save);
	
	//Also set interlevel to false.
	interlevel=false;
}

void Game::broadcastObjectEvent(int eventType,int objectType,const char* id){
	//Create a typeGameObjectEvent that can be put into the queue.
	typeGameObjectEvent e;
	//Set the event type.
	e.eventType=eventType;
	//Set the object type.
	e.objectType=objectType;
	//By default flags=0.
	e.flags=0;
	//Unless there's an id.
	if(id){
		//Set flags to 0x1 and set the id.
		e.flags|=1;
		e.id=id;
	}
	
	//Add the event to the queue.
	eventQueue.push_back(e);
}

void Game::getCurrentLevelAutoSaveRecordPath(std::string &bestTimeFilePath,std::string &bestRecordingFilePath,bool createPath){
	levels.getLevelAutoSaveRecordPath(-1,bestTimeFilePath,bestRecordingFilePath,createPath);
}

void Game::gotoNextLevel(){
	//Goto the next level.
	levels.nextLevel();
	
	//Check if the level exists.
	if(levels.getCurrentLevel()<levels.getLevelCount()){			
		setNextState(STATE_GAME);
		
		//Don't forget the music.
		getMusicManager()->pickMusic();
	}else{
		if(!levels.congratulationText.empty()){
			msgBox(levels.congratulationText,MsgBoxOKOnly,"Congratulations");
		}else{
			msgBox("You have finished the levelpack!",MsgBoxOKOnly,"Congratulations");
		}
		//Now go back to the levelselect screen.
		setNextState(STATE_LEVEL_SELECT);
		//And set the music back to menu.
		getMusicManager()->playMusic("menu");
	}
}

void Game::GUIEventCallback_OnEvent(string name,GUIObject* obj,int eventType){
	if(name=="cmdMenu"){
		setNextState(STATE_LEVEL_SELECT);
		
		//And change the music back to the menu music.
		getMusicManager()->playMusic("menu");
	}else if(name=="cmdRestart"){
		//Clear the gui.
		if(GUIObjectRoot){
			delete GUIObjectRoot;
			GUIObjectRoot=NULL;
		}
		
		//And reset the game.
		//new: we don't need to clear the save game because
		//in level replay the game won't be saved
		//TODO: it doesn't work; better leave for next release
		reset(/*false*/ true);
	}else if(name=="cmdNext"){
		//No matter what, clear the gui.
		if(GUIObjectRoot){
			delete GUIObjectRoot;
			GUIObjectRoot=NULL;
		}
		
		//And goto the next level.
		gotoNextLevel();
	}
}
