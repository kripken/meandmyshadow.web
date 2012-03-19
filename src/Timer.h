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
#ifndef TIMER_H
#define TIMER_H

//Timer class used to keep the framerate constant.
class Timer{
private:
	//Integer containing the number of ticks when the Timer started.
	int ticks;
public:
	//This will start the timer.
	//What it does is set ticks(the starttime) to SDL_GetTicks().
	void start();
	
	//This will return the number of ticks that have passed.
	//Returns: The number of ticks since the start of the timer.
	int getTicks();
};
#endif
