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

#include "GUIListBox.h"
using namespace std;

GUIListBox::GUIListBox(int left,int top,int width,int height,bool enabled,bool visible):
GUIObject(left,top,width,height,0,NULL,-1,enabled,visible){
	//Set the state -1.
	state=-1;
	
	//Create the scrollbar and add it to the children.
	scrollBar=new GUIScrollBar(0,0,16,0,1,0,0,0,0,0,true,false);
	childControls.push_back(scrollBar);
}

bool GUIListBox::handleEvents(int x,int y,bool enabled,bool visible,bool processed){
	//Boolean if the event is processed.
	bool b=processed;
	
	//The GUIObject is only enabled when he and his parent are enabled.
	enabled=enabled && this->enabled;
	//The GUIObject is only enabled when he and his parent are enabled.
	visible=visible && this->visible;
	
	//Get the absolute position.
	x+=left;
	y+=top;
	
	//Calculate the scrollbar position.
	scrollBar->left=width-16;
	scrollBar->height=height;
	int m=item.size(),n=(height-4)/24;
	if(m>n){
		scrollBar->maxValue=m-n;
		scrollBar->smallChange=1;
		scrollBar->largeChange=n;
		scrollBar->visible=true;
		b=b||scrollBar->handleEvents(x,y,enabled,visible,b);
	}else{
		scrollBar->value=0;
		scrollBar->maxValue=0;
		scrollBar->visible=false;
	}
	
	//Set state negative.
	state=-1;
	
	//Check if the GUIListBox is visible, enabled and no event has been processed before.
	if(enabled&&visible&&!b){
		//The mouse location (x=i, y=j) and the mouse button (k).
		int i,j;
		SDL_GetMouseState(&i,&j);
		
		//Convert the mouse location to a relative location.
		i-=x+2;
		j-=y+2;
		
		//Check if the mouse is inside the GUIListBox.
		if(i>=0&&i<width-4&&j>=0&&j<height-4){
			//Calculate the y location with the scrollbar position.
			int idx=j/24+scrollBar->value;
			
			//If the entry isn't above the max we have an event.
			if(idx>=0&&idx<(int)item.size()){
				state=idx;
				
				//Check if the left mouse button is pressed.
				if(event.type==SDL_MOUSEBUTTONDOWN && event.button.button==SDL_BUTTON_LEFT && value!=idx){
					value=idx;
					
					//If event callback is configured then add an event to the queue.
					if(eventCallback){
						GUIEvent e={eventCallback,name,this,GUIEventClick};
						GUIEventQueue.push_back(e);
					}
				}
			}
			
			//Check for mouse wheel scrolling.
			if(event.type==SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_WHEELDOWN && scrollBar->enabled){
				scrollBar->value+=4;
				if(scrollBar->value > scrollBar->maxValue)
					scrollBar->value = scrollBar->maxValue;
			}else if(event.type==SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_WHEELUP && scrollBar->enabled){
				scrollBar->value-=4;
				if(scrollBar->value < 0)
					scrollBar->value = 0;
			}
		}
	}
	
	//Process child controls event except for the scrollbar.
	//That's why i starts at one.
	for(unsigned int i=1;i<childControls.size();i++){
		bool b1=childControls[i]->handleEvents(x,y,enabled,visible,b);
		
		//The event is processed when either our or the childs is true (or both).
		b=b||b1;
	}
	return b;
}

void GUIListBox::render(int x,int y){
	//Rectangle the size of the GUIObject, used to draw borders.
	SDL_Rect r;
	//There's no need drawing the GUIObject when it's invisible.
	if(!visible) 
		return;
	
	//Get the absolute x and y location.
	x+=left;
	y+=top;
	
	//Default background opacity
	int clr=128;
	//TODO: Add hover check?
	
	//Draw the box.
	Uint32 color=0xFFFFFFFF|clr;
	drawGUIBox(x,y,width,height,screen,color);
	
	//We need to draw the items.
	//The number of items.
	int m=item.size();
	//The number of items that are visible.
	int n=(height-4)/24;
	//Integer containing the current entry that is being drawn.
	int i;
	//The y coordinate the current entries reaches.
	int j;
	
	//If the number of items is higer than fits on the screen set the begin value (m) to scrollBar->value+n.
	if(m>scrollBar->value+n)
		m=scrollBar->value+n;
	
	//Loop through the (visible) entries and draw them.
	for(i=scrollBar->value,j=y+1;i<m;i++,j+=24){
		//The background color for the entry.
		int clr=-1;
		//If i is the selected entry then give it a light gray background.
		if(value==i){
			clr=0xDDDDDDFF;
		}
		
		//Check if the current entry is selected. If so draw borders around it.
		if(state==i)
			drawGUIBox(x,j-1,width,25,screen,0x00000000);
		
		//Only draw when clr isn't -1.
		if(clr!=-1)
			drawGUIBox(x,j-1,width,25,screen,clr);
		
		//Now draw the text.
		const char* s=item[i].c_str();
		//Make sure the text isn't empty.
		if(s && s[0]){
			//Render black text.
			SDL_Color black={0,0,0,0};
			SDL_Surface *bm=TTF_RenderText_Blended(fontText,s,black);
			
			//Calculate the text location, center it vertically.
			r.x=x+4;
			r.y=j+12-bm->h/2;
			
			//Draw the text and free the rendered surface.
			SDL_BlitSurface(bm,NULL,screen,&r);
			SDL_FreeSurface(bm);
		}
	}
	
	//We now need to draw all the children of the GUIObject.
	for(unsigned int i=0;i<childControls.size();i++){
		childControls[i]->render(x,y);
	}
}

GUISingleLineListBox::GUISingleLineListBox(int left,int top,int width,int height,bool enabled,bool visible):
GUIObject(left,top,width,height,0,NULL,-1,enabled,visible),animation(0){}

bool GUISingleLineListBox::handleEvents(int x,int y,bool enabled,bool visible,bool processed){
	//Boolean if the event is processed.
	bool b=processed;
	
	//The GUIObject is only enabled when he and his parent are enabled.
	enabled=enabled && this->enabled;
	//The GUIObject is only enabled when he and his parent are enabled.
	visible=visible && this->visible;
	
	//Get the absolute position.
	x+=left;
	y+=top;
	
	state&=~0xF;
	if(enabled&&visible){
		//The mouse location (x=i, y=j) and the mouse button (k).
		int i,j,k;
		k=SDL_GetMouseState(&i,&j);
		
		//Convert the mouse location to a relative location.
		i-=x;
		j-=y;
		
		//The selected button.
		//0=nothing 1=left 2=right.
		int idx=0;

		//Check which button the mouse is above.
		if(i>=0&&i<width&&j>=0&&j<height){
			if(i<26 && i<width/2){
				//The left arrow.
				idx=1;
			}else if(i>=width-26){
				//The right arrow.
				idx=2;
			}
		}
		
		//If idx is 0 it means the mous doesn't hover any arrow so reset animation.
		if(idx==0)
			animation=0;
		
		//Check if there's a mouse button press or not.
		if(k&SDL_BUTTON(1)){
			if(((state>>4)&0xF)==idx) 
				state|=idx;
		}else{
			state|=idx;
		}
		
		//Check if there's a mouse press.
		if(event.type==SDL_MOUSEBUTTONDOWN && event.button.button==SDL_BUTTON_LEFT && idx){
			state=idx|(idx<<4);
		}else if(event.type==SDL_MOUSEBUTTONUP && event.button.button==SDL_BUTTON_LEFT && idx && ((state>>4)&0xF)==idx){
			int m=(int)item.size();
			if(m>0){
				if(idx==2){
					idx=value+1;
					if(idx<0||idx>=m) idx=0;
					if(idx!=value){
						value=idx;
						
						//If there is an event callback then call it.
						if(eventCallback){
							GUIEvent e={eventCallback,name,this,GUIEventClick};
							GUIEventQueue.push_back(e);
						}
					}
				}else if(idx==1){
					idx=value-1;
					if(idx<0||idx>=m) idx=m-1;
					if(idx!=value){
						value=idx;
						
						//If there is an event callback then call it.
						if(eventCallback){
							GUIEvent e={eventCallback,name,this,GUIEventClick};
							GUIEventQueue.push_back(e);
						}
					}
				}
			}
		}
		if(event.type==SDL_MOUSEBUTTONUP) state&=0xF;
	}else{
		//Set state zero.
		state=0;
	}
	
	//Also let the children handle their events.
	for(unsigned int i=0;i<childControls.size();i++){
		bool b1=childControls[i]->handleEvents(x,y,enabled,visible,b);
		
		//The event is processed when either our or the childs is true (or both).
		b=b||b1;
	}
	return b;
}

void GUISingleLineListBox::render(int x,int y){
	//Rectangle the size of the GUIObject, used to draw borders.
	SDL_Rect r;
	
	//There's no need drawing the GUIObject when it's invisible.
	if(!visible) 
		return;
	
	//NOTE: logic in the render method since it's the only part that gets called every frame.
	if((state&0xF)==0x1 || (state&0xF)==0x2){
		animation++;
		if(animation>20)
			animation=-20;
	}
	
	//Get the absolute x and y location.
	x+=left;
	y+=top;
	
	//Draw the text.
	if(value>=0 && value<(int)item.size()){
		//Get the text.
		const char* lp=item[value].c_str();
		
		//Check if the text is empty or not.
		if(lp!=NULL && lp[0]){
			//Render black text.
			SDL_Color black={0,0,0,0};
			SDL_Surface* bm=TTF_RenderText_Blended(fontGUI,lp,black);
			
			//Center the text both vertically as horizontally.
			r.x=x+(width-bm->w)/2;
			r.y=y+(height-bm->h)/2;
			
			//Draw the text and free the surface afterwards.
			SDL_BlitSurface(bm,NULL,screen,&r);
			SDL_FreeSurface(bm);
		}
	}
	
	//Draw the arrows.
	SDL_Rect r2={48,0,16,16};
	r.x=x;
	if((state&0xF)==0x1)
		r.x+=abs(animation/2);
	r.y=y+(height-16)/2;
	SDL_BlitSurface(bmGUI,&r2,screen,&r);
	r2.x=64;
	r.x=x+width-16;
	if((state&0xF)==0x2)
		r.x-=abs(animation/2);
	SDL_BlitSurface(bmGUI,&r2,screen,&r);
	
	//We now need to draw all the children of the GUIObject.
	for(unsigned int i=0;i<childControls.size();i++){
		childControls[i]->render(x,y);
	}
}
