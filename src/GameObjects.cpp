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
#include "Functions.h"
#include "Globals.h"
#include "Player.h"

GameObject::GameObject(Game* parent):parent(parent){}
GameObject::~GameObject(){}

SDL_Rect GameObject::getBox(int boxType){
	//This is the default implementation of getBox(int) method.
	switch(boxType){
	case BoxType_Current:
	case BoxType_Base:
	case BoxType_Previous:
		return box;
	}
	
	//Return an empty SDL_Rect.
	SDL_Rect tmp={0,0,0,0};
	return tmp;
}

void GameObject::setPosition(int x,int y){
	box.x=x;
	box.y=y;
}

void GameObject::saveState(){}
void GameObject::loadState(){}
void GameObject::reset(bool save){}

void GameObject::playAnimation(int flags){}
void GameObject::onEvent(int eventType){}

int GameObject::queryProperties(int propertyType,Player* obj){
	return 0;
}

void GameObject::getEditorData(std::vector<std::pair<std::string,std::string> >& obj){}
void GameObject::setEditorData(std::map<std::string,std::string>& obj){}

void GameObject::move(){}
