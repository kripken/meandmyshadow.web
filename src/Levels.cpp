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
#include "Levels.h"
#include "Functions.h"
#include "FileManager.h"
#include "TreeStorageNode.h"
#include "POASerializer.h"
#include "MD5.h"
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
using namespace std;

void Levels::clear(){
	currentLevel=0;
	loaded=false;
	levels.clear();
	levelpackDescription.clear();
	levelpackPath.clear();
	levelProgressFile.clear();
	congratulationText.clear();
}

bool Levels::loadLevels(const std::string& levelListFile){
	//We're going to load a new levellist so first clean any existing levels.
	clear();

	//If the levelListFile is empty we have nothing to load so we return false.
	if(levelListFile.empty()){
		cerr<<"ERROR: No levellist file given."<<endl;
		return false;
	}
	
	//Process the levelListFile, create a new string since lecelListFile is constant.
	string levelListNew=levelListFile;
	levelpackPath=pathFromFileName(levelListNew);

	//Create two input streams, one for the levellist file and one for the levelprogress.
	ifstream level(levelListNew.c_str());
	
	if(!level){
		cerr<<"ERROR: Can't load level list "<<levelListNew<<endl;
		return false;
	}
	
	//Load the level list file.
	TreeStorageNode obj;
	{
		POASerializer objSerializer;
		if(!objSerializer.readNode(level,&obj,true)){
			cerr<<"ERROR: Invalid file format of level list "<<levelListNew<<endl;
			return false;
		}
	}
	
	//Look for the description.
	{
		vector<string> &v=obj.attributes["description"];
		if(v.size()>0)
			levelpackDescription=v[0];
	}
	
	//Look for the congratulation text.
	{
		vector<string> &v=obj.attributes["congratulations"];
		if(v.size()>0)
			congratulationText=v[0];
	}
	
	//Loop through the level list entries.
	for(unsigned int i=0;i<obj.subNodes.size();i++){
		TreeStorageNode* obj1=obj.subNodes[i];
		if(obj1==NULL)
			continue;
		if(obj1->value.size()>=1 && obj1->name=="levelfile"){
			Level level;
			level.file=obj1->value[0];
			level.targetTime=0;
			level.targetRecordings=0;
			
			//Open the level file to retrieve the name and target time/recordings.
			TreeStorageNode obj;
			POASerializer objSerializer;
			if(objSerializer.loadNodeFromFile((levelpackPath+level.file).c_str(),&obj,true)){
				//Calc the MD5 FIRST because query obj.attributes will modify internal structure.
				obj.name.clear();
				obj.calcMD5(level.md5Digest);

				//Get the name of the level.
				vector<string>& v=obj.attributes["name"];
				if(v.size()>0)
					level.name=v[0];
				//If the name is empty then we set it to the file name.
				if(level.name.empty())
					level.name=fileNameFromPath(level.file);
				
				//Get the target time of the level.
				v=obj.attributes["time"];
				if(v.size()>0)
					level.targetTime=atoi(v[0].c_str());
				else
					level.targetTime=-1;
				//Get the target recordings of the level.
				v=obj.attributes["recordings"];
				if(v.size()>0)
					level.targetRecordings=atoi(v[0].c_str());
				else
					level.targetRecordings=-1;
			}
			
			//The default for locked is true, unless it's the first one.
			level.locked=!levels.empty();
			level.won=false;
			level.time=-1;
			level.recordings=-1;
			
			//Add the level to the levels.
			levels.push_back(level);
		}
	}
	
	loaded=true;
	return true;
}

void Levels::loadProgress(const std::string& levelProgressFile){
	//Open the levelProgress file.
	ifstream levelProgress;
	if(!levelProgressFile.empty()){
		this->levelProgressFile=levelProgressFile;
		levelProgress.open(processFileName(this->levelProgressFile).c_str());
	}
	
	//Check if the file exists.
	if(levelProgress){
		//Now load the progress/statistics.
		TreeStorageNode obj;
		{
			POASerializer objSerializer;
			if(!objSerializer.readNode(levelProgress,&obj,true)){
				cerr<<"ERROR: Invalid file format of level progress file."<<endl;
			}
		}
		
		//Loop through the entries.
		for(unsigned int i=0;i<obj.subNodes.size();i++){
			TreeStorageNode* obj1=obj.subNodes[i];
			if(obj1==NULL)
				continue;
			if(obj1->value.size()>=1 && obj1->name=="level"){
				//We've found an entry for a level, now search the correct level.
				Level* level=NULL;
				for(unsigned int o=0;o<levels.size();o++){
					if(obj1->value[0]==levels[o].file){
						level=&levels[o];
						break;
					}
				}
				
				//Check if we found the level.
				if(!level)
					continue;
				
				//Get the progress/statistics.
				for(map<string,vector<string> >::iterator i=obj1->attributes.begin();i!=obj1->attributes.end();i++){
					if(i->first=="locked"){
					level->locked=(i->second[0]=="1");
					}
					if(i->first=="won"){
						level->won=(i->second[0]=="1");
					}
					if(i->first=="time"){
						level->time=(atoi(i->second[0].c_str()));
					}
					if(i->first=="recordings"){
						level->recordings=(atoi(i->second[0].c_str()));
					}
				}
			}
		}
	}
}

void Levels::saveLevels(const std::string& levelListFile){
	//Get the fileName.
	string levelListNew=processFileName(levelListFile);
	//Open an output stream.
	ofstream level(levelListNew.c_str());

	//Check if we can use the file.
	if(!level){
		cerr<<"ERROR: Can't save level list "<<levelListNew<<endl;
		return;
	}
	
	//Storage node that will contain the data that should be written.
	TreeStorageNode obj;

	//Make sure that there's a description.
	if(!levelpackDescription.empty())
		obj.attributes["description"].push_back(levelpackDescription);
	
	//Make sure that there's a congratulation text.
	if(!congratulationText.empty())
		obj.attributes["congratulations"].push_back(congratulationText);

	//Add the levels to the file.
	for(unsigned int i=0;i<levels.size();i++){
		TreeStorageNode* obj1=new TreeStorageNode;
		obj1->name="levelfile";
		obj1->value.push_back(fileNameFromPath(levels[i].file));
		obj1->value.push_back(levels[i].name);
		obj.subNodes.push_back(obj1);
		
		//We copy them to the levelpack folder
		//Check if the levelpath is relative or absolute.
		if(levels[i].file[0]=='%'){
			copyFile(processFileName(levels[i].file).c_str(),(pathFromFileName(levelListNew)+fileNameFromPath(levels[i].file)).c_str());
		}else{
			//Make sure we aren't copying to the same location.
			if((levelpackPath+levels[i].file)!=(pathFromFileName(levelListNew)+fileNameFromPath(levels[i].file))){
				copyFile((levelpackPath+levels[i].file).c_str(),(pathFromFileName(levelListNew)+fileNameFromPath(levels[i].file)).c_str());
			}
		}
	}

	//Write the it away.
	POASerializer objSerializer;
	objSerializer.writeNode(&obj,level,false,true);
}

void Levels::addLevel(const string& levelFileName,int levelno){
	//Fill in the details.
	Level level;
	if(!levelpackPath.empty() && levelFileName.compare(0,levelpackPath.length(),levelpackPath)==0){
		level.file=fileNameFromPath(levelFileName);
	}else{
		level.file=levelFileName;
	}
	level.targetTime=0;
	level.targetRecordings=0;
	
	//Get the name of the level.
	TreeStorageNode obj;
	POASerializer objSerializer;
	if(objSerializer.loadNodeFromFile(levelFileName.c_str(),&obj,true)){
		//Calc the MD5 FIRST because query obj.attributes will modify internal structure.
		obj.name.clear();
		obj.calcMD5(level.md5Digest);

		//Get the name of the level.
		vector<string>& v=obj.attributes["name"];
		if(v.size()>0)
			level.name=v[0];
		//If the name is empty then we set it to the file name.
		if(level.name.empty())
			level.name=fileNameFromPath(levelFileName);
		
		//Get the target time of the level.
		v=obj.attributes["time"];
		if(v.size()>0)
			level.targetTime=atoi(v[0].c_str());
		else
			level.targetTime=-1;
		//Get the target recordings of the level.
		v=obj.attributes["recordings"];
		if(v.size()>0)
			level.targetRecordings=atoi(v[0].c_str());
		else
			level.targetRecordings=-1;
	}
	//Set if it should be locked or not.
	level.won=false;
	level.time=-1;
	level.recordings=-1;
	level.locked=levels.size()>0?true:false;
	
	//Check if the level should be at the end or somewhere in the middle.
	if(levelno<0 || levelno>=int(levels.size())){
		levels.push_back(level);
	}else{
		levels.insert(levels.begin()+levelno,level);
	}
	
	//NOTE: We set loaded to true.
	loaded=true;
}

void Levels::moveLevel(unsigned int level1,unsigned int level2){
	if(level1<0 || level1>=levels.size())
		return;
	if(level2<0 || level2>=levels.size())
		return;
	if(level1==level2)
		return;
	
	levels.insert(levels.begin()+level2,levels[level1]);
	if(level2<=level1)
		levels.erase(levels.begin()+level1+1);
	else
		levels.erase(levels.begin()+level1);
}

void Levels::saveLevelProgress(){
	//Check if the levels are loaded and a progress file is given.
	if(!loaded || levelProgressFile.empty())
		return;
	
	//Open the progress file.
	ofstream levelProgress(processFileName(levelProgressFile).c_str());
	if(!levelProgress)
		return;
	
	//Open an output stream.
	TreeStorageNode node;
	
	//Loop through the levels.
	for(unsigned int o=0;o<levels.size();o++){
		TreeStorageNode* obj=new TreeStorageNode;
		node.subNodes.push_back(obj);
		
		char s[64];
		
		//Set the name of the node.
		obj->name="level";
		obj->value.push_back(levels[o].file);
		
		//Set the values.
		obj->attributes["locked"].push_back(levels[o].locked?"1":"0");
		obj->attributes["won"].push_back(levels[o].won?"1":"0");
		sprintf(s,"%d",levels[o].time);
		obj->attributes["time"].push_back(s);
		sprintf(s,"%d",levels[o].recordings);
		obj->attributes["recordings"].push_back(s);
	}
	
	
	//Create a POASerializer and write away the leve node.
	POASerializer objSerializer;
	objSerializer.writeNode(&node,levelProgress,true,true);
}

const string& Levels::getLevelName(int level){
	if(level<0)
		level=currentLevel;
	return levels[level].name;
}

const unsigned char* Levels::getLevelMD5(int level){
	if(level<0)
		level=currentLevel;
	return levels[level].md5Digest;
}

void Levels::getLevelAutoSaveRecordPath(int level,std::string &bestTimeFilePath,std::string &bestRecordingFilePath,bool createPath){
	if(level<0)
		level=currentLevel;

	bestTimeFilePath.clear();
	bestRecordingFilePath.clear();

	//get level pack path.
	string levelpackPath=Levels::levelpackPath;
	string s=levels[level].file;

	//process level pack name
	for(;;){
		string::size_type lps=levelpackPath.find_last_of("/\\");
		if(lps==string::npos){
			break;
		}else if(lps==levelpackPath.size()-1){
			levelpackPath.resize(lps);
		}else{
			levelpackPath=levelpackPath.substr(lps+1);
			break;
		}
	}

	//profess file name
	{
		string::size_type lps=s.find_last_of("/\\");
		if(lps!=string::npos) s=s.substr(lps+1);
	}

	//check if it's custom level
	{
		string path="%USER%/records/autosave/";
		if(!levelpackPath.empty()){
			path+=levelpackPath;
			path+='/';
		}
		path=processFileName(path);
		if(createPath) createDirectory(path.c_str());
		s=path+s;
	}

	//calculate MD5
	s+='-';
#if EMSCRIPTEN
  s+="nomd5";
#else
	s+=Md5::toString(levels[level].md5Digest);
#endif

	//over
	bestTimeFilePath=s+"-best-time.mnmsrec";
	bestRecordingFilePath=s+"-best-recordings.mnmsrec";
}

void Levels::setLevelName(unsigned int level,const std::string& name){
	if(level<levels.size()) 
		levels[level].name=name;
}

const string& Levels::getLevelFile(int level){
	if(level<0)
		level=currentLevel;
	return levels[level].file;
}

const string& Levels::getLevelpackPath(){
	return levelpackPath;
}

struct Levels::Level* Levels::getLevel(int level){
	if(level<0)
		return &levels[currentLevel];
	return &levels[level];
}

void Levels::resetLevel(int level){
	if(level<0)
		level=currentLevel;
	
	//Set back to default.
	levels[level].locked=(level!=0);
	levels[level].won=false;
	levels[level].time=-1;
	levels[level].recordings=-1;
}

void Levels::nextLevel(){
	currentLevel++;
}

bool Levels::getLocked(unsigned int level){
	return levels[level].locked;
}

void Levels::setCurrentLevel(unsigned int level){
	currentLevel=level;
}

void Levels::setLocked(unsigned int level,bool locked){
	levels[level].locked=locked;
}

void Levels::swapLevel(unsigned int level1,unsigned int level2){
	if(level1<levels.size()&&level2<levels.size()){
		swap(levels[level1],levels[level2]);
	}
}

void Levels::removeLevel(unsigned int level){
	if(level<levels.size()){
		levels.erase(levels.begin()+level);
	}
}
