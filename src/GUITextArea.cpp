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

#include "GUITextArea.h"
using namespace std;

GUITextArea::GUITextArea(int left,int top,int width,int height,bool enabled,bool visible):
	GUIObject(left,top,width,height,0,NULL,-1,enabled,visible){
	
	//Set the state 0.
	state=0;
	key=-1;
	keyHoldTime=0;
	keyTime=5;
}

bool GUITextArea::handleEvents(int x,int y,bool enabled,bool visible,bool processed){
	//Boolean if the event is processed.
	bool b=processed;
	
	//The GUIObject is only enabled when he and his parent are enabled.
	enabled=enabled && this->enabled;
	//The GUIObject is only enabled when he and his parent are enabled.
	visible=visible && this->visible;
	
	//Get the absolute position.
	x+=left;
	y+=top;
	
	//NOTE: We don't reset the state to have a "focus" effect.  
	//Only check for events when the object is both enabled and visible.
	if(enabled&&visible){
		//Check if there's a key press and the event hasn't been already processed.
		if(state==2 && event.type==SDL_KEYDOWN && !b){
			//Get the keycode.
			int key=(int)event.key.keysym.unicode;
			
			//Check if the key is supported.
			if(key>=32&&key<=126){
				//Add the key to the string.
				caption.insert((size_t)value,1,char(key));
				value=clamp(value+1,0,caption.length());
				
				//If there is an event callback then call it.
				if(eventCallback){
					GUIEvent e={eventCallback,name,this,GUIEventChange};
					GUIEventQueue.push_back(e);
				}
			}else if(event.key.keysym.sym==SDLK_BACKSPACE){
				//Set the key values correct.
				this->key=SDLK_BACKSPACE;
				keyHoldTime=0;
				keyTime=5;
				
				//Delete one character direct to prevent a lag.
				deleteChar(true);
			}else if(event.key.keysym.sym==SDLK_DELETE){
				//Set the key values correct.
				this->key=SDLK_DELETE;
				keyHoldTime=0;
				keyTime=5;
				
				//Delete one character direct to prevent a lag.
				deleteChar(false);
			}else if(event.key.keysym.sym==SDLK_RETURN){
				//Enter, thus place a newline.
				caption+='\n';
				value=clamp(value+1,0,caption.length());
				
				//If there is an event callback then call it.
				if(eventCallback){
					GUIEvent e={eventCallback,name,this,GUIEventChange};
					GUIEventQueue.push_back(e);
				}
			}else if(event.key.keysym.sym==SDLK_RIGHT){
				//Set the key values correct.
				this->key=SDLK_RIGHT;
				keyHoldTime=0;
				keyTime=5;
				
				//Move the carrot once to prevent a lag.
				value=clamp(value+1,0,caption.length());
			}else if(event.key.keysym.sym==SDLK_LEFT){
				//Set the key values correct.
				this->key=SDLK_LEFT;
				keyHoldTime=0;
				keyTime=5;
				
				//Move the carrot once to prevent a lag.
				value=clamp(value-1,0,caption.length());
			}
			
			//The event has been processed.
			b=true;
		}else if(state==2 && event.type==SDL_KEYUP && !b){
			//Check if released key is the same as the holded key.
			if(event.key.keysym.sym==key){
				//It is so stop the key.
				key=-1;
			}
		}
		
		//The mouse location (x=i, y=j) and the mouse button (k).
		int i,j,k;
		k=SDL_GetMouseState(&i,&j);
		
		//Check if the mouse is inside the GUIObject.
		if(i>=x&&i<x+width&&j>=y&&j<y+height){
			//We can only increase our state. (nothing->hover->focus).
			if(state!=2){
				state=1;
			}
			
			//Check for a mouse button press.
			if(k&SDL_BUTTON(1)){
				//We have focus.
				state=2;
				//TODO Move carrot to place clicked 
				value=caption.length();
			}
		}else{
			//The mouse is outside the TextBox.
			//If we don't have focus but only hover we lose it.
			if(state==1){
				state=0;
			}
			
			//If it's a click event outside the textbox then we blur.
			if(event.type==SDL_MOUSEBUTTONUP && event.button.button==SDL_BUTTON_LEFT){
				//Set state to 0.
				state=0;
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

void GUITextArea::deleteChar(bool back){
	//Boolean if an event should be called.
	bool event=false;
	
	//Check if it's backspace or delete.
	if(back){
		//We need to remove a character so first make sure that there is text.
		if(caption.length()>0&&value>0){
			//Remove the character before the carrot. 
			value=clamp(value-1,0,caption.length()); 
			caption.erase((size_t)value,1); 
			
			//Set event true.
			event=true;
		}
	}else{
		if(caption.length()>0){
			//Remove the character after the carrot.
			value=clamp(value,0,caption.length());
			caption.erase((size_t)value,1);
			
			//Set event true.
			event=true;
		}
	}
	
	//If there is an event callback and a character is removed then call it.
	if(event && eventCallback){
		GUIEvent e={eventCallback,name,this,GUIEventChange};
		GUIEventQueue.push_back(e);
	}
}

void GUITextArea::render(int x,int y){
	//FIXME: Logic in the render method since that is update constant.
	if(key!=-1){
		//Increase the key time.
		keyHoldTime++;
		//Make sure the deletionTime isn't to short.
		if(keyHoldTime>=keyTime){
			keyHoldTime=0;
			keyTime--;
			if(keyTime<1)
				keyTime=1;
			
			//Now check the which key it was.
			switch(key){
				case SDLK_BACKSPACE:
					deleteChar(true);
					break;
				case SDLK_DELETE:
					deleteChar(false);
					break;
				case SDLK_LEFT:
					value=clamp(value-1,0,caption.length());
					break;
				case SDLK_RIGHT:
					value=clamp(value+1,0,caption.length());
					break;  
			}
		}
	}
	
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
	//If hovering or focused make background more visible.
	if(state==1) 
		clr=255;
	else if (state==2)
		clr=230;
	
	//Draw the box.
	Uint32 color=0xFFFFFF00|clr;
	drawGUIBox(x,y,width,height,screen,color);
	
	r.x=x+1;
	r.y=y+1;
	r.w=width-2;
	r.h=height-2;
	
	//Pointer to the string.
	char* lps=(char*)caption.c_str();
	//Pointer to a character.
	char* lp=NULL;

	//The color black.
	SDL_Color black={0,0,0,0};
	//The surface that will hold the text.
	SDL_Surface* bm=NULL;
	
	//We keep looping forever.
	//The only way out is with the break statement.
	for(;;){
		//As long as it's still the same sentence we continue.
		//It will stop when there's a newline or end of line.
		for(lp=lps;*lp!='\n'&&*lp!='\r'&&*lp!=0;lp++);
		
		//Store the character we stopped on. (End or newline)
		char c=*lp;
		//Set the character in the string to 0, making lps a string containing one sentence.
		*lp=0;
		
		//Draw the black text.
		bm=TTF_RenderText_Blended(fontText,lps,black);
		
		//Draw the text.
		SDL_Rect tmp={0,0,width-2,25};
		SDL_BlitSurface(bm,&tmp,screen,&r);
		//NOTE: We free the surface later.
		
		//Increase y with 25, about the height of the text.
		r.y+=25;
		
		//Check the stored character if it was a stop.
		if(c==0){
			//It was so break out of the for loop.
			lps=lp;
			break;
		}
		//It wasn't meaning more will follow.
		//We set lps to point after the "newline" forming a new string.
		lps=lp+1;
		//Restore the character, this way the original string doesn't get destroyed.
		*lp=c;
	}
	
	//Only draw the carrot when focus.
	if(state==2){
		r.x=x;
		r.y=y+4;
		r.w=2;
		r.h=20;
		
		//Check for any newlines.
		size_t pos=0;
		while( (caption.find('\n',pos+1)!=string::npos) && (int(caption.find('\n',pos+1))<value)){
			pos=caption.find('\n',pos+1);
			r.y+=24;
		}
		
		int advance;
		for(int n=pos;n<value;n++){ 
			if(caption[n]!='\n'){
				TTF_GlyphMetrics(fontText,caption[n],NULL,NULL,NULL,NULL,&advance); 
				r.x+=advance;
			}
		}
		
		//Make sure that the carrot is inside the textbox.
		if(r.x<x+width)
			SDL_FillRect(screen,&r,0);
	}
	
	//Anyway we free the bm surface.
	if(bm!=NULL)
		SDL_FreeSurface(bm);
	
	//We now need to draw all the children of the GUIObject.
	for(unsigned int i=0;i<childControls.size();i++){
		childControls[i]->render(x,y);
	}
}
