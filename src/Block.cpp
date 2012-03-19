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

#include "GameObjects.h"
#include "Game.h"
#include "Player.h"
#include "Block.h"
#include "Functions.h"
#include "Globals.h"
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
using namespace std;

Block::Block(int x,int y,int type,Game* parent):
	GameObject(parent),
	temp(0),
	tempSave(0),
	flags(0),
	flagsSave(0),
	dx(0),
	xSave(0),
	dy(0),
	ySave(0),
	loop(true),
	editorFlags(0)
{
	//First set the location and size of the box.
	//The default size is 50x50.
	box.x=x;
	box.y=y;
	box.w=50;
	box.h=50;

	//Also set the 
	boxBase.x=x;
	boxBase.y=y;
	
	//Set the type.
	this->type=type;
	
	//Some types need type specific code.
	if(type==TYPE_START_PLAYER){
		//This is the player start so set the player here.
		//We center the player, the player is 23px wide.
		parent->player.setPosition(box.x+(box.w-23)/2,box.y);
		parent->player.fx=box.x+(box.w-23)/2;
		parent->player.fy=box.y;
	}else if(type==TYPE_START_SHADOW){
		//This is the shadow start so set the shadow here.
		//We center the shadow, the shadow is 23px wide.
		parent->shadow.setPosition(box.x+(box.w-23)/2,box.y);
		parent->shadow.fx=box.x+(box.w-23)/2;
		parent->shadow.fy=box.y;
	}

	//And load the appearance.
	objThemes.getBlock(type)->createInstance(&appearance);
}

Block::~Block(){}

void Block::show(){
	//Check if the block is visible.
	if(checkCollision(camera,box)==true || (stateID==STATE_LEVEL_EDITOR && checkCollision(camera,boxBase)==true)){
		SDL_Rect r={0,0,50,50};

		//What we need to draw depends on the type of block.
		switch(type){
		case TYPE_CHECKPOINT:
			//Check if the checkpoint is last used.
			if(parent!=NULL && parent->objLastCheckPoint==this){
				if(!temp) appearance.changeState("activated");
				temp=1;
			}else{
				if(temp) appearance.changeState("default");
				temp=0;
			}
			break;
		case TYPE_CONVEYOR_BELT:
		case TYPE_SHADOW_CONVEYOR_BELT:
			if(temp){
				r.x=50-temp;
				r.w=temp;
				appearance.draw(screen,box.x-camera.x-50+temp,box.y-camera.y,&r);
				r.x=0;
				r.w=50-temp;
				appearance.draw(screen,box.x-camera.x+temp,box.y-camera.y,&r);
				return;
			}
			break;
		case TYPE_NOTIFICATION_BLOCK:
			if(message.empty()==false){
				appearance.draw(screen, box.x - camera.x, box.y - camera.y);
				return;
			}
			break;
		}
		
		//Always draw the base.
		appearance.drawState("base", screen, boxBase.x - camera.x, boxBase.y - camera.y);
		//Now draw normal.
		appearance.draw(screen, box.x - camera.x, box.y - camera.y);
		
		//Some types need to draw something on top of the base/default.
		switch(type){
		case TYPE_BUTTON:
			if(flags&4){
				if(temp<5) temp++;
			}else{
				if(temp>0) temp--;
			}
			appearance.drawState("button",screen,box.x-camera.x,box.y-camera.y-5+temp);
			break;
		}
	}
}

SDL_Rect Block::getBox(int boxType){
	SDL_Rect r={0,0,0,0};
	switch(boxType){
	case BoxType_Base:
		return boxBase;
	case BoxType_Previous:
		switch(type){
		case TYPE_MOVING_BLOCK:
		case TYPE_MOVING_SHADOW_BLOCK:
		case TYPE_MOVING_SPIKES:
			r.x=box.x-dx;
			r.y=box.y-dy;
			r.w=box.w;
			r.h=box.h;
			return r;
		}
		return box;
	case BoxType_Delta:
		switch(type){
		case TYPE_MOVING_BLOCK:
		case TYPE_MOVING_SHADOW_BLOCK:
		case TYPE_MOVING_SPIKES:
			r.x=dx;
			r.y=dy;
			break;
		}
		return r;
	case BoxType_Velocity:
		switch(type){
		case TYPE_MOVING_BLOCK:
		case TYPE_MOVING_SHADOW_BLOCK:
		case TYPE_MOVING_SPIKES:
			r.x=dx;
			r.y=dy;
			break;
		case TYPE_CONVEYOR_BELT:
		case TYPE_SHADOW_CONVEYOR_BELT:
			r.x=(flags&1)?0:dx;
			break;
		}
		return r;
	case BoxType_Current:
		return box;
	}
	return r;
}

void Block::setPosition(int x,int y){
	box.x=x;
	box.y=y;
	boxBase.x=x;
	boxBase.y=y;
}

void Block::saveState(){
	tempSave=temp;
	flagsSave=flags;
	xSave=box.x-boxBase.x;
	ySave=box.y-boxBase.y;
	appearance.saveAnimation();
}

void Block::loadState(){
	temp=tempSave;
	flags=flagsSave;
	switch(type){
	case TYPE_MOVING_BLOCK:
	case TYPE_MOVING_SHADOW_BLOCK:
		box.x=boxBase.x+xSave;
		box.y=boxBase.y+ySave;
		break;
	}
	appearance.loadAnimation();
}

void Block::reset(bool save){
	//We need to reset so we clear the temp and saves.
	if(save){
		temp=tempSave=xSave=ySave=0;
		flags=flagsSave=editorFlags;
	}else{
		temp=0;
		flags=editorFlags;
	}

	switch(type){
	case TYPE_MOVING_BLOCK:
	case TYPE_MOVING_SHADOW_BLOCK:
	case TYPE_MOVING_SPIKES:
		box.x=boxBase.x;
		box.y=boxBase.y;
		break;
	}
	
	//Also reset the appearance.
	appearance.resetAnimation(save);
	appearance.changeState("default");
	
	//If it's a fragile block we need to update the appearance.
	switch(type){
	case TYPE_FRAGILE:
		{
			const char* s=(flags==0)?"default":((flags==1)?"fragile1":((flags==2)?"fragile2":"fragile3"));
			appearance.changeState(s);
		}
		break;
	}
}


void Block::playAnimation(int flags){
	//TODO Why int flags????
	switch(type){
	case TYPE_SWAP:
		appearance.changeState("activated");
		break;
	case TYPE_SWITCH:
		temp^=1;
		appearance.changeState(temp?"activated":"default");
		break;
	}
}

void Block::onEvent(int eventType){
	//Event handling.
	switch(eventType){
	case GameObjectEvent_PlayerWalkOn:
		switch(type){
		case TYPE_FRAGILE:
			flags++;
			{
				const char* s=(flags==0)?"default":((flags==1)?"fragile1":((flags==2)?"fragile2":"fragile3"));
				appearance.changeState(s);
			}
			break;
		}
		break;
	case GameObjectEvent_PlayerIsOn:
		switch(type){
		case TYPE_BUTTON:
			dx=1;
			break;
		}
		break;
	case GameObjectEvent_OnToggle:
		switch(type){
		case TYPE_MOVING_BLOCK:
		case TYPE_MOVING_SHADOW_BLOCK:
		case TYPE_MOVING_SPIKES:
		case TYPE_CONVEYOR_BELT:
		case TYPE_SHADOW_CONVEYOR_BELT:
			flags^=1;
			break;
		case TYPE_PORTAL:
			appearance.changeState("activated");
			break;
		}
		break;
	case GameObjectEvent_OnSwitchOn:
		switch(type){
		case TYPE_MOVING_BLOCK:
		case TYPE_MOVING_SHADOW_BLOCK:
		case TYPE_MOVING_SPIKES:
		case TYPE_CONVEYOR_BELT:
		case TYPE_SHADOW_CONVEYOR_BELT:
			flags&=~1;
			break;
		}
		break;
	case GameObjectEvent_OnSwitchOff:
		switch(type){
		case TYPE_MOVING_BLOCK:
		case TYPE_MOVING_SHADOW_BLOCK:
		case TYPE_MOVING_SPIKES:
		case TYPE_CONVEYOR_BELT:
		case TYPE_SHADOW_CONVEYOR_BELT:
			flags|=1;
			break;
		}
		break;
	}
}

int Block::queryProperties(int propertyType,Player* obj){
	switch(propertyType){
	case GameObjectProperty_PlayerCanWalkOn:
		switch(type){
		case TYPE_BLOCK:
		case TYPE_MOVING_BLOCK:
		case TYPE_CONVEYOR_BELT:
		case TYPE_BUTTON:
			return 1;
		case TYPE_SHADOW_BLOCK:
		case TYPE_MOVING_SHADOW_BLOCK:
		case TYPE_SHADOW_CONVEYOR_BELT:
			if(obj!=NULL && obj->isShadow()) return 1;
			break;
		case TYPE_FRAGILE:
			if(flags<3) return 1;
			break;
		}
		break;
	case GameObjectProperty_IsSpikes:
		switch(type){
		case TYPE_SPIKES:
		case TYPE_MOVING_SPIKES:
			return 1;
		}
		break;
	case GameObjectProperty_Flags:
		return flags;
		break;
	default:
		break;
	}
	return 0;
}

void Block::getEditorData(std::vector<std::pair<std::string,std::string> >& obj){
	//Every block has an id.
	obj.push_back(pair<string,string>("id",id));

	//Block specific properties.
	switch(type){
	case TYPE_MOVING_BLOCK:
	case TYPE_MOVING_SHADOW_BLOCK:
	case TYPE_MOVING_SPIKES:
		{
			char s[64],s0[64];
			sprintf(s,"%d",(int)movingPos.size());
			obj.push_back(pair<string,string>("MovingPosCount",s));
			obj.push_back(pair<string,string>("disabled",(editorFlags&0x1)?"1":"0"));
			obj.push_back(pair<string,string>("loop",loop?"1":"0"));
			for(unsigned int i=0;i<movingPos.size();i++){
				sprintf(s0+1,"%d",i);
				sprintf(s,"%d",movingPos[i].x);
				s0[0]='x';
				obj.push_back(pair<string,string>(s0,s));
				sprintf(s,"%d",movingPos[i].y);
				s0[0]='y';
				obj.push_back(pair<string,string>(s0,s));
				sprintf(s,"%d",movingPos[i].w);
				s0[0]='t';
				obj.push_back(pair<string,string>(s0,s));
			}
		}
		break;
	case TYPE_CONVEYOR_BELT:
	case TYPE_SHADOW_CONVEYOR_BELT:
		{
			char s[64];
			obj.push_back(pair<string,string>("disabled",(editorFlags&0x1)?"1":"0"));
			sprintf(s,"%d",dx);
			obj.push_back(pair<string,string>("speed",s));
		}
		break;
	case TYPE_PORTAL:
		obj.push_back(pair<string,string>("automatic",(editorFlags&0x1)?"1":"0"));
		obj.push_back(pair<string,string>("destination",destination));
		break;
	case TYPE_BUTTON:
	case TYPE_SWITCH:
		{
			string s;
			switch(editorFlags&0x3){
			case 1:
				s="on";
				break;
			case 2:
				s="off";
				break;
			default:
				s="toggle";
				break;
			}
			obj.push_back(pair<string,string>("behaviour",s));
		}
		break;
	case TYPE_NOTIFICATION_BLOCK:
		{
			string value=message;
			//Change \n with the characters '\n'.
			while(value.find('\n',0)!=string::npos){
				size_t pos=value.find('\n',0);
				value=value.replace(pos,1,"\\n");
			}
			
			obj.push_back(pair<string,string>("message",value));
		}
		break;
	case TYPE_FRAGILE:
		{
			char s[64];
			sprintf(s,"%d",editorFlags);
			obj.push_back(pair<string,string>("state",s));
		}
		break;
	}
}

void Block::setEditorData(std::map<std::string,std::string>& obj){
	//Iterator used to check if the map contains certain entries.
	map<string,string>::iterator it;

	//Check if the data contains the id block.
	it=obj.find("id");
	if(it!=obj.end()){
		//Set the id of the block.
		id=obj["id"];
	}

	//Block specific properties.
	switch(type){
	case TYPE_MOVING_BLOCK:
	case TYPE_MOVING_SHADOW_BLOCK:
	case TYPE_MOVING_SPIKES:
		{
			//Make sure that the editor data contains MovingPosCount.
			it=obj.find("MovingPosCount");
			if(it!=obj.end()){
				char s0[64];
				int m=0;
				m=atoi(obj["MovingPosCount"].c_str());
				movingPos.clear();
				for(int i=0;i<m;i++){
					SDL_Rect r={0,0,0,0};
					sprintf(s0+1,"%d",i);
					s0[0]='x';
					r.x=atoi(obj[s0].c_str());
					s0[0]='y';
					r.y=atoi(obj[s0].c_str());
					s0[0]='t';
					r.w=atoi(obj[s0].c_str());
					movingPos.push_back(r);
				}
			}
			
			//Check if the disabled key is in the data.
			it=obj.find("disabled");
			if(it!=obj.end()){
				string s=obj["disabled"];
				editorFlags=0;
				if(s=="true" || atoi(s.c_str())) editorFlags|=0x1;
				flags=flagsSave=editorFlags;
			}
			
			//Check if the loop key is in the data.
			it=obj.find("loop");
			if(it!=obj.end()){
				string s=obj["loop"];
				loop=false;
				if(s=="true" || atoi(s.c_str()))
					loop=true;
			}

		}
		break;
	case TYPE_CONVEYOR_BELT:
	case TYPE_SHADOW_CONVEYOR_BELT:
		{
			//Check if there's a speed key in the editor data.
			it=obj.find("speed");
			if(it!=obj.end()){
				dx=atoi(obj["speed"].c_str());
			}
			
			//Check if the disabled key is in the data.
			it=obj.find("disabled");
			if(it!=obj.end()){
				string s=obj["disabled"];
				editorFlags=0;
				if(s=="true" || atoi(s.c_str())) editorFlags|=0x1;
				flags=flagsSave=editorFlags;
			}
		}
		break;
	case TYPE_PORTAL:
		{
			//Check if the automatic key is in the data.
			it=obj.find("automatic");
			if(it!=obj.end()){
				string s=obj["automatic"];
				editorFlags=0;
				if(s=="true" || atoi(s.c_str())) editorFlags|=0x1;
				flags=flagsSave=editorFlags;
			}
			
			//Check if the destination key is in the data.
			it=obj.find("destination");
			if(it!=obj.end()){
				destination=obj["destination"];
			}
		}
		break;
	case TYPE_BUTTON:
	case TYPE_SWITCH:
		{
			//Check if the behaviour key is in the data.
			it=obj.find("behaviour");
			if(it!=obj.end()){
				string s=obj["behaviour"];
				editorFlags=0;
				if(s=="on" || s=="On") editorFlags|=1;
				else if(s=="off" || s=="Off") editorFlags|=2;
				flags=flagsSave=editorFlags;
			}
		}
		break;
	case TYPE_NOTIFICATION_BLOCK:
		{
			//Check if the message key is in the data.
			it=obj.find("message");
			if(it!=obj.end()){
				message=obj["message"];
				//Change the characters '\n' to a real \n
				while(message.find("\\n")!=string::npos){
					message=message.replace(message.find("\\n"),2,"\n");
				}
			}
		}
		break;
	case TYPE_FRAGILE:
		{
			//Check if the status is in the data.
			it=obj.find("state");
			if(it!=obj.end()){
				editorFlags=atoi(obj["state"].c_str());
				flags=editorFlags;
				{
					const char* s=(flags==0)?"default":((flags==1)?"fragile1":((flags==2)?"fragile2":"fragile3"));
					appearance.changeState(s);
				}
			}
		}
	}
}

void Block::move(){
	appearance.updateAnimation();
	switch(type){
	case TYPE_MOVING_BLOCK:
	case TYPE_MOVING_SHADOW_BLOCK:
	case TYPE_MOVING_SPIKES:
		{
			if(!(flags&0x1)) temp++;
			int t=temp;
			SDL_Rect r0={0,0,0,0},r1;
			dx=0;
			dy=0;
			for(unsigned int i=0;i<movingPos.size();i++){
				r1.x=movingPos[i].x;
				r1.y=movingPos[i].y;
				r1.w=movingPos[i].w;
				if(t==0&&r1.w==0){
					r1.w=1;
					flags|=0x1;
				}
				if(t>=0 && t<(int)r1.w){
					int newX=boxBase.x+(int)(float(r0.x)+(float(r1.x)-float(r0.x))*float(t)/float(r1.w));
					int newY=boxBase.y+(int)(float(r0.y)+(float(r1.y)-float(r0.y))*float(t)/float(r1.w));
					dx=newX-box.x;
					dy=newY-box.y;
					box.x=newX;
					box.y=newY;
					return;
				}else if(t==(int)r1.w){
					//If the time is the time of the movingPosition then set it equal to the location.
					//We do this to prevent a slight edge between normal blocks and moving blocks.
					int newX=boxBase.x+r1.x;
					int newY=boxBase.y+r1.y;
					dx=newX-box.x;
					dy=newY-box.y;
					box.x=newX;
					box.y=newY;
					return;
				}
				t-=r1.w;
				r0.x=r1.x;
				r0.y=r1.y;
			}
			//Only reset the stuff when we're looping.
			if(loop){
				temp=0;
				if(movingPos.size()>0 && movingPos.back().x==0 && movingPos.back().y==0){
					dx=boxBase.x-box.x;
					dy=boxBase.y-box.y;
				}
				box.x=boxBase.x;
				box.y=boxBase.y;
			}
		}
		break;
	case TYPE_BUTTON:
		{
			int new_flags=dx?4:0;
			if((flags^new_flags)&4){
				flags=(flags&~4)|new_flags;
				if(parent && (new_flags || (flags&3)==0)){
					//Make sure that id isn't empty.
					if(!id.empty()){
						parent->broadcastObjectEvent(0x10000|(flags&3),-1,id.c_str());
					}else{
						cerr<<"Warning: invalid button id!"<<endl;
					}
				}
			}
			dx=0;
		}
		break;
	case TYPE_CONVEYOR_BELT:
	case TYPE_SHADOW_CONVEYOR_BELT:
		if((flags&1)==0){
			temp=(temp+dx)%50;
			if(temp<0) temp+=50;
		}
		break;
	}
}
