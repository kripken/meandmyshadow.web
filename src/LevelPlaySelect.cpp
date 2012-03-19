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
#include "LevelPlaySelect.h"
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

/////////////////////LEVEL SELECT/////////////////////
static string levelDescription,levelMedal2,levelMedal3;
static string bestTimeFilePath,bestRecordingFilePath;

LevelPlaySelect::LevelPlaySelect():LevelSelect("Select Level"){
	//Load the play button if needed.
	playButtonImage=loadImage(getDataPath()+"gfx/playbutton.png");
	timeIcon=loadImage(getDataPath()+"gfx/time.png");
	recordingsIcon=loadImage(getDataPath()+"gfx/recordings.png");
	
	play=new GUIObject(560,540,240,32,GUIObjectButton,"Play");
	play->name="cmdPlay";
	play->eventCallback=this;
	play->enabled=false;
	GUIObjectRoot->childControls.push_back(play);
	
	//show level list
	refresh();
}

LevelPlaySelect::~LevelPlaySelect(){
	play=NULL;
	recordingsIcon=NULL;
	timeIcon=NULL;
}

void LevelPlaySelect::refresh(){
	int m=levels.getLevelCount();
	numbers.clear();

	//clear the selected level
	if(selectedNumber!=NULL){
		delete selectedNumber;
		selectedNumber=NULL;
	}
	//Recreate the non selected number.
	selectedNumber=new Number();
	SDL_Rect box={40,470,50,50};
	selectedNumber->init(" ",box);
	selectedNumber->setLocked(true);
	
	levelDescription="Choose a level";
	levelMedal2="Time:             - / -";
	levelMedal3="Recordings:     - / -";
	
	bestTimeFilePath.clear();
	bestRecordingFilePath.clear();
	
	
	//Disable the play button.
	play->enabled=false;

	for(int n=0; n<m; n++){
		numbers.push_back(Number());
	}

	for(int n=0; n<m; n++){
		SDL_Rect box={(n%10)*64+80,(n/10)*64+184,0,0};
		numbers[n].init(n,box);
		numbers[n].setLocked(levels.getLocked(n));
		int medal=levels.getLevel(n)->won;
		if(medal){
			if(levels.getLevel(n)->targetTime<0 || levels.getLevel(n)->time<=levels.getLevel(n)->targetTime)
				medal++;
			if(levels.getLevel(n)->targetRecordings<0 || levels.getLevel(n)->recordings<=levels.getLevel(n)->targetRecordings)
				medal++;
		}
		numbers[n].setMedal(medal);
	}

	if(m>LEVELS_DISPLAYED_IN_SCREEN){
		levelScrollBar->maxValue=(m-LEVELS_DISPLAYED_IN_SCREEN+9)/10;
		levelScrollBar->visible=true;
	}else{
		levelScrollBar->maxValue=0;
		levelScrollBar->visible=false;
	}
	levelpackDescription->caption=levels.levelpackDescription;
	int width;
	TTF_SizeText(fontText,levels.levelpackDescription.c_str(),&width,NULL);
	levelpackDescription->width=width;
	levelpackDescription->left=(SCREEN_WIDTH-width)/2;
}

void LevelPlaySelect::selectNumber(unsigned int number,bool selected){
	if(selected){
		levels.setCurrentLevel(number);
		setNextState(STATE_GAME);
		
		//Pick music from the current music list.
		getMusicManager()->pickMusic();
	}else{
		displayLevelInfo(number);
	}
}

void LevelPlaySelect::checkMouse(){  
	int x,y;
	
	//Get the current mouse location.
	SDL_GetMouseState(&x,&y);
	
	//Check if we should replay the record.
	if(selectedNumber!=NULL){
		SDL_Rect mouse={x,y,0,0};
		if(!bestTimeFilePath.empty()){
			SDL_Rect box={380,470,372,32};
			if(checkCollision(box,mouse)){
				Game::recordFile=bestTimeFilePath;
				levels.setCurrentLevel(selectedNumber->getNumber());
				setNextState(STATE_GAME);
				
				//Pick music from the current music list.
				getMusicManager()->pickMusic();
				return;
			}
		}
		if(!bestRecordingFilePath.empty()){
			SDL_Rect box={380,502,372,32};
			if(checkCollision(box,mouse)){
				Game::recordFile=bestRecordingFilePath;
				levels.setCurrentLevel(selectedNumber->getNumber());
				setNextState(STATE_GAME);
				
				//Pick music from the current music list.
				getMusicManager()->pickMusic();
				return;
			}
		}
	}
	
	//Call the base method from the super class.
	LevelSelect::checkMouse();
}

void LevelPlaySelect::displayLevelInfo(int number){
	//Update currently selected level
	if(selectedNumber==NULL){
		selectedNumber=new Number();
	}
	SDL_Rect box={40,470,50,50};
	selectedNumber->init(number,box);
	selectedNumber->setLocked(false);

	//Show level description
	levelDescription=levels.getLevelName(number);

	//Show level medal
	int medal=levels.getLevel(number)->won;
	int time=levels.getLevel(number)->time;
	int targetTime=levels.getLevel(number)->targetTime;
	int recordings=levels.getLevel(number)->recordings;
	int targetRecordings=levels.getLevel(number)->targetRecordings;

	if(medal){
		if(targetTime<0){
			medal=-1;
		}else{
			if(targetTime<0 || time<=targetTime)
				medal++;
			if(targetRecordings<0 || recordings<=targetRecordings)
				medal++;
		}
	}
	selectedNumber->setMedal(medal);
	
	//Show best time and recordings
	if(medal){
		char s[64];
		
		if(time>0)
			if(targetTime>0)
				sprintf(s,"%-.2fs / %-.2fs",time/40.0f,targetTime/40.0f);
			else
				sprintf(s,"%-.2fs / -",time/40.0f);
		else
			s[0]='\0';
		levelMedal2=string("Time:        ")+s;

		if(recordings>=0)
			if(targetRecordings>=0)
				sprintf(s,"%5d / %d",recordings,targetRecordings);
			else
				sprintf(s,"%5d / -",recordings);
		else
			s[0]='\0';
		levelMedal3=string("Recordings: ")+s;
	}else{
		levelMedal2="Time:             - / -";
		levelMedal3="Recordings:     - / -";
	}
	
	//Show the play button.
	play->enabled=true;
	
	//Check if there is auto record file
	levels.getLevelAutoSaveRecordPath(number,bestTimeFilePath,bestRecordingFilePath,false);
	if(!bestTimeFilePath.empty()){
		FILE *f;
		f=fopen(bestTimeFilePath.c_str(),"rb");
		if(f==NULL){
			bestTimeFilePath.clear();
		}else{
			fclose(f);
		}
	}
	if(!bestRecordingFilePath.empty()){
		FILE *f;
		f=fopen(bestRecordingFilePath.c_str(),"rb");
		if(f==NULL){
			bestRecordingFilePath.clear();
		}else{
			fclose(f);
		}
	}
}

void LevelPlaySelect::render(){
	//First let the levelselect render.
	LevelSelect::render();
	
	int x,y,dy=0,m=levels.getLevelCount();
	
	//Get the current mouse location.
	SDL_GetMouseState(&x,&y);

	if(levelScrollBar)
		dy=levelScrollBar->value;
	//Upper bound of levels we'd like to display.
	if(m>dy*10+LEVELS_DISPLAYED_IN_SCREEN)
		m=dy*10+LEVELS_DISPLAYED_IN_SCREEN;
	y+=dy*64;
	
	SDL_Rect mouse={x,y,0,0};
	
	//Show currently selected level (if any)
	if(selectedNumber!=NULL){
		selectedNumber->show(0);
		
		SDL_Color fg={0,0,0};
		SDL_Surface* bm;
		
		if(!levelDescription.empty()){
			bm=TTF_RenderText_Blended(fontText,levelDescription.c_str(),fg);
			applySurface(100,470+(50-bm->h)/2,bm,screen,NULL);
			SDL_FreeSurface(bm);
		}
		
		//Only show the replay if the level is completed (won).
		if(selectedNumber->getNumber()>0 && selectedNumber->getNumber()<levels.getLevelCount()) {
			if(levels.getLevel(selectedNumber->getNumber())->won){
				if(!bestTimeFilePath.empty()){
					SDL_Rect r={0,0,32,32};
					SDL_Rect box={380,470,372,32};
					
					if(checkCollision(box,mouse)){
						r.x=32;
						SDL_FillRect(screen,&box,0xFFCCCCCC);
					}
					
					applySurface(720,470,playButtonImage,screen,&r);
				}
				
				if(!bestRecordingFilePath.empty()){
					SDL_Rect r={0,0,32,32};
					SDL_Rect box={380,502,372,32};
					
					if(checkCollision(box,mouse)){
						r.x=32;
						SDL_FillRect(screen,&box,0xFFCCCCCC);
					}
					
					applySurface(720,502,playButtonImage,screen,&r);
				}
			}
		}
		
		if(!levelMedal2.empty()){
			//Draw the icon.
			applySurface(395,470+3,timeIcon,screen,NULL);
			
			//Now draw the text.
			bm=TTF_RenderText_Blended(fontText,levelMedal2.c_str(),fg);
			applySurface(420,470+3,bm,screen,NULL);
			SDL_FreeSurface(bm);
		}

		if(!levelMedal3.empty()){
			//Draw the icon.
			applySurface(395,502+(6)/2,recordingsIcon,screen,NULL);
			
			//Now draw the text.
			bm=TTF_RenderText_Blended(fontText,levelMedal3.c_str(),fg);
			applySurface(420,502+(32-bm->h)/2,bm,screen,NULL);
			SDL_FreeSurface(bm);
		}
	}
}

void LevelPlaySelect::renderTooltip(unsigned int number,int dy){
	SDL_Color fg={0,0,0};
	char s[64];
	
	//Render the name of the level.
	SDL_Surface* name=TTF_RenderText_Blended(fontText,levels.getLevelName(number).c_str(),fg);
	SDL_Surface* time=NULL;
	SDL_Surface* recordings=NULL;
	
	//The time it took.
	if(levels.getLevel(number)->time>0){
		sprintf(s,"%-.2fs",levels.getLevel(number)->time/40.0f);
		time=TTF_RenderText_Blended(fontText,s,fg);
	}
	
	//The number of recordings it took.
	if(levels.getLevel(number)->recordings>=0){
		sprintf(s,"%d",levels.getLevel(number)->recordings);
		recordings=TTF_RenderText_Blended(fontText,s,fg);
	}
	
	
	//Now draw a square the size of the three texts combined.
	SDL_Rect r=numbers[number].box;
	r.y-=dy*64;
	if(time!=NULL && recordings!=NULL){
		r.w=(name->w)>(25+time->w+40+recordings->w)?(name->w):(25+time->w+40+recordings->w);
		r.h=name->h+5+20;
	}else{
		r.w=name->w;
		r.h=name->h;
	}
	
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
	//Increase the height to leave a gap between name and stats.
	r2.y+=30;
	if(time!=NULL){
		//Now draw the time.
		applySurface(r2.x,r2.y,timeIcon,screen,NULL);
		r2.x+=25;
		SDL_BlitSurface(time,NULL,screen,&r2);
		r2.x+=time->w+15;
	}
	if(recordings!=NULL){
		//Now draw the recordings.
		applySurface(r2.x,r2.y,recordingsIcon,screen,NULL);
		r2.x+=25;
		SDL_BlitSurface(recordings,NULL,screen,&r2);
	}
	
	//And free the surfaces.
	SDL_FreeSurface(name);
	SDL_FreeSurface(time);
	SDL_FreeSurface(recordings); 
}

void LevelPlaySelect::GUIEventCallback_OnEvent(std::string name,GUIObject* obj,int eventType){
	//Let the level select handle his GUI events.
	LevelSelect::GUIEventCallback_OnEvent(name,obj,eventType);
	
	//Check for the play button.
	if(name=="cmdPlay"){
		if(selectedNumber!=NULL){
			levels.setCurrentLevel(selectedNumber->getNumber());
			setNextState(STATE_GAME);
			
			//Pick music from the current music list.
			getMusicManager()->pickMusic();
		}
	}
}
