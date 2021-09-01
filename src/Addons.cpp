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
#include "Addons.h"
#include "GameState.h"
#include "Functions.h"
#include "FileManager.h"
#include "Globals.h"
#include "Objects.h"
#include "GUIObject.h"
#include "GUIScrollBar.h"
#include "GUIListBox.h"
#include "POASerializer.h"
#include "InputManager.h"
#include <string>
#include <sstream>
#include <iostream>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL.h>
using namespace std;

Addons::Addons(){
	//Load the backgroundimage and the addons file.
	background=loadImage(getDataPath()+"gfx/menu/background.png");
	//Render the title.
	SDL_Color black={0,0,0};
	title=TTF_RenderText_Blended(fontTitle,"Addons",black);
	
	FILE* addon=fopen((getUserPath(USER_CACHE)+"addons").c_str(),"wb");
	action=NONE;

	addons=NULL;

	//Create the gui.
	GUIObject* obj;
	if(GUIObjectRoot){
		delete GUIObjectRoot;
		GUIObjectRoot=NULL;
	}

	//Try to get(download) the addonsList.
	if(getAddonsList(addon)==false) {
		//It failed so we show the error message.
		GUIObjectRoot=new GUIObject(0,0,800,600);

		obj=new GUIObject(90,96,200,32,GUIObjectLabel,"Unable to initialze addon menu:");
		obj->name="lbl";
		GUIObjectRoot->childControls.push_back(obj);
		
		obj=new GUIObject(120,130,200,32,GUIObjectLabel,error.c_str());
		obj->name="lbl";
		GUIObjectRoot->childControls.push_back(obj);
		
		obj=new GUIObject(90,550,200,32,GUIObjectButton,"Back");
		obj->name="cmdBack";
		obj->eventCallback=this;
		GUIObjectRoot->childControls.push_back(obj);
		return;
	}
	
	//Downloaded the addons file now we can create the GUI.
	GUIObjectRoot=new GUIObject(0,0,800,600);
	obj=new GUIObject(90,96,200,32,GUIObjectButton,"Levels");
	obj->name="cmdLvls";
	obj->eventCallback=this;
	GUIObjectRoot->childControls.push_back(obj);
	obj=new GUIObject(300,96,200,32,GUIObjectButton,"Level Packs");
	obj->name="cmdLvlpacks";
	obj->eventCallback=this;
	GUIObjectRoot->childControls.push_back(obj);
	obj=new GUIObject(510,96,200,32,GUIObjectButton,"Themes");
	obj->name="cmdThemes";
	obj->eventCallback=this;
	GUIObjectRoot->childControls.push_back(obj);

	//Create the list for the addons.
	//By default levels will be selected.
	list=new GUIListBox(90,140,620,400);
	list->item=addonsToList("levels");
	list->name="lstAddons";
	list->eventCallback=this;
	GUIObjectRoot->childControls.push_back(list);
	type="levels";
	
	//And the buttons at the bottom of the screen.
	obj=new GUIObject(90,550,200,32,GUIObjectButton,"Back");
	obj->name="cmdBack";
	obj->eventCallback=this;
	GUIObjectRoot->childControls.push_back(obj);
	actionButton=new GUIObject(510,550,200,32,GUIObjectButton,"Install");
	actionButton->name="cmdInstall";
	actionButton->eventCallback=this;
	GUIObjectRoot->childControls.push_back(actionButton);
	updateButton=new GUIObject(300,550,200,32,GUIObjectButton,"Update");
	updateButton->name="cmdUpdate";
	updateButton->enabled=false;
	updateButton->visible=false;
	updateButton->eventCallback=this;
	GUIObjectRoot->childControls.push_back(updateButton);

}

Addons::~Addons(){
	delete addons;
	
	//Free the title surface.
	SDL_FreeSurface(title);
	
	//If the GUIObjectRoot exist delete it.
	if(GUIObjectRoot){
		delete GUIObjectRoot;
		GUIObjectRoot=NULL;
	}
}

bool Addons::getAddonsList(FILE* file){
	//First we download the file.
	if(downloadFile("http://meandmyshadow.sourceforge.net/game/addons03",file)==false){
		error="ERROR: unable to download addons file!";
		cerr<<error<<endl;
		return false;
	}
	fclose(file);
	
	//Load the downloaded file.
	ifstream addonFile;
	addonFile.open((getUserPath(USER_CACHE)+"addons").c_str());
	
	if(!addonFile) {
		error="ERROR: unable to load addon_list file!";
		cerr<<error<<endl;
		return false;
	}
	
	//Parse the addonsfile.
	TreeStorageNode obj;
	{
		POASerializer objSerializer;
		if(!objSerializer.readNode(addonFile,&obj,true)){
			error="ERROR: Invalid file format of addons file!";
			cerr<<error<<endl;
			return false;
		}
	}
	
	//Also load the installed_addons file.
	ifstream iaddonFile;
	iaddonFile.open((getUserPath(USER_CONFIG)+"installed_addons").c_str());
	
	if(!iaddonFile) {
		//The installed_addons file doesn't exist, so we create it.
		ofstream iaddons;
		iaddons.open((getUserPath(USER_CONFIG)+"installed_addons").c_str());
		iaddons<<" "<<endl;
		iaddons.close();
		
		//Also load the installed_addons file.
		iaddonFile.open((getUserPath(USER_CONFIG)+"installed_addons").c_str());
		if(!iaddonFile) {
			error="ERROR: Unable to create the installed_addons file.";
			cerr<<error<<endl;
			return false;
		}
	}
	
	//And parse the installed_addons file.
	TreeStorageNode obj1;
	{
		POASerializer objSerializer;
		if(!objSerializer.readNode(iaddonFile,&obj1,true)){
			error="ERROR: Invalid file format of the installed_addons!";
			cerr<<error<<endl;
			return false;
		}
	}
	
	
	//Fill the vector.
	addons = new std::vector<Addon>;
	fillAddonList(*addons,obj,obj1);
		
	//Close the files.
	iaddonFile.close();
	addonFile.close();
	return true;
}

void Addons::fillAddonList(std::vector<Addons::Addon> &list, TreeStorageNode &addons, TreeStorageNode &installed_addons){
	//Loop through the blocks of the addons file.
	//These should contain the types levels, levelpacks, themes.
	for(unsigned int i=0;i<addons.subNodes.size();i++){
		TreeStorageNode* block=addons.subNodes[i];
		if(block==NULL) continue;
		
		string type;
		type=block->name;
		//Now loop the entries(subNodes) of the block.
		for(unsigned int i=0;i<block->subNodes.size();i++){
			TreeStorageNode* entry=block->subNodes[i];
			if(entry==NULL) continue;
			if(entry->name=="entry" && entry->value.size()==1){
				//The entry is valid so create a new Addon.
				Addon addon = *(new Addon);
				addon.type=type;
				addon.name=entry->value[0];
				addon.file=entry->attributes["file"][0];
				if(!entry->attributes["folder"].empty()){
					addon.folder=entry->attributes["folder"][0];
				}
				addon.author=entry->attributes["author"][0];
				addon.version=atoi(entry->attributes["version"][0].c_str());
				addon.upToDate=false;
				addon.installed=false;
				
				//Check if the addon is already installed.
				for(unsigned int i=0;i<installed_addons.subNodes.size();i++){
					TreeStorageNode* installed=installed_addons.subNodes[i];
					if(installed==NULL) continue;
					if(installed->name=="entry" && installed->value.size()==3){
						if(addon.type.compare(installed->value[0])==0 && addon.name.compare(installed->value[1])==0) {
							addon.installed=true;
							addon.installedVersion=atoi(installed->value[2].c_str());
							if(addon.installedVersion>=addon.version) {
								addon.upToDate=true;
							}

						}
					}
				}
				
				//Finally put him in the list.
				list.push_back(addon);
			}
		}
	}
}

std::vector<std::string> Addons::addonsToList(const std::string &type){
	std::vector<std::string> result;
	
	for(unsigned int i=0;i<addons->size();i++) {
		//Check if the addon is from the right type.
		if((*addons)[i].type==type) {
			string entry = (*addons)[i].name + " by " + (*addons)[i].author;
			if((*addons)[i].installed) {
				if((*addons)[i].upToDate) {
					entry += " *";
				} else {
					entry += " +";
				}
			}
			result.push_back(entry);
		}
	}
	return result;
}

bool Addons::saveInstalledAddons(){
	if(!addons) return false;

	//Open the file.
	ofstream iaddons;
	iaddons.open((getUserPath(USER_CONFIG)+"installed_addons").c_str());
	if(!iaddons) return false;
	
	//Loop all the levels.
	TreeStorageNode installed;
	for(unsigned int i=0;i<addons->size();i++) {
		//Check if the level is installed or not.
		if((*addons)[i].installed) {
			TreeStorageNode *entry=new TreeStorageNode;
			entry->name="entry";
			entry->value.push_back((*addons)[i].type);
			entry->value.push_back((*addons)[i].name);
			char version[64];
			sprintf(version,"%d",(*addons)[i].installedVersion);
			entry->value.push_back(version);
			
			installed.subNodes.push_back(entry);
		}
	}
	
	
	//And write away the file.
	POASerializer objSerializer;
	objSerializer.writeNode(&installed,iaddons,true,true);
	
	return true;
}

void Addons::handleEvents(){
	//Check if we should quit.
	if(event.type==SDL_QUIT){
		//Save the installed addons before exiting.
		saveInstalledAddons();
		setNextState(STATE_EXIT);
	}

	//Check if escape is pressed, if so return to the main menu.
	if(inputMgr.isKeyUpEvent(INPUTMGR_ESCAPE)){
		setNextState(STATE_MENU);
	}
}

void Addons::logic(){}

void Addons::render(){
	//We only need to draw the background.
	applySurface(0,0,background,screen,NULL);
	
	//Draw the title.
	applySurface((800-title->w)/2,16,title,screen,NULL);
	
	//Draw line below selected item
	SDL_Rect r;
	r.y=124;
	r.h=2;
	if(type=="levelpacks"){
		r.x=317;
		r.w=164;
	}else if (type=="themes"){
		r.x=557;
		r.w=106;
	}else{
		r.x=144;
		r.w=89;
	}
	SDL_FillRect(screen,&r,0x000000);
}

void Addons::GUIEventCallback_OnEvent(std::string name,GUIObject* obj,int eventType){
	if(name=="cmdLvlpacks"){
		list->item=addonsToList("levelpacks");
		list->value=0;
		type="levelpacks";
		GUIEventCallback_OnEvent("lstAddons",list,GUIEventChange);
	}else if(name=="cmdLvls"){
		list->item=addonsToList("levels");
		list->value=0;
		type="levels";
		GUIEventCallback_OnEvent("lstAddons",list,GUIEventChange);
	}else if(name=="cmdThemes"){
		list->item=addonsToList("themes");
		list->value=0;
		type="themes";
		GUIEventCallback_OnEvent("lstAddons",list,GUIEventChange);
	}else if(name=="lstAddons"){
		//Get the addon struct that belongs to it.
		Addon *addon=NULL;
		if(list->item.size()>0) {
			string entry = list->item[list->value];
			if(type.compare("levels")==0) {
				for(unsigned int i=0;i<addons->size();i++) {
					std::string prefix=(*addons)[i].name;
					if(!entry.compare(0, prefix.size(), prefix)) {
						addon=&(*addons)[i];
					}
				}
			} else if(type.compare("levelpacks")==0) {
				for(unsigned int i=0;i<addons->size();i++) {
					std::string prefix=(*addons)[i].name;
					if(!entry.compare(0, prefix.size(), prefix)) {
						addon=&(*addons)[i];
					}
				} 
			} else if(type.compare("themes")==0) {
				for(unsigned int i=0;i<addons->size();i++) {
					std::string prefix=(*addons)[i].name;
					if(!entry.compare(0, prefix.size(), prefix)) {
						addon=&(*addons)[i];
					}
				}
			}
		}
		
		selected=addon;
		updateActionButton();
		updateUpdateButton();
	}else if(name=="cmdBack"){
		saveInstalledAddons();
		setNextState(STATE_MENU);
	}else if(name=="cmdUpdate"){

		//First remove the addon and then install it again.
		if(type.compare("levels")==0) {	
			if(downloadFile(selected->file,(getUserPath(USER_DATA)+"/levels/"))!=false){
				selected->upToDate=true;
				selected->installedVersion=selected->version;
				list->item=addonsToList("levels");
				updateActionButton();
				updateUpdateButton();
			}else{
				cerr<<"ERROR: Unable to download addon!"<<endl;
				msgBox("ERROR: Unable to download addon!",MsgBoxOKOnly,"ERROR:");
				return;
			}
		}else if(type.compare("levelpacks")==0) {
			if(!removeDirectory((getUserPath(USER_DATA)+"levelpacks/"+selected->folder+"/").c_str())){
				cerr<<"ERROR: Unable to remove the directory "<<(getUserPath(USER_DATA)+"levelpacks/"+selected->folder+"/")<<"."<<endl;
				return;
			}	
			if(downloadFile(selected->file,(getUserPath(USER_CACHE)+"/tmp/"))!=false){
				extractFile(getUserPath(USER_CACHE)+"/tmp/"+fileNameFromPath(selected->file,true),getUserPath(USER_DATA)+"/levelpacks/"+selected->folder+"/");
				selected->upToDate=true;
				selected->installedVersion=selected->version;
				list->item=addonsToList("levelpacks");
				updateActionButton();
				updateUpdateButton();
			}else{
				cerr<<"ERROR: Unable to download addon!"<<endl;
				msgBox("ERROR: Unable to download addon!",MsgBoxOKOnly,"ERROR:");
				return;
			}
		}else if(type.compare("themes")==0) {
			if(!removeDirectory((getUserPath(USER_DATA)+"themes/"+selected->folder+"/").c_str())){
				cerr<<"ERROR: Unable to remove the directory "<<(getUserPath(USER_DATA)+"themes/"+selected->folder+"/")<<"."<<endl;
				return;
			}		
			if(downloadFile(selected->file,(getUserPath(USER_CACHE)+"/tmp/"))!=false){
				extractFile((getUserPath(USER_CACHE)+"/tmp/"+fileNameFromPath(selected->file,true)),(getUserPath(USER_DATA)+"/themes/"+selected->folder+"/"));
				selected->upToDate=true;
				selected->installedVersion=selected->version;
				list->item=addonsToList("themes");
				updateActionButton();
				updateUpdateButton();
			}else{
				cerr<<"ERROR: Unable to download addon!"<<endl;
				msgBox("ERROR: Unable to download addon!",MsgBoxOKOnly,"ERROR:");
				return;
			}
		}
		    
	
	}else if(name=="cmdInstall"){
		switch(action) {
		  case NONE:
		    break;
		  case INSTALL:
			//Download the addon.
			if(type.compare("levels")==0) {
				if(downloadFile(selected->file,getUserPath(USER_DATA)+"/levels/")!=false){
					selected->upToDate=true;
					selected->installed=true;
					selected->installedVersion=selected->version;
					list->item=addonsToList("levels");
					updateActionButton();
					updateUpdateButton();
				}else{
					cerr<<"ERROR: Unable to download addon!"<<endl;
					msgBox("ERROR: Unable to download addon!",MsgBoxOKOnly,"ERROR:");
					return;
				}
			}else if(type.compare("levelpacks")==0) {
				if(downloadFile(selected->file,getUserPath(USER_CACHE)+"/tmp/")!=false){
					extractFile(getUserPath(USER_CACHE)+"/tmp/"+fileNameFromPath(selected->file,true),getUserPath(USER_DATA)+"/levelpacks/"+selected->folder+"/");
					selected->upToDate=true;
					selected->installed=true;
					selected->installedVersion=selected->version;
					list->item=addonsToList("levelpacks");
					updateActionButton();
					updateUpdateButton();
				}else{
					cerr<<"ERROR: Unable to download addon!"<<endl;
					msgBox("ERROR: Unable to download addon!",MsgBoxOKOnly,"ERROR:");
					return;
				}
			}else if(type.compare("themes")==0) {
				if(downloadFile(selected->file,getUserPath(USER_CACHE)+"/tmp/")!=false){
					extractFile(getUserPath(USER_CACHE)+"/tmp/"+fileNameFromPath(selected->file,true),getUserPath(USER_DATA)+"/themes/"+selected->folder+"/");
					selected->upToDate=true;
					selected->installed=true;
					selected->installedVersion=selected->version;
					list->item=addonsToList("themes");
					updateActionButton();
					updateUpdateButton();
				}else{
					cerr<<"ERROR: Unable to download addon!"<<endl;
					msgBox("ERROR: Unable to download addon!",MsgBoxOKOnly,"ERROR:");
					return;
				}
			}
		    break;
		  case UNINSTALL:
			//Uninstall the addon.
			if(type.compare("levels")==0) {
				if(remove((getUserPath(USER_DATA)+"levels/"+fileNameFromPath(selected->file)).c_str())){
					cerr<<"ERROR: Unable to remove the file "<<(getUserPath(USER_DATA) + "levels/" + fileNameFromPath(selected->file))<<"."<<endl;
					return;
				}
				
				selected->upToDate=false;
				selected->installed=false;
				list->item=addonsToList("levels");
				updateActionButton();
				updateUpdateButton();
			}else if(type.compare("levelpacks")==0) {
				if(!removeDirectory((getUserPath(USER_DATA)+"levelpacks/"+selected->folder+"/").c_str())){
					cerr<<"ERROR: Unable to remove the directory "<<(getUserPath(USER_DATA)+"levelpacks/"+selected->folder+"/")<<"."<<endl;
					return;
				}
				  
				selected->upToDate=false;
				selected->installed=false;
				list->item=addonsToList("levelpacks");
				updateActionButton();
				updateUpdateButton();
			}else if(type.compare("themes")==0) {
				if(!removeDirectory((getUserPath(USER_DATA)+"themes/"+selected->folder+"/").c_str())){
					cerr<<"ERROR: Unable to remove the directory "<<(getUserPath(USER_DATA)+"themes/"+selected->folder+"/")<<"."<<endl;
					return;
				}
				  
				selected->upToDate=false;
				selected->installed=false;
				list->item=addonsToList("themes");
				updateActionButton();
				updateUpdateButton();
			}
		    break;
		}
	}
}

void Addons::updateUpdateButton(){
	//some sanity check
	if(selected==NULL){
		updateButton->enabled=false;
		return;
	}

	//Check if the selected addon is installed.
	if(selected->installed){
		//It is installed, but is it uptodate?
		if(selected->upToDate){
			//The addon is installed and there is no need to show the button.
			updateButton->enabled=false;
			updateButton->visible=false;
		}else{
			//Otherwise show the button
			updateButton->enabled=true;
			updateButton->visible=true;
		}
	}else{
		//The addon isn't installed so we can only install it.
		updateButton->enabled=false;
	}
}



void Addons::updateActionButton(){
	//some sanity check
	if(selected==NULL){
		actionButton->enabled=false;
		action = NONE;
		return;
	}

	//Check if the selected addon is installed.
	if(selected->installed){
		//It is installed, but is it uptodate?
		actionButton->enabled=true;
		actionButton->caption="Uninstall";
		action = UNINSTALL;
	}else{
		//The addon isn't installed so we can only install it.
		actionButton->enabled=true;
		actionButton->caption="Install";
		action = INSTALL;
	}
}
