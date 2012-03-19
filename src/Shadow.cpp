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
#include "FileManager.h"
#include "Game.h"
#include "Player.h"
#include "Shadow.h"
#include <vector>
#include <iostream>
using namespace std;

Shadow::Shadow(Game* objParent):Player(objParent){
	//Most of the initialising happens in the Player's constructor.
	//Here we only set some shadow specific options.
	called=false;
	shadow=true;
}

void Shadow::moveLogic(){
	//If we're called and there are still moves left we to that move.
	if(called && state < (signed)playerButton.size()){
		int currentKey=playerButton[state];

		xVel=0;
		//Check if the current move is walking.
		if(currentKey & PlayerButtonRight) xVel=7;
		if(currentKey & PlayerButtonLeft) xVel=-7;

		//Check if the current move is jumping.
		if((currentKey & PlayerButtonJump) && !inAir){
			isJump=true;
		}else{
			isJump=false;
		}

		//Check if the current move is an action (DOWN arrow key).
		if(currentKey & PlayerButtonDown){
			downKeyPressed=true;
		}else{
			downKeyPressed=false;
		}

		//We've done the move so move on to the next one.
		state++;
	}else{
		//We ran out of moves so reset it.
		//FIXME: Every frame when called is false this will be done?
		called=false;
		state=0;
		xVel=0;
	}
}

void Shadow::meCall(){
	called=true;
}

void Shadow::stateReset(){
	state=0;
	called=false;
}

void Shadow::loadState(){
	Player::loadState();
	called=false;
	playerButton.clear();
}
