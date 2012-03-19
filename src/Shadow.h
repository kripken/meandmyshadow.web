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
#ifndef SHADOW_H
#define SHADOW_H

#include "Player.h"

//The shadow class, it extends the player class since their almost the same.
class Shadow : public Player{
protected:
	//Boolean if the shadow is called by the player.
	//If so the shadow will copy the moves the player made.
	bool called;

	friend class Player;
public:
	//Constructor, it sets a few variables and calls the Player's constructor.
	//objParent: Pointer to the game instance.
	Shadow(Game* objParent);

	//Method that's called before the move function.
	//It's used to let the shadow do his logic, moving and jumping.
	void moveLogic();
	
	//Method used to notify the shadow that he is called.
	//He then must copy the moves that are given to him.
	void meCall();
	
	//Method used to reset the state.
	virtual void stateReset();
	//Method used to load the state.
	virtual void loadState();
};
#endif
