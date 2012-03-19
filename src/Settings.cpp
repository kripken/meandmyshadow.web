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

#include "Settings.h"
#include <SDL/SDL.h>
#include <string>
#include <stdio.h>
using namespace std;

Settings::Settings(const string fileName): fileName(fileName){
	char s[32];
	settings["sound"]="1";
	settings["music"]="1";
	settings["musiclist"]="default";
	settings["fullscreen"]="0";
	settings["theme"]="Cloudscape";
	settings["leveltheme"]="1";
	settings["internet"]="1";
	settings["lastlevelpack"]="tutorial";
	settings["internet-proxy"]="";

	//key settings
	sprintf(s,"%d",(int)SDLK_UP);
	settings["key_up"]=settings["key_jump"]=s;

	sprintf(s,"%d",(int)SDLK_DOWN);
	settings["key_down"]=settings["key_action"]=s; //SDLK_DOWN

	sprintf(s,"%d",(int)SDLK_LEFT);
	settings["key_left"]=s; //SDLK_LEFT

	sprintf(s,"%d",(int)SDLK_RIGHT);
	settings["key_right"]=s; //SDLK_RIGHT

	sprintf(s,"%d",(int)SDLK_SPACE);
	settings["key_space"]=s; //SDLK_SPACE

	sprintf(s,"%d",(int)SDLK_ESCAPE);
	settings["key_escape"]=s; //SDLK_ESCAPE

	sprintf(s,"%d",(int)SDLK_r);
	settings["key_restart"]=s; //SDLK_r

	sprintf(s,"%d",(int)SDLK_TAB);
	settings["key_tab"]=s;

	sprintf(s,"%d",(int)SDLK_F2);
	settings["key_save"]=s; //SDLK_F2

	sprintf(s,"%d",(int)SDLK_F3);
	settings["key_load"]=s; //SDLK_F3

	sprintf(s,"%d",(int)SDLK_F4);
	settings["key_swap"]=s; //SDLK_F4

	sprintf(s,"%d",(int)SDLK_F5);
	settings["key_teleport"]=s; //SDLK_F5

	sprintf(s,"%d",(int)SDLK_F12);
	settings["key_suicide"]=s;

	sprintf(s,"%d",(int)SDLK_RSHIFT);
	settings["key_shift"]=s; //SDLK_RSHIFT
	
	sprintf(s,"%d",(int)SDLK_PAGEUP);
	settings["key_next"]=s; //SDLK_PAGEUP
	
	sprintf(s,"%d",(int)SDLK_PAGEDOWN);
	settings["key_previous"]=s; //SDLK_PAGEDOWN

	settings["key_up2"]=settings["key_down2"]=settings["key_left2"]=settings["key_right2"]=
		settings["key_jump2"]=settings["key_action2"]=
		settings["key_space2"]=settings["key_escape2"]=settings["key_restart2"]=settings["key_tab2"]=
		settings["key_save2"]=settings["key_load2"]=settings["key_swap2"]=settings["key_teleport2"]=
		settings["key_suicide2"]=settings["key_shift2"]=settings["key_next2"]=settings["key_previous2"]="0";
}


void Settings::parseFile(){
	//We open the settings file.
	ifstream file;

	file.open(fileName.c_str());
	if(!file){
		cout<<"Can't find config file!"<<endl;
		createFile();
	}

	//Now we're going to walk throught the file line by line.
	string line;
	while(getline(file,line)){
		string temp = line;

		unComment(temp);
		if(temp.empty() || empty(temp))
			continue;
		
		//The line is good so we parse it.
		parseLine(temp);
	}

	//And close the file.
	file.close();
}

void Settings::parseLine(const string &line){
	if((line.find('=') == line.npos) || !validLine(line))
		cout<<"Warning illegal line in config file!"<<endl;
	
	string temp = line;
	temp.erase(0, temp.find_first_not_of("\t "));
	int seperator = temp.find('=');

	//Get the key and trim it.
	string key, value;
	key = line.substr(0, seperator);
	if(key.find('\t')!=line.npos || key.find(' ')!=line.npos)
		key.erase(key.find_first_of("\t "));
	
	//Get the value and trim it.
	value = line.substr(seperator + 1);
	value.erase(0, value.find_first_not_of("\t "));
	value.erase(value.find_last_not_of("\t ") + 1);
	
	//Add the setting to the settings map.
	setValue(key,value);
}

bool Settings::validLine(const string &line){
	string temp = line;
	temp.erase(0, temp.find_first_not_of("\t "));
	if(temp[0] == '=')
		return false;

	for(size_t i = temp.find('=') + 1; i < temp.length(); i++)
		return true;
	return false;
}

void Settings::unComment(string &line){
	if (line.find('#') != line.npos)
		line.erase(line.find('#'));
}

bool Settings::empty(const string &line){
	return (line.find_first_not_of(' ')==line.npos);
}

string Settings::getValue(const string &key){
	if(settings.find(key) == settings.end()){
		cout<<"Key "<<key<<" couldn't be found!"<<endl;
		return "";
	}
	return settings[key];
}

bool Settings::getBoolValue(const string &key){
	if(settings.find(key) == settings.end()){
		cout<<"Key "<<key<<" couldn't be found!"<<endl;
		return false;
	}
	return (settings[key] == "1");
}

void Settings::setValue(const string &key, const string &value){
	if(settings.find(key) == settings.end()){
		cout<<"Key "<<key<<" couldn't be found!"<<endl;
		return;
	}
	settings[key]=value;
}

void Settings::createFile(){
	ofstream file;
	file.open(fileName.c_str());
	
	//Default Config file.
	file<<"#MeAndMyShadow config file. Created on "<<endl;
	
	map<string, string>::iterator iter;
	for(iter = settings.begin(); iter != settings.end(); ++iter){
		file << iter->first << " = " << iter->second << endl;
	}
	
	//And close the file.
	file.close();
}

void Settings::save(){
	ofstream file;
	file.open(fileName.c_str());
	
	//First get the date and time.
	time_t rawtime;
	struct tm* timedate;
	
	time(&rawtime);
	timedate=localtime(&rawtime);
	
	//Now write it to the first line of the config file.
	//Note: There's no endl at the end since that's already in asctime(timeinfo).
	file<<"#MeAndMyShadow config file. Created on "<<asctime(timedate);
	
	//Loop through the settings and save them.
	map<string,string>::const_iterator iter;
	for(iter=settings.begin(); iter!=settings.end(); ++iter){
		file<<iter->first<<" = "<<iter->second<<endl;
	}
	file.close();
}
