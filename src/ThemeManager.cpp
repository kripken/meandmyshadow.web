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

#include "ThemeManager.h"
#include "POASerializer.h"
#include "Functions.h"
#include "FileManager.h"
#include "Game.h"
#include <string.h>
#include <iostream>
using namespace std;

//The ThemeStack that is be used by the GameState.
ThemeStack objThemes;

bool ThemeManager::loadFile(const string& fileName){
	POASerializer objSerializer;
	TreeStorageNode objNode;

	//First we destroy the current ThemeManager.
	destroy();

	//Now we try to load the file, if it fails we return false.
	if(!objSerializer.loadNodeFromFile(fileName.c_str(),&objNode,true)){
		cerr<<"ERROR: Unable to open theme file: "<<fileName<<endl;
		return false;
	}
	
	//Set the themePath.
	themePath=pathFromFileName(fileName);

	//Retrieve the name of the theme from the file.
	{
		vector<string> &v=objNode.attributes["name"];
		if(v.size()>0) themeName=v[0];
	}
	
	//Loop the subnodes of the theme.
	for(unsigned int i=0;i<objNode.subNodes.size();i++){
		TreeStorageNode *obj=objNode.subNodes[i];
		
		//Check if it's a block or a background.
		if(obj->name=="block" && obj->value.size()>0){
			map<string,int>::iterator it=Game::blockNameMap.find(obj->value[0]);
			if(it!=Game::blockNameMap.end()){
				int idx=it->second;
				if(!objBlocks[idx]) objBlocks[idx]=new ThemeBlock;
				if(!objBlocks[idx]->loadFromNode(obj,themePath)){
					cerr<<"ERROR: Unable to load "<<Game::blockName[idx]<<" for theme "<<fileName<<endl;
					delete objBlocks[idx];
					objBlocks[idx]=NULL;
					return false;
				}
			}
		}else if(obj->name=="background" && obj->value.size()>0){
			if(!objBackground) objBackground=new ThemeBackground();
			if(!objBackground->addPictureFromNode(obj,themePath)){
				cerr<<"ERROR: Unable to load background for theme "<<fileName<<endl;
				delete objBackground;
				objBackground=NULL;
				return false;
			}
		}else if(obj->name=="character" && obj->value.size()>0){
			if(obj->value[0]=="Shadow"){
				if(!shadow) shadow=new ThemeCharacter();
				if(!shadow->loadFromNode(obj,themePath)){
					cerr<<"ERROR: Unable to load shadow for theme "<<fileName<<endl;
					delete shadow;
					shadow=NULL;
					return false;
				}
			}else if(obj->value[0]=="Player"){
				if(!player) player=new ThemeCharacter();
				if(!player->loadFromNode(obj,themePath)){
					cerr<<"ERROR: Unable to load player for theme "<<fileName<<endl;
					delete player;
					player=NULL;
					return false;
				}
			}
		}
	}
	
	//Done and nothing went wrong so return true.
	return true;
}

bool ThemeBlock::loadFromNode(TreeStorageNode* objNode, string themePath){
	destroy();
	
	//Loop the subNodes.
	for(unsigned int i=0;i<objNode->subNodes.size();i++){
		TreeStorageNode *obj=objNode->subNodes[i];
		
		//Check if the subnode is an editorPicture or a blockState.
		if(obj->name=="editorPicture"){
			if(!editorPicture.loadFromNode(obj,themePath)) return false;
		}else if(obj->name=="blockState" && obj->value.size()>0){
			string& s=obj->value[0];
			map<string,ThemeBlockState*>::iterator it=blockStates.find(s);
			if(it==blockStates.end()) blockStates[s]=new ThemeBlockState;
			if(!blockStates[s]->loadFromNode(obj,themePath)) return false;
		}
	}
	
	//Done and nothing went wrong so return true.
	return true;
}

bool ThemeBlockState::loadFromNode(TreeStorageNode* objNode, string themePath){
	destroy();
	
	//Retrieve the oneTimeAnimation attribute.
	{
		vector<string> &v=objNode->attributes["oneTimeAnimation"];
		
		//Check if there are enough values for the oneTimeAnimation attribute.
		if(v.size()>=2 && !v[0].empty()){
			oneTimeAnimationLength=atoi(v[0].c_str());
			nextState=v[1];
		}
	}
	
	//Loop the subNodes.
	for(unsigned int i=0;i<objNode->subNodes.size();i++){
		TreeStorageNode *obj=objNode->subNodes[i];
		if(obj->name=="object"){
			ThemeObject *obj1=new ThemeObject();
			if(!obj1->loadFromNode(obj,themePath)){
				delete obj1;
				return false;
			}
			themeObjects.push_back(obj1);
		}
	}
	
	//Done and nothing went wrong so return true.
	return true;
}

bool ThemeCharacter::loadFromNode(TreeStorageNode* objNode,string themePath){
	destroy();
	
	//Loop the subNodes.
	for(unsigned int i=0;i<objNode->subNodes.size();i++){
		TreeStorageNode *obj=objNode->subNodes[i];
		
		//Check if the subnode is an characterState.
		if(obj->name=="characterState" && obj->value.size()>0){
			string& s=obj->value[0];
			map<string,ThemeCharacterState*>::iterator it=characterStates.find(s);
			if(it==characterStates.end()) characterStates[s]=new ThemeCharacterState;
			if(!characterStates[s]->loadFromNode(obj,themePath)) return false;
		}
	}
	
	//Done and nothing went wrong so return true.
	return true;
}


bool ThemeCharacterState::loadFromNode(TreeStorageNode* objNode,string themePath){
	destroy();
	
	//Retrieve the oneTimeAnimation attribute.
	{
		vector<string> &v=objNode->attributes["oneTimeAnimation"];
		
		//Check if there are enough values for the oneTimeAnimation attribute.
		if(v.size()>=2 && !v[0].empty()){
			oneTimeAnimationLength=atoi(v[0].c_str());
			nextState=v[1];
		}
	}
	
	//Loop the subNodes.
	for(unsigned int i=0;i<objNode->subNodes.size();i++){
		TreeStorageNode *obj=objNode->subNodes[i];
		if(obj->name=="object"){
			ThemeObject *obj1=new ThemeObject();
			if(!obj1->loadFromNode(obj,themePath)){
				delete obj1;
				return false;
			}
			themeObjects.push_back(obj1);
		}
	}
	
	//Done and nothing went wrong so return true.
	return true;
}

bool ThemeObject::loadFromNode(TreeStorageNode* objNode,string themePath){
	destroy();
	
	//Retrieve the animation attribute.
	{
		vector<string> &v=objNode->attributes["animation"];
		if(v.size()>=2){
			animationLength=atoi(v[0].c_str());
			animationLoopPoint=atoi(v[1].c_str());
		}
	}
	//Retrieve the oneTimeAnimation attribute.
	{
		vector<string> &v=objNode->attributes["oneTimeAnimation"];
		if(v.size()>=2){
			animationLength=atoi(v[0].c_str());
			animationLoopPoint=atoi(v[1].c_str())|0x80000000;
		}
	}
	//Retrieve the invisibleAtRunTime attribute.
	{
		vector<string> &v=objNode->attributes["invisibleAtRunTime"];
		if(v.size()>0 && !v[0].empty()){
			invisibleAtRunTime=atoi(v[0].c_str())?true:false;
		}
	}
	//Retrieve the invisibleAtDesignTime attribute.
	{
		vector<string> &v=objNode->attributes["invisibleAtDesignTime"];
		if(v.size()>0 && !v[0].empty()){
			invisibleAtDesignTime=atoi(v[0].c_str())?true:false;
		}
	}
	
	//Loop the subnodes.
	for(unsigned int i=0;i<objNode->subNodes.size();i++){
		TreeStorageNode *obj=objNode->subNodes[i];
		if(obj->name=="picture" || obj->name=="pictureAnimation"){
			if(!picture.loadFromNode(obj,themePath)){
				return false;
			}
		}else if(obj->name=="editorPicture"){
			if(!editorPicture.loadFromNode(obj,themePath)){
				return false;
			}
		}else if(obj->name=="optionalPicture" && obj->value.size()>=6){
			ThemePicture *objPic=new ThemePicture();
			double f=atof(obj->value[5].c_str());
			if(!objPic->loadFromNode(obj,themePath)){
				delete objPic;
				return false;
			}
			optionalPicture.push_back(pair<double,ThemePicture*>(f,objPic));
		}else if(obj->name=="offset" || obj->name=="offsetAnimation"){
			if(!offset.loadFromNode(obj)) return false;
		}
	}
	
	//Done and nothing went wrong so return true.
	return true;
}

bool ThemePicture::loadFromNode(TreeStorageNode* objNode,string themePath){
	destroy();
	
	//Check if the node has enough values.
	if(objNode->value.size()>0){
		//Load teh picture.
		picture=loadImage(themePath+objNode->value[0]);
		if(picture==NULL) return false;
		
		//Check if it's an animation.
		if(objNode->name=="pictureAnimation"){
			if(!offset.loadFromNode(objNode)) return false;
			return true;
		}else if(objNode->value.size()>=5){
			typeOffsetPoint r={atoi(objNode->value[1].c_str()),
				atoi(objNode->value[2].c_str()),
				atoi(objNode->value[3].c_str()),
				atoi(objNode->value[4].c_str()),0,0};
			offset.offsetData.push_back(r);
			offset.length=0;
			return true;
		}
	}
	
	//Done and nothing went wrong so return true.
	return false;
}

bool ThemeOffsetData::loadFromNode(TreeStorageNode* objNode){
	destroy();
	
	//Check what kind of offset it is.
	if(objNode->name=="pictureAnimation"){
		for(unsigned int i=0;i<objNode->subNodes.size();i++){
			TreeStorageNode* obj=objNode->subNodes[i];
			if(obj->name=="point" && obj->value.size()>=4){
				typeOffsetPoint r={atoi(obj->value[0].c_str()),
					atoi(obj->value[1].c_str()),
					atoi(obj->value[2].c_str()),
					atoi(obj->value[3].c_str()),1,1};
				if(obj->value.size()>=5) r.frameCount=atoi(obj->value[4].c_str());
				if(obj->value.size()>=6) r.frameDisplayTime=atoi(obj->value[5].c_str());
				offsetData.push_back(r);
				length+=r.frameCount*r.frameDisplayTime;
			}
		}
		return true;
	}else if(objNode->name=="offsetAnimation"){
		for(unsigned int i=0;i<objNode->subNodes.size();i++){
			TreeStorageNode* obj=objNode->subNodes[i];
			if(obj->name=="point" && obj->value.size()>=2){
				typeOffsetPoint r={atoi(obj->value[0].c_str()),
					atoi(obj->value[1].c_str()),0,0,1,1};
				if(obj->value.size()>=3) r.frameCount=atoi(obj->value[2].c_str());
				if(obj->value.size()>=4) r.frameDisplayTime=atoi(obj->value[3].c_str());
				offsetData.push_back(r);
				length+=r.frameCount*r.frameDisplayTime;
			}
		}
		return true;
	}else if(objNode->name=="offset" && objNode->value.size()>=2){
		typeOffsetPoint r={atoi(objNode->value[0].c_str()),
			atoi(objNode->value[1].c_str()),0,0,0,0};
		offsetData.push_back(r);
		length=0;
		return true;
	}
	
	//Done and nothing went wrong so return true.
	return false;
}

void ThemeObjectInstance::draw(SDL_Surface *dest,int x,int y,SDL_Rect *clipRect){
	//Get the picture.
	SDL_Surface *src=picture->picture;
	if(src==NULL) return;
	int ex=0,ey=0,xx=0,yy=0,ww=0,hh=0;
	int animationNew=animation&0x7FFFFFFF;
	{
		vector<typeOffsetPoint> &v=picture->offset.offsetData;
		if(picture->offset.length==0 || animationNew<v[0].frameDisplayTime){
			xx=v[0].x;
			yy=v[0].y;
			ww=v[0].w;
			hh=v[0].h;
		}else if(animationNew>=picture->offset.length){
			int i=v.size()-1;
			xx=v[i].x;
			yy=v[i].y;
			ww=v[i].w;
			hh=v[i].h;
		}else{
			int t=animationNew-v[0].frameDisplayTime;
			for(unsigned int i=1;i<v.size();i++){
				int tt=t/v[i].frameDisplayTime;
				if(tt>=0 && tt<v[i].frameCount){
					xx=(int)((float)v[i-1].x+(float)(v[i].x-v[i-1].x)*(float)(tt+1)/(float)v[i].frameCount+0.5f);
					yy=(int)((float)v[i-1].y+(float)(v[i].y-v[i-1].y)*(float)(tt+1)/(float)v[i].frameCount+0.5f);
					ww=(int)((float)v[i-1].w+(float)(v[i].w-v[i-1].w)*(float)(tt+1)/(float)v[i].frameCount+0.5f);
					hh=(int)((float)v[i-1].h+(float)(v[i].h-v[i-1].h)*(float)(tt+1)/(float)v[i].frameCount+0.5f);
					break;
				}else{
					t-=v[i].frameCount*v[i].frameDisplayTime;
				}
			}
		}
	}
	//Get the offset.
	{
		vector<typeOffsetPoint> &v=parent->offset.offsetData;
		if(v.empty()){
			ex=0;
			ey=0;
		}else if(parent->offset.length==0 || animationNew<v[0].frameDisplayTime){
			ex=v[0].x;
			ey=v[0].y;
		}else if(animationNew>=parent->offset.length){
			int i=v.size()-1;
			ex=v[i].x;
			ey=v[i].y;
		}else{
			int t=animationNew-v[0].frameDisplayTime;
			for(unsigned int i=1;i<v.size();i++){
				int tt=t/v[i].frameDisplayTime;
				if(tt>=0 && tt<v[i].frameCount){
					ex=(int)((float)v[i-1].x+(float)(v[i].x-v[i-1].x)*(float)(tt+1)/(float)v[i].frameCount+0.5f);
					ey=(int)((float)v[i-1].y+(float)(v[i].y-v[i-1].y)*(float)(tt+1)/(float)v[i].frameCount+0.5f);
					break;
				}else{
					t-=v[i].frameCount*v[i].frameDisplayTime;
				}
			}
		}
	}
	
	//And finally draw the ThemeObjectInstance.
	if(clipRect){
		int d;
		d=clipRect->x-ex;
		if(d>0){
			ex+=d;
			xx+=d;
			ww-=d;
		}
		d=clipRect->y-ey;
		if(d>0){
			ey+=d;
			yy+=d;
			hh-=d;
		}
		if(ww>clipRect->w) ww=clipRect->w;
		if(hh>clipRect->h) hh=clipRect->h;
	}
	if(ww>0&&hh>0){
		SDL_Rect r1={xx,yy,ww,hh};
		SDL_Rect r2={x+ex,y+ey,0,0};
		SDL_BlitSurface(src,&r1,dest,&r2);
	}
}

void ThemeObjectInstance::updateAnimation(){
	//First get the animation length.
	int m;
	m=parent->animationLength;
	
	//If it's higher than 0 then we have an animation.
	if(m>0 && animation>=0){
		//Increase the animation frame.
		animation++;
		//Check if the animation is beyond the length, if so set it to the looppoint.
		if(animation>=m)
			animation=parent->animationLoopPoint;
	}
}

void ThemeBlockInstance::updateAnimation(){
	//Make sure the currentState isn't null.
	if(currentState!=NULL){
		//Call the updateAnimation method of the currentState.
		currentState->updateAnimation();
		
		//Get the length of the animation.
		int m=currentState->parent->oneTimeAnimationLength;
		
		//If it's higher than 0 then we have an animation.
		//Also check if it's past the lenght, meaning done.
		if(m>0 && currentState->animation>=m){
			//Now we can change the state to the nextState.
			changeState(currentState->parent->nextState);
		}
	}
}

void ThemeCharacterInstance::updateAnimation(){
	//Make sure the currentState isn't null.
	if(currentState!=NULL){
		//Call the updateAnimation method of the currentState.
		currentState->updateAnimation();
		
		//Get the length of the animation.
		int m=currentState->parent->oneTimeAnimationLength;
		
		//If it's higher than 0 then we have an animation.
		//Also check if it's past the lenght, meaning done.
		if(m>0 && currentState->animation>=m){
			//Now we can change the state to the nextState.
			changeState(currentState->parent->nextState);
		}
	}
}

void ThemeBlock::createInstance(ThemeBlockInstance* obj){
	//Make sure the given ThemeBlockInstance is ready.
	obj->blockStates.clear();
	obj->currentState=NULL;
	
	//Loop through the blockstates.
	for(map<string,ThemeBlockState*>::iterator it=blockStates.begin();it!=blockStates.end();it++){
		//Get the themeBlockStateInstance of the given ThemeBlockInstance.
		ThemeBlockStateInstance &obj1=obj->blockStates[it->first];
		//Set the parent of the state instance.
		obj1.parent=it->second;
		//Get the vector with themeObjects.
		vector<ThemeObject*> &v=it->second->themeObjects;
		
		//Loop through them.
		for(unsigned int i=0;i<v.size();i++){
			//Create an instance for every one.
			ThemeObjectInstance p;
			//Set the parent.
			p.parent=v[i];
			
			//Choose the picture.
			if(stateID==STATE_LEVEL_EDITOR){
				if(p.parent->invisibleAtDesignTime)
					continue;
				if(p.parent->editorPicture.picture!=NULL)
					p.picture=&p.parent->editorPicture;
			}else{
				if(p.parent->invisibleAtRunTime)
					continue;
			}
			
			//Get the number of optional Pictures.
			int m=p.parent->optionalPicture.size();
			//If p.picture is null, not an editor picture, and there are optional pictures then give one random.
			if(p.picture==NULL && m>0){
				double f=0.0,f1=1.0/256.0;
				for(int j=0;j<8;j++){
					f+=f1*(double)(rand()&0xff);
					f1*=(1.0/256.0);
				}
				for(int j=0;j<m;j++){
					f-=p.parent->optionalPicture[j].first;
					if(f<0.0){
						p.picture=p.parent->optionalPicture[j].second;
						break;
					}
				}
			}
			
			//If random turned out to give nothing then give the non optional picture.
			if(p.picture==NULL && p.parent->picture.picture!=NULL)
				p.picture=&p.parent->picture;
			//If the picture isn't null then can we give it to the ThemeBlockStateInstance.
			if(p.picture!=NULL)
				obj1.objects.push_back(p);
		}
	}
	
	//Change the state to the default one.
	//FIXME: Is that needed?
	obj->changeState("default");
}

void ThemeCharacter::createInstance(ThemeCharacterInstance* obj){
	//Make sure the given ThemeCharacterInstance is ready.
	obj->characterStates.clear();
	obj->currentState=NULL;
	
	//Loop through the characterstates.
	for(map<string,ThemeCharacterState*>::iterator it=characterStates.begin();it!=characterStates.end();it++){
		//Get the themeCharacterStateInstance of the given ThemeCharacterInstance.
		ThemeCharacterStateInstance &obj1=obj->characterStates[it->first];
		//Set the parent of the state instance.
		obj1.parent=it->second;
		//Get the vector with themeObjects.
		vector<ThemeObject*> &v=it->second->themeObjects;
		
		//Loop through them.
		for(unsigned int i=0;i<v.size();i++){
			//Create an instance for every one.
			ThemeObjectInstance p;
			//Set the parent.
			p.parent=v[i];
			
			//Make sure it isn't invisible at runtime.
			if(p.parent->invisibleAtRunTime)
				continue;

			//Get the number of optional Pictures.
			int m=p.parent->optionalPicture.size();
			//If p.picture is null, not an editor picture, and there are optional pictures then give one random.
			if(p.picture==NULL && m>0){
				double f=0.0,f1=1.0/256.0;
				for(int j=0;j<8;j++){
					f+=f1*(double)(rand()&0xff);
					f1*=(1.0/256.0);
				}
				for(int j=0;j<m;j++){
					f-=p.parent->optionalPicture[j].first;
					if(f<0.0){
						p.picture=p.parent->optionalPicture[j].second;
						break;
					}
				}
			}
			
			//If random turned out to give nothing then give the non optional picture.
			if(p.picture==NULL && p.parent->picture.picture!=NULL)
				p.picture=&p.parent->picture;
			//If the picture isn't null then can we give it to the ThemeCharacterStateInstance.
			if(p.picture!=NULL)
				obj1.objects.push_back(p);
		}
	}
	
	//Set it to the standing right state.
	obj->changeState("standright");
}

void ThemePicture::draw(SDL_Surface *dest,int x,int y,int animation,SDL_Rect *clipRect){
	//Get the Picture.
	if(picture==NULL) return;
	int ex=0,ey=0,xx,yy,ww,hh;
	{
		vector<typeOffsetPoint> &v=offset.offsetData;
		if(offset.length==0 || animation<v[0].frameDisplayTime){
			xx=v[0].x;
			yy=v[0].y;
			ww=v[0].w;
			hh=v[0].h;
		}else if(animation>=offset.length){
			int i=v.size()-1;
			xx=v[i].x;
			yy=v[i].y;
			ww=v[i].w;
			hh=v[i].h;
		}else{
			int t=animation-v[0].frameDisplayTime;
			for(unsigned int i=1;i<v.size();i++){
				int tt=t/v[i].frameDisplayTime;
				if(tt>=0 && tt<v[i].frameCount){
					xx=(int)((float)v[i-1].x+(float)(v[i].x-v[i-1].x)*(float)(tt+1)/(float)v[i].frameCount+0.5f);
					yy=(int)((float)v[i-1].y+(float)(v[i].y-v[i-1].y)*(float)(tt+1)/(float)v[i].frameCount+0.5f);
					ww=(int)((float)v[i-1].w+(float)(v[i].w-v[i-1].w)*(float)(tt+1)/(float)v[i].frameCount+0.5f);
					hh=(int)((float)v[i-1].h+(float)(v[i].h-v[i-1].h)*(float)(tt+1)/(float)v[i].frameCount+0.5f);
					break;
				}else{
					t-=v[i].frameCount*v[i].frameDisplayTime;
				}
			}
		}
	}
	
	//Draw the Picture.
	if(clipRect){
		int d;
		d=clipRect->x-ex;
		if(d>0){
			ex+=d;
			xx+=d;
			ww-=d;
		}
		d=clipRect->y-ey;
		if(d>0){
			ey+=d;
			yy+=d;
			hh-=d;
		}
		if(ww>clipRect->w) ww=clipRect->w;
		if(hh>clipRect->h) hh=clipRect->h;
	}
	if(ww>0&&hh>0){
		SDL_Rect r1={xx,yy,ww,hh};
		SDL_Rect r2={x+ex,y+ey,0,0};
		SDL_BlitSurface(picture,&r1,dest,&r2);
	}
}

void ThemeBackgroundPicture::draw(SDL_Surface *dest){
	//Check if the picture is visible.
	if(!(picture&&srcSize.w>0&&srcSize.h>0&&destSize.w>0&&destSize.h>0))
		return;
	
	//Calculate the draw area.
	int sx=(int)((float)destSize.x+currentX-cameraX*(float)camera.x+0.5f);
	int sy=(int)((float)destSize.y+currentY-cameraY*(float)camera.y+0.5f);
	int ex,ey;
	
	//Include repeating.
	if(repeatX){
		sx%=destSize.w;
		if(sx>0) sx-=destSize.w;
		ex=SCREEN_WIDTH;
	}else{
		if(sx<=-(int)destSize.w || sx>=SCREEN_WIDTH) return;
		ex=sx+1;
	}
	if(repeatY){
		sy%=destSize.h;
		if(sy>0) sy-=destSize.h;
		ey=SCREEN_HEIGHT;
	}else{
		if(sy<=-(int)destSize.h || sy>=SCREEN_HEIGHT) return;
		ey=sy+1;
	}
	
	//And finally draw the ThemeBackgroundPicture.
	for(int x=sx;x<ex;x+=destSize.w){
		for(int y=sy;y<ey;y+=destSize.h){
			SDL_Rect r={x,y,0,0};
			SDL_BlitSurface(picture,&srcSize,dest,&r);
		}
	}
}

bool ThemeBackgroundPicture::loadFromNode(TreeStorageNode* objNode,string themePath){
	//Load the picture.
	picture=loadImage(themePath+objNode->value[0]);
	if(picture==NULL) return false;
	
	//Retrieve the source size.
	{
		vector<string> &v=objNode->attributes["srcSize"];
		if(v.size()>=4){
			srcSize.x=atoi(v[0].c_str());
			srcSize.y=atoi(v[1].c_str());
			srcSize.w=atoi(v[2].c_str());
			srcSize.h=atoi(v[3].c_str());
		}else{
			srcSize.x=0;
			srcSize.y=0;
			srcSize.w=picture->w;
			srcSize.h=picture->h;
		}
	}
	
	//Retrieve the destination size.
	{
		vector<string> &v=objNode->attributes["destSize"];
		if(v.size()>=4){
			destSize.x=atoi(v[0].c_str());
			destSize.y=atoi(v[1].c_str());
			destSize.w=atoi(v[2].c_str());
			destSize.h=atoi(v[3].c_str());
		}else{
			destSize.x=0;
			destSize.y=0;
			destSize.w=srcSize.w;
			destSize.h=srcSize.h;
		}
	}
	
	//Retrieve if it should be repeated.
	{
		vector<string> &v=objNode->attributes["repeat"];
		if(v.size()>=2){
			repeatX=atoi(v[0].c_str())?true:false;
			repeatY=atoi(v[1].c_str())?true:false;
		}else{
			repeatX=true;
			repeatY=true;
		}
	}
	
	//Retrieve the speed.
	{
		vector<string> &v=objNode->attributes["speed"];
		if(v.size()>=2){
			speedX=atof(v[0].c_str());
			speedY=atof(v[1].c_str());
		}else{
			speedX=0.0f;
			speedY=0.0f;
		}
	}
	
	//Retrieve the camera speed.
	{
		vector<string> &v=objNode->attributes["cameraSpeed"];
		if(v.size()>=2){
			cameraX=atof(v[0].c_str());
			cameraY=atof(v[1].c_str());
		}else{
			cameraX=0.0f;
			cameraY=0.0f;
		}
	}
	
	//Done and nothing went wrong so return true.
	return true;
}
