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

#include "GUIScrollBar.h"
using namespace std;

void GUIScrollBar::calcPos(){
	//Floats ...
	float f,f1,f2;
	
	//The value can't be below the minimum value or above the maximum.
	if(value<minValue)
		value=minValue;
	else if(value>maxValue)
		value=maxValue;
	
	//
	if(orientation){
		f=(float)(top+16);
		f2=(float)(height-32);
	}else{
		f=(float)(left+16);
		f2=(float)(width-32);
	}
	if(largeChange<=0) f2=-1;
	if(f2>0){
		valuePerPixel = (maxValue - minValue + largeChange) / f2;
		if(valuePerPixel > 0.0001f) f1 = largeChange / valuePerPixel;
		if(f1 < 4 && f2 > 4){
			valuePerPixel = (maxValue - minValue) / (f2 - 4);
			f1 = 4;
		}
		thumbStart = f + (value - minValue) / valuePerPixel;
		thumbEnd = thumbStart + f1;
	}else{
		valuePerPixel = -1;
		thumbStart = f;
		thumbEnd = f - 1;
	}
}

bool GUIScrollBar::handleEvents(int x,int y,bool enabled,bool visible,bool processed){
	//Boolean if the event is processed.
	bool b=processed;
	
	//The GUIObject is only enabled when he and his parent are enabled.
	enabled=enabled && this->enabled;
	//The GUIObject is only enabled when he and his parent are enabled.
	visible=visible && this->visible;
	
	//Check if the mouse button is released.
	if(event.type==SDL_MOUSEBUTTONUP || !(enabled&&visible)){
		//It so we have lost any focus at all.
		state=0;
	}else if(event.type==SDL_MOUSEMOTION || event.type==SDL_MOUSEBUTTONDOWN){
		//The mouse button is down and it's moving
		int i,j,k,f,f1,f2,f3;
		state&=~0xFF;
		k=SDL_GetMouseState(&i,&j);
		i-=x;
		j-=y;
		bool bInControl_0;
		if(orientation){
			f=top;
			f1=f+height;
			bInControl_0=(i>=left&&i<left+width);
			i=j;
		}else{
			f=left;
			f1=f+width;
			bInControl_0=(j>=top&&j<top+height);
		}
		//===
		if((state&0x0000FF00)==0x300&&(k&SDL_BUTTON(1))&&event.type==SDL_MOUSEMOTION&&valuePerPixel>0){
			//drag thumb
			state|=3;
			int val = criticalValue + (int)(((float)i - startDragPos) * valuePerPixel + 0.5f);
			if(val<minValue) val=minValue;
			else if(val>maxValue) val=maxValue;
			if(value!=val){
				value=val;
				changed=true;
			}
			b=true;
		}else if(bInControl_0){
			if(valuePerPixel > 0){
				f2=f+16;
				f3=f1-16;
			}else{
				f2=f3=(f+f1)/2;
			}
			if(i<f){ //do nothing
			}else if(i<f2){ //-smallchange
				state=(state&~0xFF)|1;
				if(event.type==SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) state=(state&~0x0000FF00)|((state&0xFF)<<8);
				else if((state&0x0000FF00)&&((state&0xFF)!=((state>>8)&0xFF))) state&=~0xFF;
				if(event.type==SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT){
					int val=value-smallChange;
					if(val<minValue) val=minValue;
					if(value!=val){
						value=val;
						changed=true;
					}
					timer=8;
				}
				b=true;
			}else if(i>=f3 && i<f1){ //+smallchange
				state=(state&~0xFF)|5;
				if(event.type==SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) state=(state&~0x0000FF00)|((state&0xFF)<<8);
				else if((state&0x0000FF00)&&((state&0xFF)!=((state>>8)&0xFF))) state&=~0xFF;
				if(event.type==SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT){
					int val=value+smallChange;
					if(val>maxValue) val=maxValue;
					if(value!=val){
						value=val;
						changed=true;
					}
					timer=8;
				}
				b=true;
			}else if(valuePerPixel<=0){ //do nothing
			}else if(i<(int)thumbStart){ //-largechange
				state=(state&~0xFF)|2;
				if(event.type==SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) state=(state&~0x0000FF00)|((state&0xFF)<<8);
				else if((state&0x0000FF00)&&((state&0xFF)!=((state>>8)&0xFF))) state&=~0xFF;
				if(event.type==SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT){
					int val=value-largeChange;
					if(val<minValue) val=minValue;
					if(value!=val){
						value=val;
						changed=true;
					}
					timer=8;
				}
				if(state&0xFF) criticalValue = minValue + (int)(float(i - f2) * valuePerPixel + 0.5f);
				b=true;
			}else if(i<(int)thumbEnd){ //start drag
				state=(state&~0xFF)|3;
				if(event.type==SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) state=(state&~0x0000FF00)|((state&0xFF)<<8);
				else if((state&0x0000FF00)&&((state&0xFF)!=((state>>8)&0xFF))) state&=~0xFF;
				if(event.type==SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT){
					criticalValue=value;
					startDragPos = (float)i;
				}
				b=true;
			}else if(i<f3){ //+largechange
				state=(state&~0xFF)|4;
				if(event.type==SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) state=(state&~0x0000FF00)|((state&0xFF)<<8);
				else if((state&0x0000FF00)&&((state&0xFF)!=((state>>8)&0xFF))) state&=~0xFF;
				if(event.type==SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT){
					int val=value+largeChange;
					if(val>maxValue) val=maxValue;
					if(value!=val){
						value=val;
						changed=true;
					}
					timer=8;
				}
				if(state&0xFF) criticalValue = minValue - largeChange + (int)(float(i - f2) * valuePerPixel + 0.5f);
				b=true;
			}
		}
	}
	
	//Get the absolute position.
	x+=left;
	y+=top;
	
	//Also let the children handle their events.
	for(unsigned int i=0;i<childControls.size();i++){
		bool b1=childControls[i]->handleEvents(x,y,enabled,visible,b);
		
		//The event is processed when either our or the childs is true (or both).
		b=b||b1;
	}
	return b;
}

void GUIScrollBar::renderScrollBarButton(int index,int x1,int y1,int x2,int y2,int srcleft,int srctop){
	//Make sure the button isn't inverted.
	if(x2<=x1||y2<=y1)
		return;
	
	//The color.
	int clr=-1;
	//Rectangle the size of the button.
	SDL_Rect r={x1,y1,x2-x1,y2-y1};
	
	//Check 
	if((state&0xFF)==index){
		if(((state>>8)&0xFF)==index){
			//Set the color gray.
			clr=0xDDDDDDFF;
		}else{
			//Set the color to lightgray.
			clr=0xFFFFFFFF;
		}
	}
	
	//Draw a box.
	drawGUIBox(r.x,r.y,r.w,r.h,screen,clr);
	
	//Boolean if there should be an image on the button.
	bool b;
	//The check depends on the orientation.
	if(orientation)
		b=(y2-y1>=14);
	else
		b=(x2-x1>=14);
	
	//Check if the image can be drawn.
	if(b&&srcleft>=0&&srctop>=0){
		//It can thus draw it.
		SDL_Rect r1={srcleft,srctop,16,16};
		r.x=(x1+x2)/2-8;
		r.y=(y1+y2)/2-8;
		SDL_BlitSurface(bmGUI,&r1,screen,&r);
	}
}

void GUIScrollBar::render(int x,int y){
	//There's no use in rendering the scrollbar when invisible.
	if(!visible)
		return;
	
	//Check if the scrollbar is enabled.
	if(enabled){
		//Check if the state is right.
		if((state&0xFF)==((state>>8)&0xFF)){
			//Switch the state (change)/
			switch(state&0xFF){
			case 1:
				//It's a small negative change.
				//Check if it's time.
				if((--timer)<=0){
					//Reduce the value.
					int val=value-smallChange;
					
					//Make sure it doesn't go too low.
					if(val<minValue)
						val=minValue;
					if(value!=val){
						value=val;
						changed=true;
					}
					
					//Set the time to two.
					timer=2;
				}
				break;
			case 2:
				//It's a lager negative change.
				//Check if it's time.
				if((--timer)<=0){
					if(value<criticalValue)
						state&=~0xFF;
					else{
						//Reduce the value.
						int val=value-largeChange;
						
						//Make sure it doesn't go too low.
						if(val<minValue)
							val=minValue;
						if(value!=val){
							value=val;
							changed=true;
						}
						
						//Set the time to two.
						timer=2;
					}
				}
				break;
			case 4:
				//It's a lager positive change.
				//Check if it's time.
				if((--timer)<=0){
					if(value>criticalValue)
						state&=~0xFF;
					else{
						//Increase the value.
						int val=value+largeChange;
						
						//Make sure it doesn't go too high.
						if(val>maxValue)
							val=maxValue;
						if(value!=val){
							value=val;
							changed=true;
						}
						
						//Set the time to two.
						timer=2;
					}
				}
				break;
			case 5: 
				//It's a small positive change.
				//Check if it's time.
				if((--timer)<=0){
					//Increase the value.
					int val=value+smallChange;
					
					//Make sure ti doesn't go too high.
					if(val>maxValue)
						val=maxValue;
					if(value!=val){
						value=val;
						changed=true;
					}
					
					//Set the time to two.
					timer=2;
				}
				break;
			}
		}
	}
	
	//If the scrollbar changed then invoke a GUIEvent.
	if(changed){
		if(eventCallback){
			GUIEvent e={eventCallback,name,this,GUIEventChange};
			GUIEventQueue.push_back(e);
		}
		changed=false;
	}
	
	//We calculate the position since it could have changed.
	calcPos();
	
	//Now the actual drawing begins.
	if(orientation){
		//The scrollbar is vertically orientated.
		if(valuePerPixel>0){
			//There are four buttons so draw them.
			renderScrollBarButton(2,x+left,y+top,x+left+width,y+top+height,-1,-1);
			renderScrollBarButton(1,x+left,y+top,x+left+width,y+top+16,80,0);
			renderScrollBarButton(3,x+left,y-1+(int)thumbStart,x+left+width,y+1+(int)thumbEnd,0,16);
			renderScrollBarButton(5,x+left,y+top+height-16,x+left+width,y+top+height,96,0);
		}else{
			//There are two buttons so draw them.
			int f=top+height/2;
			renderScrollBarButton(1,x+left,y+top,x+left+width,y+1+f,80,0);
			renderScrollBarButton(5,x+left,y+f,x+left+width,y+top+height,96,0);
		}
	}else{
		//The scrollbar is horizontally orientated.
		if(valuePerPixel>0){
			//There are five buttons so draw them.
			renderScrollBarButton(1,x+left,y+top,x+left+16,y+top+height,48,0);
			renderScrollBarButton(2,x+left+15,y+top,x+(int)thumbStart,y+top+height,-1,-1);
			renderScrollBarButton(3,x-1+(int)thumbStart,y+top,x+1+(int)thumbEnd,y+top+height,16,16);
			renderScrollBarButton(4,x+(int)thumbEnd,y+top,x+left+width-15,y+top+height,-1,-1);
			renderScrollBarButton(5,x+left+width-16,y+top,x+left+width,y+top+height,64,0);
		}else{
			//There are two buttons so draw them.
			int f=left+width/2;
			renderScrollBarButton(1,x+left,y+top,x+1+f,y+top+height,48,0);
			renderScrollBarButton(5,x+f,y+top,x+left+width,y+top+height,64,0);
		}
	}
	
	//Get the absolute position.
	x+=left;
	y+=top;
	
	//We now need to draw all the children of the GUIObject.
	for(unsigned int i=0;i<childControls.size();i++){
		childControls[i]->render(x,y);
	}
}
