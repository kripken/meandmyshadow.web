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
#ifndef LEVELEDITSELECT_H
#define LEVELEDITSELECT_H

#include "LevelSelect.h"
#include "GameState.h"
#include "GameObjects.h"
#include "Player.h"
#include "GUIObject.h"
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#include <SDL/SDL_ttf.h>
#include <vector>
#include <string>

//This is the LevelEditSelect state, here you can select levelpacks and levels.
class LevelEditSelect :public LevelSelect{
private:
	//Pointer to the GUIObjectRoot of the levelselect main gui.
	GUIObject* levelEditGUIObjectRoot;
  
	//Pointer to the new levelpack textfield.
	GUIObject* levelpackName;
	
	//Pointer to the remove levelpack button.
	GUIObject* propertiesPack;
	//Pointer to the remove levelpack button.
	GUIObject* removePack;
	
	//Pointer to the move map button.
	GUIObject* move;
	//Pointer to the remove map button.
	GUIObject* remove;
	//Pointer to the edit map button.
	GUIObject* edit;
	
	//String that contains the name of the levelpack.
	std::string packName;
	
	//Method that will list the levelpacks and change the listbox field.
	void listPacks();
	
	//Method that should be called when changing the current levelpack in an abnormal way.
	void changePack();
	
	//This method will show a popup with levelpack specific settings.
	void packProperties();
	
	//This method will show an add level dialog.
	void addLevel();
	
	//This method will show an move level dialog.
	void moveLevel();
public:
	//Constructor.
	LevelEditSelect();
	//Destructor.
	~LevelEditSelect();
	
	//Inherited from LevelSelect.
	void refresh();
	void selectNumber(unsigned int number,bool selected);
	
	//Inherited from GameState.
	void render();

	//Inherited from LevelSelect.
	void renderTooltip(unsigned int number,int dy);
	
	//GUI events will be handled here.
	void GUIEventCallback_OnEvent(std::string name,GUIObject* obj,int eventType);
};

#endif
