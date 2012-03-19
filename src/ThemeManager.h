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

#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include "Globals.h"
#include "TreeStorageNode.h"
#include <string.h>
#include <math.h>
#include <string>
#include <vector>
#include <utility>
#include <iostream>
using namespace std;

//Structure containing offset data for one frame.
struct typeOffsetPoint{
	//The location (x,y) and size (w,h).
	int x,y,w,h;
	//The frame to which this offset applies.
	int frameCount;
	//The number of frames this offset is shown.
	int frameDisplayTime;
};

//We already need the classes so declare them here.
class ThemeOffsetData;
class ThemePicture;
class ThemeObject;
class ThemeBlockState;
class ThemeBlock;
class ThemeCharacterState;
class ThemeCharacter;


//Instance class of a ThemeObject, this is used by the other Instance classes.
class ThemeObjectInstance{
public:
	//Pointer to the picture.
	ThemePicture* picture;
	//Pointer to the parent the object an instance os is.
	ThemeObject* parent;
	
	//Integer containing the current animation frame.
	int animation;
	//Integer containing the saved animation frame.
	int savedAnimation;
public:
	//Constructor.
	ThemeObjectInstance():picture(NULL),parent(NULL),animation(0),savedAnimation(0){}
	
	//Method used to draw the ThemeObject.
	//dest: The destination surface to draw the ThemeObject on.
	//x: The x location on the dest surface.
	//y: The y location on the dest surface.
	//clipRect: Rectangle used to clip.
	void draw(SDL_Surface* dest,int x,int y,SDL_Rect* clipRect=NULL);
	
	//Method that will update the animation.
	void updateAnimation();
	
	//Method that will reset the animation.
	//save: Boolean if the saved animation should be deleted.
	void resetAnimation(bool save){
		animation=0;
		if(save){
			savedAnimation=0;
		}
	}
	//Method that will save the animation.
	void saveAnimation(){
		savedAnimation=animation;
	}
	//Method that will load a saved animation.
	void loadAnimation(){
		animation=savedAnimation;
	}
};

//Instance class of a ThemeBlockState, this is used by the ThemeBlockInstance.
class ThemeBlockStateInstance{
public:
	//Pointer to the parent the state an instance of is.
	ThemeBlockState *parent;
	//Vector containing the ThemeObjectInstances.
	vector<ThemeObjectInstance> objects;
	
	//Integer containing the current animation frame.
	int animation;
	//Integer containing the saved animation frame.
	int savedAnimation;
public:
	//Constructor.
	ThemeBlockStateInstance():parent(NULL),animation(0){}
	
	//Method used to draw the ThemeBlockState.
	//dest: The destination surface to draw the ThemeBlockState on.
	//x: The x location on the dest surface.
	//y: The y location on the dest surface.
	//clipRect: Rectangle used to clip.
	void draw(SDL_Surface *dest,int x,int y,SDL_Rect *clipRect=NULL){
		for(unsigned int i=0;i<objects.size();i++){
			objects[i].draw(dest,x,y,clipRect);
		}
	}
	
	//Method that will update the animation.
	void updateAnimation(){
		for(unsigned int i=0;i<objects.size();i++){
			objects[i].updateAnimation();
		}
		animation++;
	}
	//Method that will reset the animation.
	//save: Boolean if the saved state should be deleted.
	void resetAnimation(bool save){
		for(unsigned int i=0;i<objects.size();i++){
			objects[i].resetAnimation(save);
		}
		animation=0;
		if(save){
			savedAnimation=0;
		}
	}
	//Method that will save the animation.
	void saveAnimation(){
		for(unsigned int i=0;i<objects.size();i++){
			objects[i].saveAnimation();
		}
		savedAnimation=animation;
	}
	//Method that will load a saved animation.
	void loadAnimation(){
		for(unsigned int i=0;i<objects.size();i++){
			objects[i].loadAnimation();
		}
		animation=savedAnimation;
	}
};

//Instance of a ThemeBlock, this is used by blocks in the game to prevent changing the theme in game.
//It also allows animation to run independently.
class ThemeBlockInstance{
public:
	//Pointer to the current state.
	ThemeBlockStateInstance* currentState;
	//The name of the current state.
	string currentStateName;
	
	//Map containing the blockStates.
	map<string,ThemeBlockStateInstance> blockStates;
	//String containing the name of the saved state.
	string savedStateName;
public:
	//Constructor.
	ThemeBlockInstance():currentState(NULL){}
	
	//Method used to draw the ThemeBlock.
	//dest: The destination surface to draw the ThemeBlock on.
	//x: The x location on the dest surface.
	//y: The y location on the dest surface.
	//clipRect: Rectangle used to clip.
	//Returns: True if it succeeds.
	bool draw(SDL_Surface *dest,int x,int y,SDL_Rect *clipRect=NULL){
		if(currentState!=NULL){
			currentState->draw(dest,x,y,clipRect);
			return true;
		}
		return false;
	}
	//Method that will draw a specific state.
	//s: The name of the state to draw.
	//dest: The destination surface to draw the ThemeBlock on.
	//x: The x location on the dest surface.
	//y: The y location on the dest surface.
	//clipRect: Rectangle used to clip.
	//Returns: True if it succeeds.
	bool drawState(const string& s,SDL_Surface *dest,int x,int y,SDL_Rect *clipRect=NULL){
		map<string,ThemeBlockStateInstance>::iterator it=blockStates.find(s);
		if(it!=blockStates.end()){
			it->second.draw(dest,x,y,clipRect);
			return true;
		}
		return false;
	}
	
	//Method that will change the current state.
	//s: The name of the state to change to.
	//reset: Boolean if the animation should reset.
	//Returns: True if it succeeds (exists).
	bool changeState(const string& s,bool reset=true){
		//Get the new state.
		map<string,ThemeBlockStateInstance>::iterator it=blockStates.find(s);
		//Check if it exists.
		if(it!=blockStates.end()){
			//Set the state.
			currentState=&(it->second);
			currentStateName=it->first;
			
			//FIXME: Is it needed to set the savedStateName here?
			if(savedStateName.empty())
				savedStateName=currentStateName;
			
			//If reset then reset the animation.
			if(reset)
				currentState->resetAnimation(true);
			return true;
		}
		
		//It doesn't so return false.
		return false;
	}
	
	//Method that will update the animation.
	void updateAnimation();
	//Method that will reset the animation.
	//save: Boolean if the saved state should be deleted.
	void resetAnimation(bool save){
		for(map<string,ThemeBlockStateInstance>::iterator it=blockStates.begin();it!=blockStates.end();it++){
			it->second.resetAnimation(save);
		}
		if(save){
			savedStateName.clear();
		}
	}
	//Method that will save the animation.
	void saveAnimation(){
		for(map<string,ThemeBlockStateInstance>::iterator it=blockStates.begin();it!=blockStates.end();it++){
			it->second.saveAnimation();
		}
		savedStateName=currentStateName;
	}
	//Method that will restore a saved animation.
	void loadAnimation(){
		for(map<string,ThemeBlockStateInstance>::iterator it=blockStates.begin();it!=blockStates.end();it++){
			it->second.loadAnimation();
		}
		changeState(savedStateName,false);
	}
};

//Instance class of a ThemeCharacterState, this is used by the ThemeCharacterInstance.
class ThemeCharacterStateInstance{
public:
	//Pointer to the parent the state an instance of is.
	ThemeCharacterState* parent;
	//Vector containing the ThemeObjectInstances.
	vector<ThemeObjectInstance> objects;
	
	//Integer containing the current animation frame.
	int animation;
	//Integer containing the saved animation frame.
	int savedAnimation;
public:
	//Constructor.
	ThemeCharacterStateInstance():parent(NULL),animation(0){}
	
	//Method used to draw the ThemeCharacterState.
	//dest: The destination surface to draw the ThemeCharacterState on.
	//x: The x location on the dest surface.
	//y: The y location on the dest surface.
	//clipRect: Rectangle used to clip.
	void draw(SDL_Surface *dest,int x,int y,SDL_Rect *clipRect=NULL){
		for(unsigned int i=0;i<objects.size();i++){
			objects[i].draw(dest,x,y,clipRect);
		}
	}
	
	//Method that will update the animation.
	void updateAnimation(){
		for(unsigned int i=0;i<objects.size();i++){
			objects[i].updateAnimation();
		}
		animation++;
	}
	//Method that will reset the animation.
	//save: Boolean if the saved state should be deleted.
	void resetAnimation(bool save){
		for(unsigned int i=0;i<objects.size();i++){
			objects[i].resetAnimation(save);
		}
		animation=0;
		if(save)
			savedAnimation=0;
	}
	//Method that will save the animation.
	void saveAnimation(){
		for(unsigned int i=0;i<objects.size();i++){
			objects[i].saveAnimation();
		}
		savedAnimation=animation;
	}
	//Method that will load a saved animation.
	void loadAnimation(){
		for(unsigned int i=0;i<objects.size();i++){
			objects[i].loadAnimation();
		}
		animation=savedAnimation;
	}
};

//Instance of a ThemeCharacter.
class ThemeCharacterInstance{
public:
	//Pointer to the current state.
	ThemeCharacterStateInstance* currentState;
	//The name of the current state.
	string currentStateName;
	
	//Map containing the ThemeCharacterStates.
	map<string,ThemeCharacterStateInstance> characterStates;
	//String containing the name of the saved state.
	string savedStateName;
public:
	//Constructor.
	ThemeCharacterInstance():currentState(NULL){}
	
	//Method used to draw the ThemeCharacter.
	//dest: The destination surface to draw the ThemeCharacter on.
	//x: The x location on the dest surface.
	//y: The y location on the dest surface.
	//clipRect: Rectangle used to clip.
	//Returns: True if it succeeds.
	bool draw(SDL_Surface *dest,int x,int y,SDL_Rect *clipRect=NULL){
		if(currentState!=NULL){
			currentState->draw(dest,x,y,clipRect);
			return true;
		}
		return false;
	}
	//Method that will draw a specific state.
	//s: The name of the state to draw.
	//dest: The destination surface to draw the ThemeCharacter on.
	//x: The x location on the dest surface.
	//y: The y location on the dest surface.
	//clipRect: Rectangle used to clip.
	//Returns: True if it succeeds.
	bool drawState(const string& s,SDL_Surface *dest,int x,int y,SDL_Rect *clipRect=NULL){
		map<string,ThemeCharacterStateInstance>::iterator it=characterStates.find(s);
		if(it!=characterStates.end()){
			it->second.draw(dest,x,y,clipRect);
			return true;
		}
		return false;
	}
	
	//Method that will change the current state.
	//s: The name of the state to change to.
	//reset: Boolean if the animation should reset.
	//Returns: True if it succeeds (exists).
	bool changeState(const string& s,bool reset=true){
		//Get the new state.
		map<string,ThemeCharacterStateInstance>::iterator it=characterStates.find(s);
		//Check if it exists.
		if(it!=characterStates.end()){
			//Set the state.
			currentState=&(it->second);
			currentStateName=it->first;
			
			//FIXME: Is it needed to set the savedStateName here?
			if(savedStateName.empty())
				savedStateName=currentStateName;
			
			//If reset then reset the animation.
			if(reset)
				currentState->resetAnimation(true);
			return true;
		}
		
		//It doesn't so return false.
		return false;
	}
	
	//Method that will update the animation.
	void updateAnimation();
	//Method that will reset the animation.
	//save: Boolean if the saved state should be deleted.
	void resetAnimation(bool save){
		for(map<string,ThemeCharacterStateInstance>::iterator it=characterStates.begin();it!=characterStates.end();it++){
			it->second.resetAnimation(save);
		}
		if(save)
			savedStateName.clear();
	}
	//Method that will save the animation.
	void saveAnimation(){
		for(map<string,ThemeCharacterStateInstance>::iterator it=characterStates.begin();it!=characterStates.end();it++){
			it->second.saveAnimation();
		}
		savedStateName=currentStateName;
	}
	//Method that will restore a saved animation.
	void loadAnimation(){
		for(map<string,ThemeCharacterStateInstance>::iterator it=characterStates.begin();it!=characterStates.end();it++){
			it->second.loadAnimation();
		}
		changeState(savedStateName,false);
	}
};

//Class containing the offset data.
class ThemeOffsetData{
public:
	//Vector containing the offsetDatas.
	vector<typeOffsetPoint> offsetData;
	//The length of the "animation" in frames.
	int length;
public:
	//Constructor.
	ThemeOffsetData():length(0){}
	//Destructor.
	~ThemeOffsetData(){}
	
	//Method used to destroy the offsetData.
	void destroy(){
		//Set length to zero.
		length=0;
		//And clear the offsetData vector.
		offsetData.clear();
	}
	
	//Method that will load the offsetData from a node.
	//objNode: Pointer to the TreeStorageNode to read the data from.
	//Returns: True if it succeeds without errors.
	bool loadFromNode(TreeStorageNode* objNode);
};

//This is the lowest level of the theme system.
//It's a picture with offset data.
class ThemePicture{
public:
	//The SDL_Surface containing the picture.
	SDL_Surface* picture;
	//Offset data for the picture.
	ThemeOffsetData offset;
public:
	//Constructor.
	ThemePicture():picture(NULL){}
	//Destructor.
	~ThemePicture(){}
	
	//Method used to destroy the picture.
	void destroy(){
		//FIXME: Shouldn't the image be freed? (ImageManager)
		picture=NULL;
		//Destroy the offset data.
		offset.destroy();
	}
	bool loadFromNode(TreeStorageNode* objNode, string themePath);
	
	//Method that will draw the ThemePicture.
	//dest: The destination surface.
	//x: The x location on the dest to draw the picture.
	//y: The y location on the dest to draw the picture.
	//animation: The frame of the animation to draw.
	//clipRect: Rectangle to clip the picture.
	void draw(SDL_Surface* dest,int x,int y,int animation=0,SDL_Rect* clipRect=NULL);
};

//The ThemeObject class is used to contain a basic theme element.
//Contains the picture, animation information, etc...
class ThemeObject{
public:
	//Integer containing the length of the animation.
	int animationLength;
	//Integer containing the frame from where the animation is going to loop.
	int animationLoopPoint;
	
	//Boolean if the animation is invisible at run time (Game state).
	bool invisibleAtRunTime;
	//Boolean if the animation is invisible at design time (Level editor).
	bool invisibleAtDesignTime;
	
	//Picture of the ThemeObject.
	ThemePicture picture;
	//Picture of the ThemeObject shown when in the level editor.
	ThemePicture editorPicture;
	
	//Vector containing optionalPicture for the ThemeObject.
	vector<pair<double,ThemePicture*> > optionalPicture;
	
	//ThemeOffsetData for the ThemeObject.
	ThemeOffsetData offset;
public:
	//Constructor.
	ThemeObject():animationLength(0),animationLoopPoint(0),invisibleAtRunTime(false),invisibleAtDesignTime(false){}
	//Destructor.
	~ThemeObject(){
		//Loop through the optionalPicture and delete them.
		for(unsigned int i=0;i<optionalPicture.size();i++){
			delete optionalPicture[i].second;
		}
	}
	
	//Method that will destroy the ThemeObject.
	void destroy(){
		//Loop through the optionalPicture and delete them.
		for(unsigned int i=0;i<optionalPicture.size();i++){
			delete optionalPicture[i].second;
		}
		optionalPicture.clear();
		animationLength=0;
		animationLoopPoint=0;
		invisibleAtRunTime=false;
		invisibleAtDesignTime=false;
		picture.destroy();
		editorPicture.destroy();
		offset.destroy();
	}
	
	//Method that will load a ThemeObject from a node.
	//objNode: The TreeStorageNode to read the object from.
	//themePath: Path to the theme.
	//Returns: True if it succeeds.
	bool loadFromNode(TreeStorageNode* objNode,string themePath);
};

//Class containing a single state of a themed block.
class ThemeBlockState{
public:
	//The length in frames of the oneTimeAnimation.
	int oneTimeAnimationLength;
	//String containing the name of the next state.
	string nextState;
	//Vector containing the themeObjects that make up this state.
	vector<ThemeObject*> themeObjects;
public:
	//Constructor.
	ThemeBlockState():oneTimeAnimationLength(0){}
	//Destructor.
	~ThemeBlockState(){
		//Loop through the ThemeObjects and delete them.
		for(unsigned int i=0;i<themeObjects.size();i++){
			delete themeObjects[i];
		}
	}
	
	//Method that will destroy the ThemeBlockState.
	void destroy(){
		//Loop through the ThemeObjects and delete them.
		for(unsigned int i=0;i<themeObjects.size();i++){
			delete themeObjects[i];
		}
		//Clear the themeObjects vector.
		themeObjects.clear();
		//Set the length to 0.
		oneTimeAnimationLength=0;
		//Clear the nextState string.
		nextState.clear();
	}
	
	//Method that will load a ThemeBlockState from a node.
	//objNode: The TreeStorageNode to read the state from.
	//themePath: Path to the theme.
	//Returns: True if it succeeds.
	bool loadFromNode(TreeStorageNode* objNode,string themePath);
};

//Class containing the needed things for a themed block.
class ThemeBlock{
public:
	//Picture that is shown only in the level editor.
	ThemePicture editorPicture;
	
	//Map containing ThemeBlockStates for the different states of a block.
	map<string,ThemeBlockState*> blockStates;
public:
	//Constructor.
	ThemeBlock(){}
	//Destructor/
	~ThemeBlock(){
		//Loop through the ThemeBlockStates and delete them,
		for(map<string,ThemeBlockState*>::iterator i=blockStates.begin();i!=blockStates.end();i++){
			delete i->second;
		}
	}
	
	//Method that will destroy the ThemeBlock.
	void destroy(){
		//Loop through the ThemeBlockStates and delete them,
		for(map<string,ThemeBlockState*>::iterator i=blockStates.begin();i!=blockStates.end();i++){
			delete i->second;
		}
		//Clear the blockStates map.
		blockStates.clear();
		editorPicture.destroy();
	}
	
	//Method that will load a ThemeBlock from a node.
	//objNode: The TreeStorageNode to load the ThemeBlock from.
	//themePath: The path to the theme.
	//Returns: True if it succeeds.
	bool loadFromNode(TreeStorageNode* objNode,string themePath);
	
	//Method that will create a ThemeBlockInstance.
	//obj: Pointer that will be filled with the instance.
	void createInstance(ThemeBlockInstance* obj);
};

//Class containing one state of a ThemeCharacter
class ThemeCharacterState{
public:
	//The length in frames of the oneTimeAnimation.
	int oneTimeAnimationLength;
	//String containing the name of the next id.
	string nextState;
	//Vector with the themeObjects in the character state.
	vector<ThemeObject*> themeObjects;
public:
	//Constructor.
	ThemeCharacterState():oneTimeAnimationLength(0){}
	//Destructor.
	~ThemeCharacterState(){
		//Loop through the themeObjects and delete them.
		for(unsigned int i=0;i<themeObjects.size();i++){
			delete themeObjects[i];
		}
	}
	
	//Method used to destroy the ThemeCharacterState.
	void destroy(){
		//Loop through the themeObjects and delete them.
		for(unsigned int i=0;i<themeObjects.size();i++){
			delete themeObjects[i];
		}
		//Clear the themeObjects vector.
		themeObjects.clear();
		//Set oneTimeAnimation to zero.
		oneTimeAnimationLength=0;
		//Clear the nextState string.
		nextState.clear();
	}
	
	//Method that will load the ThemeCharacterState from a node.
	//objNode: The TreeStorageNode to load the state from.
	//themePath: Path to the theme.
	//Returns: True if it succeeds.
	bool loadFromNode(TreeStorageNode* objNode,string themePath);
};

//Class containing the things needed for a themed character.
class ThemeCharacter{
public:
	//Map containing ThemeCharacterStates for the different states of a character.
	map<string,ThemeCharacterState*> characterStates;
public:
	//Constructor.
	ThemeCharacter(){}
	//Destructor.
	~ThemeCharacter(){
		//Loop through the states and delete them.
		for(map<string,ThemeCharacterState*>::iterator i=characterStates.begin();i!=characterStates.end();i++){
			delete i->second;
		}
	}
	
	//Method that will destroy the ThemeCharacter.
	void destroy(){
		//Loop through the states and delete them.
		for(map<string,ThemeCharacterState*>::iterator i=characterStates.begin();i!=characterStates.end();i++){
			delete i->second;
		}
		//Clear the characterStates map.
		characterStates.clear();
	}
	
	//Method that will load a ThemeCharacter from a node.
	//objNode: The TreeStorageNode to load the ThemeCharacter from.
	//themePath: The path to the theme.
	//Returns: True if it succeeds.
	bool loadFromNode(TreeStorageNode* objNode,string themePath);
	
	//Method that will create a ThemeCharacterInstance.
	//obj: Pointer that will be filled with the instance.
	void createInstance(ThemeCharacterInstance* obj);
};

//ThemeBackgroundPicture is a class containing the picture for the background.
class ThemeBackgroundPicture{
private:
	//SDL_Surface containing the picture.
	SDL_Surface* picture;
	//Rectangle that should be taken from the picture.
	SDL_Rect srcSize;
	//Rectangle with the size it will have on the destination (screen).
	SDL_Rect destSize;
	
	//Boolean if the image should be repeated over the x-axis.
	bool repeatX;
	//Boolean if the image should be repeated over the y-axis.
	bool repeatY;
	
	//Float containing the speed the background picture moves along the x-axis.
	float speedX;
	//Float containing the speed the background picture moves along the y-axis.
	float speedY;
	
	//Float containing the horizontal speed the picture will have when moving the camera (horizontally).
	float cameraX;
	//Float containing the vertical speed the picture will have when moving the camera (vertically).
	float cameraY;
private:
	//Float with the current x position.
	float currentX;
	//Float with the current y position.
	float currentY;
	
	//Stored x location for when loading a state.
	float savedX;
	//Stored y location for when loading a state.
	float savedY;
public:
	//Constructor.
	ThemeBackgroundPicture(){
		//Set some default values.
		picture=NULL;
		memset(&srcSize,0,sizeof(srcSize));
		memset(&destSize,0,sizeof(destSize));
		repeatX=true;
		repeatY=true;
		speedX=0.0f;
		speedY=0.0f;
		cameraX=0.0f;
		cameraY=0.0f;
		currentX=0.0f;
		currentY=0.0f;
		savedX=0.0f;
		savedY=0.0f;
	}
	
	//Method that will update the animation.
	void updateAnimation(){
		//Move the picture along the x-axis.
		currentX+=speedX;
		if(repeatX && destSize.w>0){
			float f=(float)destSize.w;
			if(currentX>f || currentX<-f) currentX-=f*floor(currentX/f);
		}
		
		//Move the picture along the y-axis.
		currentY+=speedY;
		if(repeatY && destSize.h>0){
			float f=(float)destSize.h;
			if(currentY>f || currentY<-f) currentY-=f*floor(currentY/f);
		}
	}
	
	//Method that will reset the animation.
	//save: Boolean if the saved state should be deleted.
	void resetAnimation(bool save){
		currentX=0.0f;
		currentY=0.0f;
		if(save){
			savedX=0.0f;
			savedY=0.0f;
		}
	}
	//Method that will save the animation.
	void saveAnimation(){
		savedX=currentX;
		savedY=currentY;
	}
	//Method that will load the animation.
	void loadAnimation(){
		currentX=savedX;
		currentY=savedY;
	}
	
	//Method used to draw the ThemeBackgroundPicture.
	//dest: Pointer to the SDL_Surface the picture should be drawn.
	void draw(SDL_Surface *dest);
	
	//Method used to load the ThemeBackgroundPicture from a node.
	//objNode: The TreeStorageNode to load the picture from.
	//themePath: The path to the theme.
	bool loadFromNode(TreeStorageNode* objNode,string themePath);
};

//Class that forms the complete background of a theme.
//It is in fact nothing more than a vector containing multiple ThemeBackgroundPictures.
class ThemeBackground{
private:
	//Vector containing the ThemeBackgroundPictures.
	vector<ThemeBackgroundPicture> picture;
public:
	//Method that will update the animation of all the background pictures.
	void updateAnimation(){
		for(unsigned int i=0;i<picture.size();i++){
			picture[i].updateAnimation();
		}
	}
	
	//Method that will reset the animation of all the background pictures.
	//save: Boolean if the saved state should be deleted.
	void resetAnimation(bool save){
		for(unsigned int i=0;i<picture.size();i++){
			picture[i].resetAnimation(save);
		}
	}
	
	//Method that will save the animation of all the background pictures.
	void saveAnimation(){
		for(unsigned int i=0;i<picture.size();i++){
			picture[i].saveAnimation();
		}
	}
	//Method that will load the animation of all the background pictures.
	void loadAnimation(){
		for(unsigned int i=0;i<picture.size();i++){
			picture[i].loadAnimation();
		}
	}
	
	//This method will draw all the background pictures.
	//dest: Pointer to the SDL_Surface to draw them on.
	void draw(SDL_Surface* dest){
		for(unsigned int i=0;i<picture.size();i++){
			picture[i].draw(dest);
		}
	}
	
	//Method that will add a ThemeBackgroundPicture to the ThemeBackground.
	//objNode: The treeStorageNode to read from.
	//themePath: The path to the theme.
	//Returns: True if it succeeds.
	bool addPictureFromNode(TreeStorageNode* objNode,string themePath){
		picture.push_back(ThemeBackgroundPicture());
		return picture.back().loadFromNode(objNode,themePath);
	}
};

//The ThemeManager is actaully a whole theme, filled with ThemeBlocks, ThemeCharacter and ThemeBackground.
class ThemeManager{
private:
	//The ThemeCharacter of the shadow.
	ThemeCharacter* shadow;
	//The ThemeCharacter of the player.
	ThemeCharacter* player;
	
	//Array containing a ThemeBlock for every block type.
	ThemeBlock* objBlocks[TYPE_MAX];
	
	//The ThemeBackground.
	ThemeBackground* objBackground;
public:
	//String containing the path to the string.
	string themePath;
	//String containing the theme name.
	string themeName;
public:
	//Constructor.
	ThemeManager(){
		//Make sure the pointers are set to NULL.
		objBackground=NULL;
		//Reserve enough memory for the ThemeBlocks.
		memset(objBlocks,0,sizeof(objBlocks));
		shadow=NULL;
		player=NULL;
	}
	//Destructor.
	~ThemeManager(){
		//Delete the ThemeCharacter of the shadow.
		if(shadow)
			delete shadow;
		//Delete the ThemeCharacter of the player.
		if(player)
			delete player;
		//Loop through the ThemeBlocks and delete them.
		for(int i=0;i<TYPE_MAX;i++){
			if(objBlocks[i])
				delete objBlocks[i];
		}
		//Delete the ThemeBackground.
		if(objBackground)
			delete objBackground;
	}

	//Method used to destroy the ThemeManager.
	void destroy(){
		//Delete the ThemeCharacter of the shadow.
		if(shadow)
			delete shadow;
		//Delete the ThemeCharacter of the player.
		if(player)
			delete player;
		//Loop through the ThemeBlocks and delete them.
		for(int i=0;i<TYPE_MAX;i++){
			if(objBlocks[i])
				delete objBlocks[i];
		}
		//Delete the ThemeBackground.
		if(objBackground)
			delete objBackground;
		
		//And clear the themeName.
		themeName.clear();
	}
	
	//Method that will load the theme from a file.
	//fileName: The file to load the theme from.
	//Returns: True if it succeeds.
	bool loadFile(const string& fileName);
	
	//Get a pointer to the ThemeBlock of a given block type.
	//index: The type of block.
	//Returns: Pointer to the ThemeBlock.
	ThemeBlock* getBlock(int index){
		return objBlocks[index];
	}
	//Get a pointer to the ThemeCharacter of the shadow or the player.
	//isShadow: Boolean if it's the shadow
	//Returns: Pointer to the ThemeCharacter.
	ThemeCharacter* getCharacter(bool isShadow){
		if(isShadow)
			return shadow;
		return player;
	}
	//Get a pointer to the ThemeBackground of the theme.
	//Returns: Pointer to the ThemeBackground.
	ThemeBackground* getBackground(){
		return objBackground;
	}
};

//Class that combines multiple ThemeManager into one stack.
//If a file is not in a certain theme it will use one of a lower theme.
class ThemeStack{
private:
	//Vector containing the themes in the stack.
	vector<ThemeManager*> objThemes;
public:
	//Constructor.
	ThemeStack(){}
	//Destructor.
	~ThemeStack(){
		//Loop through the themes and delete them.
		for(unsigned int i=0;i<objThemes.size();i++)
			delete objThemes[i];
	}
	
	//Method that will destroy the ThemeStack.
	void destroy(){
		//Loop through the themes and delete them.
		for(unsigned int i=0;i<objThemes.size();i++)
			delete objThemes[i];
		//Clear the vector to prevent dangling pointers.
		objThemes.clear();
	}
	
	//Method that will append a theme to the stack.
	//obj: The ThemeManager to add.
	void appendTheme(ThemeManager* obj){
		objThemes.push_back(obj);
	}
	//Method that will remove the last theme added to the stack.
	void removeTheme(){
		//Make sure that the stack isn't empty.
		if(!objThemes.empty()){
			delete objThemes.back();
			objThemes.pop_back();
		}
	}
	
	//Method that will append a theme that will be loaded from file.
	//fileName: The file to load the theme from.
	//Returns: Pointer to the newly added theme, NULL if failed.
	ThemeManager* appendThemeFromFile(const string& fileName){
		//Create a new themeManager.
		ThemeManager* obj=new ThemeManager();
		
		//Let it load from the given file.
		if(!obj->loadFile(fileName)){
			//Failed thus delete the theme and return null.
			cerr<<"ERROR: Failed loading theme "<<fileName<<endl;
			delete obj;
			return NULL;
		}else{
			//Succeeded, add it to the stack and return it.
			objThemes.push_back(obj);
			return obj;
		}
	}
	
	//Get the number of themes in the stack.
	//Returns: The theme count.
	int themeCount(){
		return (int)objThemes.size();
	}
	
	//Operator overloading so that the themes can be accesed using the [] operator.
	//i: The index.
	ThemeManager* operator[](int i){
		return objThemes[i];
	}
	//Get a pointer to the ThemeBlock of a given block type.
	//index: The type of block.
	//Returns: Pointer to the ThemeBlock.
	ThemeBlock* getBlock(int index){
		//Loop through the themes from top to bottom.
		for(int i=objThemes.size()-1;i>=0;i--){
			//Get the block from the theme.
			ThemeBlock* obj=objThemes[i]->getBlock(index);
			//Check if it isn't null.
			if(obj)
				return obj;
		}
		
		//Nothing found.
		return NULL;
	}
	//Get a pointer to the ThemeCharacter of the shadow or the player.
	//isShadow: Boolean if it's the shadow
	//Returns: Pointer to the ThemeCharacter.
	ThemeCharacter* getCharacter(bool isShadow){
		//Loop through the themes from top to bottom.
		for(int i=objThemes.size()-1;i>=0;i--){
			//Get the ThemeCharacter from the theme.
			ThemeCharacter* obj=objThemes[i]->getCharacter(isShadow);
			//Check if it isn't null.
			if(obj)
				return obj;
		}
		
		//Nothing found.
		return NULL;
	}
	//Get a pointer to the ThemeBackground of the theme.
	//Returns: Pointer to the ThemeBackground.
	ThemeBackground* getBackground(){
		//Loop through the themes from top to bottom.
		for(int i=objThemes.size()-1;i>=0;i--){
			//Get the ThemeBackground from the theme.
			ThemeBackground* obj=objThemes[i]->getBackground();
			//Check if it isn't null.
			if(obj)
				return obj;
		}
		
		//Nothing found.
		return NULL;
	}
};

//The ThemeStack that is be used by the GameState.
extern ThemeStack objThemes;

#endif
