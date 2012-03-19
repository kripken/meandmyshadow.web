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
#ifndef GUISCROLLBAR_H
#define GUISCROLLBAR_H

#include "GUIObject.h"

//Constant integers containing the two possible orientations of the scrollbar.
const int ScrollBarHorizontal=0;
const int ScrollBarVertical=1;

class GUIScrollBar:public GUIObject{
public:
	//The minimum value of the scrollbar.
	int minValue;
	//The maximum value of the scrollbar.
	int maxValue;
	
	//The step size when a small step is made.
	int smallChange;
	//The step size when a large step is made.
	int largeChange;
	
	//The orientation of the scrollbar.
	int orientation;
private:
	//
	float thumbStart;
	//
	float thumbEnd;
	float valuePerPixel;
	//Float containing the start position when dragging the scrollbar.
	float startDragPos;
	
	int criticalValue;
	int timer;
	
	//Boolean if the scrollbar position has changed.
	bool changed;
	
	//Method that will calculate the position of the scrollbar.
	void calcPos();
	
	//Method that will render a scrollbar button.
	//index: 
	//x1:
	//y1:
	//x2:
	//y2:
	//srcLeft:
	//srcRight:
	void renderScrollBarButton(int index,int x1,int y1,int x2,int y2,int srcLeft,int srcTop);
public:
	GUIScrollBar(int left=0,int top=0,int width=0,int height=0,int orientation=0,
		int value=0,int minValue=0,int maxValue=100,int smallChange=10,int largeChange=50,
		bool enabled=true,bool visible=true):
		GUIObject(left,top,width,height,0,NULL,value,enabled,visible),
		minValue(minValue),maxValue(maxValue),smallChange(smallChange),largeChange(largeChange),orientation(orientation),
		criticalValue(0),timer(0),changed(false)
	{
		//In the constructor we simply call calcPos().
		calcPos();
	}
	
	
	//Method used to handle mouse and/or key events.
	//x: The x mouse location.
	//y: The y mouse location.
	//enabled: Boolean if the parent is enabled or not.
	//visible: Boolean if the parent is visible or not.
	//processed: Boolean if the event has been processed (by the parent) or not.
	//Returns: Boolean if the event is processed by the child.
	virtual bool handleEvents(int x=0,int y=0,bool enabled=true,bool visible=true,bool processed=false);
	//Method that will render the GUIScrollBar.
	//x: The x location to draw the GUIObject. (x+left)
	//y: The y location to draw the GUIObject. (y+top)
	virtual void render(int x=0,int y=0);
};

#endif
