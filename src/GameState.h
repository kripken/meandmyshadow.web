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
#ifndef GAMESTATE_H
#define GAMESTATE_H


class GameState{
public:
	//Destructor.
	virtual ~GameState(){};
  
	//In this method all the key and mouse events should be handled.
	//Note: The GUIEvents won't be handled here.
	virtual void handleEvents()=0;
	
	//All the logic that needs to be done should go in this method.
	virtual void logic()=0;
	
	//This method handles all the rendering.
	virtual void render()=0;
};

#endif
