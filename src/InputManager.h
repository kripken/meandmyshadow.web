/****************************************************************************
** Copyright (C) 2012 me and my shadow developers
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

#ifndef INPUTMANAGER_H
#define INPUTMANAGER_H

#include <SDL/SDL.h>
#include <vector>
#include <string>

enum InputManagerKeys{
	INPUTMGR_UP,
	INPUTMGR_DOWN,
	INPUTMGR_LEFT,
	INPUTMGR_RIGHT,
	INPUTMGR_JUMP,
	INPUTMGR_ACTION,
	INPUTMGR_SPACE,
	INPUTMGR_ESCAPE,
	INPUTMGR_RESTART,
	INPUTMGR_TAB,
	INPUTMGR_SAVE,
	INPUTMGR_LOAD,
	INPUTMGR_SWAP,
	INPUTMGR_TELEPORT,
	INPUTMGR_SUICIDE,
	INPUTMGR_SHIFT,
	INPUTMGR_NEXT,
	INPUTMGR_PREVIOUS,
	INPUTMGR_MAX
};

class InputManager{
public:
	InputManager();
	~InputManager();

	//get and set key code of each key.
	int getKeyCode(InputManagerKeys key,bool isAlternativeKey);
	void setKeyCode(InputManagerKeys key,int keyCode,bool isAlternativeKey);

	//load and save key settings from config file.
	void loadConfig();
	void saveConfig();

	//show the config screen.
	void showConfig();

	//get key name from key code
	static std::string getKeyCodeName(int keyCode);

	//update the key state, according to current SDL event, etc.
	void updateState(bool hasEvent);

	//check if there is KeyDown event.
	bool isKeyDownEvent(InputManagerKeys key);
	//check if there is KeyUp event.
	bool isKeyUpEvent(InputManagerKeys key);
	//check if specified key is down.
	bool isKeyDown(InputManagerKeys key);

	//open all joysticks.
	void openAllJoysitcks();
	//close all joysticks.
	void closeAllJoysticks();
private:
	//the key code of each key.
	// - note of key code:
	//   0 means this key is disabled (??)
	//   1 to 4095 (0xFFF) means keyboard keys, 
	//     currently SDLKey is less than 4095
	//   >= 4096: bit field value means joystick.
	//     0xWWXXYYZZ
	//     WW = joystick index. currently unused, should be 0.
	//     XX = joystick button type: 1-axis 2-button 3-hat, currently ball is unsupported
	//     YY = joystick button index. (we assume joystick has at most 256 buttons)
	//     ZZ = value. if type=axis then value should be 1 or 0xFF.
	//                 if type=button then it's unused.
	//                 if type=hat then it's SDL_HAT_LEFT, SDL_HAT_RIGHT, SDL_HAT_UP or SDL_HAT_DOWN.
	int keys[INPUTMGR_MAX];
	int alternativeKeys[INPUTMGR_MAX];
	//the bit-field flag array saves the key states.
	// 0x1 means the key is down.
	// 0x2 means KeyDown event.
	// 0x4 means KeyUp event.
	int keyFlags[INPUTMGR_MAX];
	//contains all joysticks.
	std::vector<SDL_Joystick*> joysticks;
	//internal function
	int getKeyState(int keyCode,int oldState,bool hasEvent);
};

extern InputManager inputMgr;

#endif //INPUTMANAGER_H
