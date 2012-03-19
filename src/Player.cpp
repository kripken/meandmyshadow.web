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
#include "Player.h"
#include "Game.h"
#include "Functions.h"
#include "FileManager.h"
#include "Globals.h"
#include "Objects.h"
#include "InputManager.h"
#include "MD5.h"
#include <iostream>
#include <fstream>
#include <SDL/SDL_mixer.h>
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
using namespace std;

#ifdef RECORD_FILE_DEBUG
string recordKeyPressLog,recordKeyPressLog_saved;
vector<SDL_Rect> recordPlayerPosition,recordPlayerPosition_saved;
#endif

Player::Player(Game* objParent):xVelBase(0),yVelBase(0),objParent(objParent){
	//Set the dimensions of the player.
	//The size of the player is 21x40.
	box.x=0;
	box.y=0;
	box.w=21;
	box.h=40;

	//Set his velocity to zero.
	xVel=0;
	yVel=0;

	//Set the start position.
	fx=0;
	fy=0;

	//Check if sound is enabled.
	if(getSettings()->getBoolValue("sound")==true){
		//It is so load the sounds.
		jumpSound=Mix_LoadWAV((getDataPath()+"sfx/jump.wav").c_str());
		hitSound=Mix_LoadWAV((getDataPath()+"sfx/hit.wav").c_str());
		saveSound=Mix_LoadWAV((getDataPath()+"sfx/checkpoint.wav").c_str());
		swapSound=Mix_LoadWAV((getDataPath()+"sfx/swap.wav").c_str());
		toggleSound=Mix_LoadWAV((getDataPath()+"sfx/toggle.wav").c_str());
		errorSound=Mix_LoadWAV((getDataPath()+"sfx/error.wav").c_str());
	}
	
	//Set some default values.
	inAir=true;
	isJump=false;
	onGround=true;
	shadowCall=false;
	shadow=false;
	canMove=true;
	holdingOther=false;
	dead=false;
	record=false;
	downKeyPressed=false;
	spaceKeyPressed=false;
	recordIndex=-1;
#ifdef RECORD_FILE_DEBUG
	recordKeyPressLog.clear();
	recordKeyPressLog_saved.clear();
	recordPlayerPosition.clear();
	recordPlayerPosition_saved.clear();
#endif
	objNotificationBlock=NULL;
	
	//Some default values for animation variables.
	direction=0;
	jumpTime=0;

	state=0;

	//xVelSaved is used to store if there's a state saved or not.
	xVelSaved=0x80000000;
}

Player::~Player(){
	//We only need to clean up the sounds if they were loaded.
	if(getSettings()->getBoolValue("sound")==true){
		Mix_FreeChunk(jumpSound);
		Mix_FreeChunk(hitSound);
		Mix_FreeChunk(saveSound);
		Mix_FreeChunk(swapSound);
		Mix_FreeChunk(toggleSound);
	}
}

bool Player::isPlayFromRecord(){
	return recordIndex>=0; // && recordIndex<(int)recordButton.size();
}

//get the game record object.
std::vector<int>* Player::getRecord(){
	return &recordButton;
}

#ifdef RECORD_FILE_DEBUG
string& Player::keyPressLog(){
	return recordKeyPressLog;
}
vector<SDL_Rect>& Player::playerPosition(){
	return recordPlayerPosition;
}
#endif

//play the record.
void Player::playRecord(){
	//TODO:
	recordIndex=0;
}

void Player::spaceKeyDown(class Shadow* shadow){
	//Start recording or stop, depending on the recording state.
	if(record==false){
		//We start recording.
		if(shadow->called==true){
			//The shadow is still busy so first stop him before we can start recording.
			shadowCall=false;
			shadow->called=false;
			shadow->playerButton.clear();
		}else if(!dead){
			//The shadow isn't moving and we aren't dead so start recording.
			record=true;

			//We start a recording meaning we need to increase recordings by one.
			objParent->recordings++;
		}
	}else{
		//The player is recording so stop recording and call the shadow.
		record=false;
		shadowCall=true;
	}
}

void Player::handleInput(class Shadow* shadow){
	//Check if we should read the input from record file.
	//Actually, we read input from record file in
	//another function shadowSetState.
	bool readFromRecord=false;
	if(recordIndex>=0 && recordIndex<(int)recordButton.size()) readFromRecord=true;

	if(!readFromRecord){
		//Reset horizontal velocity.
		xVel=0;
		if(inputMgr.isKeyDown(INPUTMGR_RIGHT)){
			//Walking to the right.
			xVel+=7;
		}
		if(inputMgr.isKeyDown(INPUTMGR_LEFT)){
			//Walking to the left.
			xVel-=7;
		}
		
		//Check if a key has been released.
		if(/*event.type==SDL_KEYUP || */!inputMgr.isKeyDown(INPUTMGR_ACTION)){
			//It has so downKeyPressed can't be true.
			downKeyPressed=false;
		}
		/*
		//Don't reset spaceKeyPressed or when you press the space key
		//and release another key then the bug occurs. (ticket #44)
		if(event.type==SDL_KEYUP || !inputMgr.isKeyDown(INPUTMGR_SPACE)){
			spaceKeyPressed=false;
		}*/
	}

	//Check if a key is pressed (down).
	if(inputMgr.isKeyDownEvent(INPUTMGR_JUMP) && !readFromRecord){
		//The up key, if we aren't in the air we start jumping.
		//Fixed a potential bug
		if(!inAir && !isJump){
#ifdef RECORD_FILE_DEBUG
			char c[64];
			sprintf(c,"[%05d] Jump key pressed\n",objParent->time);
			cout<<c;
			recordKeyPressLog+=c;
#endif
			isJump=true;
		}
	}else if(inputMgr.isKeyDownEvent(INPUTMGR_SPACE) && !readFromRecord){
		//Fixed a potential bug
		if(!spaceKeyPressed){
#ifdef RECORD_FILE_DEBUG
			char c[64];
			sprintf(c,"[%05d] Space key pressed\n",objParent->time);
			cout<<c;
			recordKeyPressLog+=c;
#endif
			spaceKeyDown(shadow);
			spaceKeyPressed=true;
		}
	}else if(inputMgr.isKeyDownEvent(INPUTMGR_ACTION)){
		//Downkey is pressed.
		//Fixed a potential bug
		if(!downKeyPressed){
#ifdef RECORD_FILE_DEBUG
			char c[64];
			sprintf(c,"[%05d] Action key pressed\n",objParent->time);
			cout<<c;
			recordKeyPressLog+=c;
#endif
			downKeyPressed=true;
		}
	}else if(inputMgr.isKeyDownEvent(INPUTMGR_SAVE)){
		//F2 only works in the level editor.
		if(!(dead || shadow->dead) && stateID==STATE_LEVEL_EDITOR){
			//Save the state.
			if(objParent)
				objParent->saveState();
		}
	}else if(inputMgr.isKeyDownEvent(INPUTMGR_LOAD) && !readFromRecord){
		//F3 is used to load the last state.
		if(objParent)
			objParent->loadState();
	}else if(inputMgr.isKeyDownEvent(INPUTMGR_SWAP)){
		//F4 will swap the player and the shadow, but only in the level editor.
		if(!(dead || shadow->dead) && stateID==STATE_LEVEL_EDITOR){
			swapState(shadow);
		}
	}else if(inputMgr.isKeyDownEvent(INPUTMGR_TELEPORT)){
		//F5 will revive and teleoprt the player to the cursor. Only works in the level editor.
		//Shift+F5 teleports the shadow.
		if(stateID==STATE_LEVEL_EDITOR){
			//get the position of the cursor.
			int x,y;
			SDL_GetMouseState(&x,&y);
			x+=camera.x;
			y+=camera.y;

			if(inputMgr.isKeyDown(INPUTMGR_SHIFT)){
				//teleports the shadow.
				shadow->dead=false;
				shadow->box.x=x;
				shadow->box.y=y;
			}else{
				//teleports the player.
				dead=false;
				box.x=x;
				box.y=y;
			}

			//play sound?
			if(getSettings()->getBoolValue("sound")){
				Mix_PlayChannel(-1,swapSound,0);
			}
		}
	}else if(inputMgr.isKeyDownEvent(INPUTMGR_SUICIDE)){
		//F12 is suicide and only works in the leveleditor.
		if(stateID==STATE_LEVEL_EDITOR){
			die();
			shadow->die();
		}
	}
}

void Player::setPosition(int x,int y){
	box.x=x;
	box.y=y;
}

void Player::move(vector<GameObject*> &levelObjects){
	//Pointer to a checkpoint.
	GameObject* objCheckPoint=NULL;
	//Pointer to a swap.
	GameObject* objSwap=NULL;
	
	//Set the objShadowBlock to NULL.
	//Only for swapping to prevent the shadow from swapping in a shadow block.
	objShadowBlock=NULL;
	
	//Set the objNotificationBlock to NULL.
	objNotificationBlock=NULL;
	
	//Boolean if the player can teleport.
	bool canTeleport=true;
	
	//Set the object the player is currently standing to NULL.
	objCurrentStand=NULL;

	//Check if the player is still alive.
	if(dead==false){
		//Add gravity
		if(inAir==true){
			yVel+=1;
			
			//Cap fall speed to 13.
			if(yVel>13){
				yVel=13;
			}
		}
		
		//Boolean if the player is moved, used for squash detection.
		bool playerMoved=false;

		//Check if the player can move.
		if(canMove==true){
			//Check if the player is moving or not.
			if(xVel>0){ 
				direction=0;
				onGround=false;
				if(appearance.currentStateName!="walkright"){
					appearance.changeState("walkright");
				}
			}else if(xVel<0){ 
				direction=1;
				onGround=false;
				if(appearance.currentStateName!="walkleft"){
					appearance.changeState("walkleft");
				}
			}else if(xVel==0){
				onGround=true;
				if(direction==1){
					appearance.changeState("standleft");
				}else{
					appearance.changeState("standright");
				}
			}
			//Move the player.
			box.x+=xVel;
			
			//Loop through the levelobjects.
			for(unsigned int o=0; o<levelObjects.size(); o++){
				//Check if the player can walk on the object.
				if(levelObjects[o]->queryProperties(GameObjectProperty_PlayerCanWalkOn,this)){
					//Get the collision box of the levelobject.
					SDL_Rect r=levelObjects[o]->getBox();
					
					//Check collision with the player.
					if(checkCollision(box,r)){
						//We have collision, get the velocity of the box.
						SDL_Rect v=levelObjects[o]->getBox(BoxType_Delta);
						
						//Check on which side of the box the player is.
						if(box.x + box.w/2 <= r.x + r.w/2){
							//The left side of the block.
							if(xVel+xVelBase>v.x){
								if(box.x>r.x-box.w){
									box.x=r.x-box.w;
									
									playerMoved=true;
								}
							}
						}else{
							//The right side of the block.
							if(xVel+xVelBase<v.x){
								if(box.x<r.x+r.w){
									box.x=r.x+r.w;
									
									playerMoved=true;
								}
							}
						}
					}
				}
			}
		}
		
		//Now apply the yVel. (gravity, jumping, etc..)
		box.y+=yVel;
		
		//Pointer to the object the player standed on.
		GameObject* lastStand=NULL;
		
		//???
		inAir=true;
		canMove=true;
		
		//Loop through all the levelObjects.
		for(unsigned int o=0; o<levelObjects.size(); o++){
			//Check if the object is solid.
			if(levelObjects[o]->queryProperties(GameObjectProperty_PlayerCanWalkOn,this)){
				SDL_Rect r=levelObjects[o]->getBox();
				if(checkCollision(r,box)==true){ //TODO:fix some bug
					SDL_Rect v=levelObjects[o]->getBox(BoxType_Delta);
					
					if(box.y+box.h/2<=r.y+r.h/2){
						if(yVel>=v.y || yVel>=0){
							inAir=false;
							box.y=r.y-box.h;
							yVel=1; //???
							lastStand=levelObjects[o];
							lastStand->onEvent(GameObjectEvent_PlayerIsOn);
							
							//The player is moved, if it's a moving block check for squating.
							if(v.y!=0){
								playerMoved=true;
							}
						}
					}else{
						if(yVel<=v.y+1){ 
							yVel=v.y>0?v.y:0; 
							if(box.y<r.y+r.h){
								box.y=r.y+r.h;
								
								//The player is moved, if it's a moving block check for squating.
								if(v.y!=0){
									playerMoved=true;
								}
							}
						}
					}
				}
			}
			
			//Check if the object is a checkpoint.
			if(levelObjects[o]->type==TYPE_CHECKPOINT && checkCollision(box,levelObjects[o]->getBox())){
				//If we're not the shadow set the gameTip to Checkpoint.
				if(!shadow && objParent!=NULL)
					objParent->gameTipIndex=TYPE_CHECKPOINT;
				
				//And let objCheckPoint point to this object.
				objCheckPoint=levelObjects[o];
			}

			//Check if the object is a swap.
			if(levelObjects[o]->type==TYPE_SWAP && checkCollision(box,levelObjects[o]->getBox())){
				//If we're not the shadow set the gameTip to swap.
				if(!shadow && objParent!=NULL)
					objParent->gameTipIndex=TYPE_SWAP;
				
				//And let objSwap point to this object.
				objSwap=levelObjects[o];
			}
			
			//Check if the object is an exit.
			//This doesn't work if the state is Level editor.
			if(levelObjects[o]->type==TYPE_EXIT && stateID!=STATE_LEVEL_EDITOR && checkCollision(box,levelObjects[o]->getBox())){
				//Check if it's playing game record (?)
				if(recordIndex>=0){
					objParent->recordingEnded();
					return;
				}
				
				//We can't just handle the winning here (in the middle of the update cycle)/
				//So set won in Game true.
				objParent->won=true;
			}

			//Check if the object is a portal.
			if(levelObjects[o]->type==TYPE_PORTAL && checkCollision(box,levelObjects[o]->getBox())){
				//Check if the teleport id isn't empty.
				if((dynamic_cast<Block*>(levelObjects[o]))->id.empty()){
					cerr<<"Warning: Invalid teleport id!"<<endl;
					canTeleport=false;
				}
				
				//If we're not the shadow set the gameTip to portal.
				if(!shadow && objParent!=NULL)
					objParent->gameTipIndex=TYPE_PORTAL;
				
				//Check if we can teleport and should (downkey -or- auto).
				if(canTeleport && (downKeyPressed || (levelObjects[o]->queryProperties(GameObjectProperty_Flags,this)&1))){
					canTeleport=false;
					if(downKeyPressed || levelObjects[o]!=objLastTeleport){
						downKeyPressed=false;
						
						//Loop the levelobjects again to find the destination.
						for(unsigned int oo=o+1;;){
							//We started at our index+1.
							//Meaning that if we reach the end of the vector then we need to start at the beginning.
							if(oo>=levelObjects.size())
								oo-=(int)levelObjects.size();
							//It also means that if we reach the same index we need to stop.
							//If the for loop breaks this way then we have no succes.
							if(oo==o){
								//Couldn't teleport so play the error sound.
								if(getSettings()->getBoolValue("sound")){
									Mix_PlayChannel(-1,errorSound,0);
								}
								break;
							}
							
							//Check if the second (oo) object is a portal.
							if(levelObjects[oo]->type==TYPE_PORTAL){
								//Check the id against the destination of the first portal.
								if((dynamic_cast<Block*>(levelObjects[o]))->destination==(dynamic_cast<Block*>(levelObjects[oo]))->id){
									//Call the event.
									levelObjects[o]->onEvent(GameObjectEvent_OnToggle);
									objLastTeleport=levelObjects[oo];
									
									//Get the destination location and teleport the player.
									SDL_Rect r=levelObjects[oo]->getBox();
									box.x=r.x+5;
									box.y=r.y+2;
									
									//Check if music/sound is enabled.
									if(getSettings()->getBoolValue("sound")){
										Mix_PlayChannel(-1,swapSound,0);
									}
									break;
								}
							}
							
							//Increase oo.
							oo++;
						}
					}
				}
			}
			
			//Check if the object is a switch.
			if(levelObjects[o]->type==TYPE_SWITCH && checkCollision(box,levelObjects[o]->getBox())){
				//If we're not the shadow set the gameTip to switch.
				if(!shadow && objParent!=NULL)
					objParent->gameTipIndex=TYPE_SWITCH;
				
				//If the down key is pressed then invoke an event.
				if(downKeyPressed){
					//Play the animation.
					levelObjects[o]->playAnimation(1);
					
					//Check if sound is enabled, if so play the toggle sound.
					if(getSettings()->getBoolValue("sound")==true){
						Mix_PlayChannel(-1,toggleSound,0);
					}
					
					if(objParent!=NULL){
						//Make sure that the id isn't emtpy.
						if(!(dynamic_cast<Block*>(levelObjects[o]))->id.empty()){
							objParent->broadcastObjectEvent(0x10000 | (levelObjects[o]->queryProperties(GameObjectProperty_Flags,this)&3),
								-1,(dynamic_cast<Block*>(levelObjects[o]))->id.c_str());
						}else{
							cerr<<"Warning: invalid switch id!"<<endl;
						}
					}
				}
			}
			
			//Check if the object is a shadow block, only if we are the player.
			if((levelObjects[o]->type==TYPE_SHADOW_BLOCK || levelObjects[o]->type==TYPE_MOVING_SHADOW_BLOCK) && checkCollision(box,levelObjects[o]->getBox()) && !shadow){
				objShadowBlock=levelObjects[o];
			}
			
			//Check if the object is a notification block, only if we are the player.
			if(levelObjects[o]->type==TYPE_NOTIFICATION_BLOCK && checkCollision(box,levelObjects[o]->getBox()) && !shadow){
				objNotificationBlock=levelObjects[o];
			}
			
			//Check if the object is deadly.
			if(levelObjects[o]->queryProperties(GameObjectProperty_IsSpikes,this)){
				//It is so get the collision box.
				SDL_Rect r=levelObjects[o]->getBox();
				
				//TODO: pixel-accuracy hit test.
				//For now we shrink the box.
				r.x+=2;
				r.y+=2;
				r.w-=4;
				r.h-=4;
				
				//Check collision, if the player collides then let him die.
				if(checkCollision(box,r))
					die();
			}
		}
		
		//Check if the player was moved, if so check if the player is squashed.
		if(playerMoved){
			for(unsigned int o=0;o<levelObjects.size();o++){
				SDL_Rect r2=levelObjects[o]->getBox();
				if(levelObjects[o]->queryProperties(GameObjectProperty_PlayerCanWalkOn,this) && checkCollision(box,r2))
					die();
			}
		}
		
		//Check if the player fell of the level.
		if(box.y>LEVEL_HEIGHT)
			die();

		//Check if the player changed blocks, meaning stepped onto a block.
		objCurrentStand=lastStand;
		if(lastStand!=objLastStand){
			objLastStand=lastStand;
			if(lastStand){
				//Call the walk on event of the laststand.
				lastStand->onEvent(GameObjectEvent_PlayerWalkOn);
				
				//Bugfix for Fragile blocks.
				if(lastStand->type==TYPE_FRAGILE && !lastStand->queryProperties(GameObjectProperty_PlayerCanWalkOn,this)){
					inAir=true;
					onGround=false;
					isJump=false;
				}
			}
		}
		
		//Check if the player can teleport.
		if(canTeleport)
			objLastTeleport=NULL;

		//Check the checkpoint pointer only if the downkey is pressed.
		//new: don't save the game if playing game record
		if(objParent!=NULL && downKeyPressed && objCheckPoint!=NULL && !isPlayFromRecord()){
			//Checkpoint thus save the state.
			if(objParent->saveState())
				objParent->objLastCheckPoint=objCheckPoint;
		}
		//Check the swap pointer only if the down key is pressed.
		if(objSwap!=NULL && downKeyPressed && objParent!=NULL){
			//Now check if the shadow we're the shadow or not.
			if(shadow){
				if(!(dead || objParent->player.dead)){
					//Check if the player isn't in front of a shadow block.
					if(!objParent->player.objShadowBlock){
						objParent->player.swapState(this);
						objSwap->playAnimation(1);
					}else{
						//We can't swap so play the error sound.
						if(getSettings()->getBoolValue("sound")==true){
							Mix_PlayChannel(-1,errorSound,0);
						}
					}
				}
			}else{
				if(!(dead || objParent->shadow.dead)){
					//Check if the player isn't in front of a shadow block.
					if(!objShadowBlock){
						swapState(&objParent->shadow);
						objSwap->playAnimation(1);
					}else{
						//We can't swap so play the error sound.
						if(getSettings()->getBoolValue("sound")==true){
							Mix_PlayChannel(-1,errorSound,0);
						}
					}
				}
			}
		}
		
		//Check for jump appearance (inAir).
		if(inAir && !dead){
			if(direction==1){
				if(yVel>0){
					if(appearance.currentStateName!="fallleft")
						appearance.changeState("fallleft");
				}else{
					if(appearance.currentStateName!="jumpleft")
						appearance.changeState("jumpleft");
				}
			}else{
				if(yVel>0){
					if(appearance.currentStateName!="fallright")
						appearance.changeState("fallright");
				}else{
					if(appearance.currentStateName!="jumpright")
						appearance.changeState("jumpright");
				}
			}
		}
	}
	
	//Finally we reset some stuff.
	downKeyPressed=false;
	xVelBase=0;
	yVelBase=0;
}


void Player::jump(){
	//Check if the player is dead or not.
	if(dead==true){
		//The player can't jump if he's dead.
		isJump=false;
	}
	
	//Check if the player can jump.
	if(isJump==true && inAir==false){
		//Set the jump velocity.
		yVel=-13;
		inAir=true;
		isJump=false;
		jumpTime++;
		
		//Check if sound is enabled, if so play the jump sound.
		if(getSettings()->getBoolValue("sound")==true){
			Mix_PlayChannel(-1,jumpSound,0);
		}
	}
}

void Player::show(){
	//Check if we should render the recorded line.
	//Only do this when we're recording and we're not the shadow.
	if(shadow==false && record==true){
		//FIXME: Adding an entry not in update but in render?
		line.push_back(SDL_Rect());
		line[line.size()-1].x=box.x+11;
		line[line.size()-1].y=box.y+20;
		
		//Loop through the line dots and draw them.
		for(int l=0; l<(signed)line.size(); l++){
			appearance.drawState("line",screen,line[l].x-camera.x,line[l].y-camera.y,NULL);
		}
	}
	
	//NOTE: We do logic here, because it's only needed by the appearance.
	appearance.updateAnimation();
	appearance.draw(screen, box.x-camera.x, box.y-camera.y, NULL);
}

void Player::shadowSetState(){
	int currentKey=0;

	//Check if we should read the input from record file.
	if(recordIndex>=0){ // && recordIndex<(int)recordButton.size()){
		//read the input from record file
		if(recordIndex<(int)recordButton.size()){
			currentKey=recordButton[recordIndex];
			recordIndex++;
		}

		//Reset horizontal velocity.
		xVel=0;
		if(currentKey&PlayerButtonRight){
			//Walking to the right.
			xVel+=7;
		}
		if(currentKey&PlayerButtonLeft){
			//Walking to the left.
			xVel-=7;
		}

		if(currentKey&PlayerButtonJump){
			//The up key, if we aren't in the air we start jumping.
			if(inAir==false){
				isJump=true;
			}else{
				//Shouldn't go here
				cout<<"Replay BUG"<<endl;
			}
		}

		//check the down key
		downKeyPressed=(currentKey&PlayerButtonDown)!=0;

		//check the space key
		if(currentKey&PlayerButtonSpace){
			spaceKeyDown(&objParent->shadow);
		}
	}else{
		//read the input from keyboard.
		recordIndex=-1;

		//Check for xvelocity.
		if(xVel>0)
			currentKey|=PlayerButtonRight;
		if(xVel<0)
			currentKey|=PlayerButtonLeft;

		//Check for jumping.
		if(isJump){
#ifdef RECORD_FILE_DEBUG
			char c[64];
			sprintf(c,"[%05d] Jump key recorded\n",objParent->time-1);
			cout<<c;
			recordKeyPressLog+=c;
#endif
			currentKey|=PlayerButtonJump;
		}

		//Check if the downbutton is pressed.
		if(downKeyPressed){
#ifdef RECORD_FILE_DEBUG
			char c[64];
			sprintf(c,"[%05d] Action key recorded\n",objParent->time-1);
			cout<<c;
			recordKeyPressLog+=c;
#endif
			currentKey|=PlayerButtonDown;
		}

		if(spaceKeyPressed){
#ifdef RECORD_FILE_DEBUG
			char c[64];
			sprintf(c,"[%05d] Space key recorded\n",objParent->time-1);
			cout<<c;
			recordKeyPressLog+=c;
#endif
			currentKey|=PlayerButtonSpace;
		}

		//Record it.
		recordButton.push_back(currentKey);
	}

#ifdef RECORD_FILE_DEBUG
	if(recordIndex>=0){
		if(recordIndex>0 && recordIndex<=int(recordPlayerPosition.size())/2){
			SDL_Rect &r1=recordPlayerPosition[recordIndex*2-2];
			SDL_Rect &r2=recordPlayerPosition[recordIndex*2-1];
			if(r1.x!=box.x || r1.y!=box.y || r2.x!=objParent->shadow.box.x || r2.y!=objParent->shadow.box.y){
				char c[192];
				sprintf(c,"Replay ERROR [%05d] %d %d %d %d Expected: %d %d %d %d\n",
					objParent->time-1,box.x,box.y,objParent->shadow.box.x,objParent->shadow.box.y,r1.x,r1.y,r2.x,r2.y);
				cout<<c;
			}
		}
	}else{
		recordPlayerPosition.push_back(box);
		recordPlayerPosition.push_back(objParent->shadow.box);
	}
#endif

	//reset spaceKeyPressed.
	spaceKeyPressed=false;

	//Only add an entry if the player is recording.
	if(record){
		//Add the action.
		if(!dead){
			playerButton.push_back(currentKey);
			
			//Change the state.
			state++;
		}
	}
}

void Player::shadowGiveState(Shadow* shadow){
	//Check if the player calls the shadow.
	if(shadowCall==true){
		//Clear any recording still with the shadow.
		shadow->playerButton.clear();

		//Loop the recorded moves and add them to the one of the shadow.
		for(unsigned int s=0;s<playerButton.size();s++){
			shadow->playerButton.push_back(playerButton[s]);
		}

		//Reset the state of both the player and the shadow.
		stateReset();
		shadow->stateReset();

		//Clear the recording at the player's side.
		playerButton.clear();
		line.clear();

		//Set shadowCall false
		shadowCall=false;
		//And let the shadow know that the player called him.
		shadow->meCall();
	}
}

void Player::stateReset(){
	//Reset the state by setting it to 0.
	state=0;
}

void Player::otherCheck(class Player* other){
	//First make sure the player isn't dead.
	//And check for velocity of the block the player is standing on.
	if(!dead){
		if(objCurrentStand!=NULL){
			//Now get the velocity of the object the player is standing on.
			SDL_Rect v=objCurrentStand->getBox(BoxType_Velocity);
			
			//Set the base velocity to the velocity of the object.
			xVelBase=v.x;
			yVelBase=v.y;
			
			//Already move the player box.
			box.x+=v.x;
			box.y+=v.y;
		}
	}
	//Now do the same for the shadow.
	if(!other->dead){
		if(other->objCurrentStand!=NULL){
			//Now get the velocity of the object the shadow is standing on.
			SDL_Rect v=other->objCurrentStand->getBox(BoxType_Velocity);
			
			//Set the base velocity to the velocity of the object.
			other->xVelBase=v.x;
			other->yVelBase=v.y;
			
			//Already move the shadow box.
			other->box.x+=v.x;
			other->box.y+=v.y;
		}
	}
	
	//Now check if the player is on the shadow.
	//First make sure they are both alive.
	if(!dead && !other->dead){
		//Get the box of the shadow.
		SDL_Rect boxShadow=other->getBox();
		
		//Check if the player is on top of the shadow.
		if(checkCollision(box,boxShadow)==true){
			//We have collision now check if the other is standing on top of you.
			if(box.y+box.h<=boxShadow.y+13 && !other->inAir){
				int yVelocity=yVel-1;
				if(yVelocity>0){
					box.y-=yVel;
					box.y+=boxShadow.y-(box.y+box.h);
					inAir=false;
					canMove=false;
					onGround=true;
					other->holdingOther=true;
					other->appearance.changeState("holding");
					
					//Change our own appearance to standing.
					if(direction==1){
						appearance.changeState("standleft");
					}else{
						appearance.changeState("standright");
					}
					
					//Apply the velocity the shadow has.
					box.x+=other->xVelBase;
					box.y+=other->yVelBase;
				}
			}else if(boxShadow.y+boxShadow.h<=box.y+13 && !inAir){
				int yVelocity=other->yVel-1;
				if(yVelocity>0){
					other->box.y-=other->yVel;
					other->box.y+=box.y-(other->box.y+other->box.h);
					other->inAir=false;
					other->canMove=false;
					other->onGround=true;
					holdingOther=true;
					appearance.changeState("holding");
					
					//Change our own appearance to standing.
					if(other->direction==1){
						other->appearance.changeState("standleft");
					}else{
						other->appearance.changeState("standright");
					}
					
					//Apply the velocity the shadow has.
					other->box.x+=xVelBase;
					other->box.y+=yVelBase;
				}
			}
		}else{
			holdingOther=false;
			other->holdingOther=false;
		}
	}
	
	//And set currentStand to null.
	objCurrentStand=NULL;
	other->objCurrentStand=NULL;
}

SDL_Rect Player::getBox(){
	return box;
}

void Player::setMyCamera(){
	//Only change the camera when the player isn't dead.
	if(dead)
		return;
	
	//Check if the player is halfway pass the halfright of the screen.
	if(box.x>camera.x+450){
		//It is so ease the camera to the right.
		camera.x+=(box.x-camera.x-400)>>4;
		
		//Check if the camera isn't going too far.
		if(box.x<camera.x+450){
			camera.x=box.x-450;
		}
	}

	//Check if the player is halfway pass the halfleft of the screen.
	if(box.x<camera.x+350){
		//It is so ease the camera to the left.
		camera.x+=(box.x-camera.x-400)>>4;
		
		//Check if the camera isn't going too far.
		if(box.x>camera.x+350){
			camera.x=box.x-350;
		}
	}

	//If the camera is too far to the left we set it to 0.
	if(camera.x<0){
		camera.x=0;
	}
	//If the camera is too far to the right we set it to the max right.
	if(camera.x+camera.w>LEVEL_WIDTH){
		camera.x=LEVEL_WIDTH-camera.w;
	}

	//Check if the player is halfway pass the lower half of the screen.
	if(box.y>camera.y+350){
		//Ir is so ease the camera down.
		camera.y+=(box.y-camera.y-300)>>4;
		
		//Check if the camera isn't going too far.
		if(box.y<camera.y+350){
			camera.y=box.y-350;
		}
	}

	//Check if the player is halfway pass the upper half of the screen.
	if(box.y<camera.y+250){
		//It is so ease the camera up.
		camera.y+=(box.y-camera.y-300)>>4;
		
		//Check if the camera isn't going too far.
		if(box.y>camera.y+250){
			camera.y=box.y-250;
		}
	}

	//If the camera is too far up we set it to 0.
	if(camera.y<0){
		camera.y=0;
	}
	//If the camera is too far down we set it to the max down.
	if(camera.y+camera.h>LEVEL_HEIGHT){
		camera.y=LEVEL_HEIGHT-camera.h;
	}
}

void Player::reset(bool save){
	//Set the location of the player to it's initial state.
	box.x=fx;
	box.y=fy;

	//Reset back to default value.
	inAir=true;
	isJump=false;
	onGround=true;
	shadowCall=false;
	canMove=true;
	holdingOther=false;
	dead=false;
	record=false;
	downKeyPressed=false;
	spaceKeyPressed=false;

	//Some animation variables.
	appearance.resetAnimation(save);
	appearance.changeState("standright");
	direction=0;

	state=0;
	xVel=0; //??? fixed a strange bug in game replay
	yVel=0;

	objCurrentStand=NULL;
	objNotificationBlock=NULL;

	//Clear the recording.
	line.clear();
	playerButton.clear();
	recordButton.clear();
	recordIndex=-1;
#ifdef RECORD_FILE_DEBUG
	recordKeyPressLog.clear();
	recordPlayerPosition.clear();
#endif

	//xVelSaved is used to indicate if there's a state saved or not.
	if(save){
		xVelSaved=0x80000000;
	}
}

void Player::saveState(){
	//We can only save the state when the player isn't dead.
	if(!dead){
		boxSaved.x=box.x;
		boxSaved.y=box.y;
		xVelSaved=xVel;
		yVelSaved=yVel;
		inAirSaved=inAir;
		isJumpSaved=isJump;
		onGroundSaved=onGround;
		canMoveSaved=canMove;
		holdingOtherSaved=holdingOther;
		
		//Let the appearance save.
		appearance.saveAnimation();

		//Save the record
		savedRecordButton=recordButton;
#ifdef RECORD_FILE_DEBUG
		recordKeyPressLog_saved=recordKeyPressLog;
		recordPlayerPosition_saved=recordPlayerPosition;
#endif
		
		//Only play the sound when it's enabled.
		if(getSettings()->getBoolValue("sound")==true){
			//To prevent playing the sound twice, only the player can cause the sound.
			if(!shadow)
				Mix_PlayChannel(-1,saveSound,0);
		}
	}
}

void Player::loadState(){
	//Check with xVelSaved if there's a saved state.
	if(xVelSaved==int(0x80000000)){
		//There isn't so reset the game to load the first initial state.
		//NOTE: There's no need in removing the saved state since there is none.
		reset(false);
		return;
	}
	
	//Restore the saved values.
	box.x=boxSaved.x;
	box.y=boxSaved.y;
	//xVel is set to 0 since it's saved counterpart is used to indicate a saved state.
	xVel=0;
	yVel=yVelSaved;
	
	//Restore teh saved values.
	inAir=inAirSaved;
	isJump=isJumpSaved;
	onGround=onGroundSaved;
	canMove=canMoveSaved;
	holdingOther=holdingOtherSaved;
	dead=false;
	record=false;
	shadowCall=false;
	stateReset();
	
	//Restore the appearance.
	appearance.loadAnimation();
	
	//Clear any recorded stuff.
	line.clear();
	playerButton.clear();

	//Load the previously saved record
	recordButton=savedRecordButton;
#ifdef RECORD_FILE_DEBUG
	recordKeyPressLog=recordKeyPressLog_saved;
	recordPlayerPosition=recordPlayerPosition_saved;
#endif
}

void Player::swapState(Player* other){
	//We need to swap the values of the player with the ones of the given player.
	swap(box.x,other->box.x);
	swap(box.y,other->box.y);
	//NOTE: xVel isn't there since it's used for something else.
	swap(yVel,other->yVel);
	swap(inAir,other->inAir);
	swap(isJump,other->isJump);
	swap(onGround,other->onGround);
	swap(canMove,other->canMove);
	swap(holdingOther,other->holdingOther);
	swap(dead,other->dead);
	
	//Also reset the state of the other.
	other->stateReset();
	
	//Play the swap sound.
	if(getSettings()->getBoolValue("sound")==true){
		Mix_PlayChannel(-1,swapSound,0);
	}
}

bool Player::canSaveState(){
	//We can only save the state if the player isn't dead.
	return !dead;
}

bool Player::canLoadState(){
	//We use xVelSaved to indicate if a state is saved or not.
	return xVelSaved != int(0x80000000);
}

void Player::die(){
	//Make sure the player isn't already dead.
	if(!dead){
		dead=true;
		//If sound is enabled run the hit sound.
		if(getSettings()->getBoolValue("sound")==true){
			Mix_PlayChannel(-1,hitSound,0);
		}
		
		//Change the apearance to die.
		appearance.changeState("die");
	}
	
	//We set the jumpTime to 120 when this is the shadow.
	//That's the countdown for the "Your shadow has died." message.
	if(shadow){
		jumpTime=80;
	}
}
