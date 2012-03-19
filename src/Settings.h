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
#ifndef SETTINGS_H
#define SETTINGS_H

#include <iostream>
#include <string>
#include <sstream>
#include <map>
#include <fstream>
#include <ctime>

//The Settings class is used for loading/saving the config file and keeping track of the settings.
class Settings{
private:
	//The name of the settingsfile.
	const std::string fileName;
	
	//Map containing all the settings.
	//The key is the name of the setting.
	std::map<std::string,std::string> settings;
	
	//Method that will create the config file.
	void createFile();
	//Parse a given line and, if valid, add it to the settings map.
	//line: The line to parse.
	void parseLine(const std::string &line);
	//Validates a given line. (Check if it contains a '=')
	//line: The line to validate.
	//Returns: True if the line is valid.
	bool validLine(const std::string &line);
	//Remove comments from a given line.
	//Comments start with '#' and continues to the end of the line.
	//line: The line to uncomment.
	void unComment(std::string &line);
	//Checks if the line is empty.
	//line: The line to check.
	//Returns: True if the line is emtpy.
	bool empty(const std::string &line);
public:
	//Constructor.
	//fileName: The name of the file to use.
	Settings(std::string fileName);
	
	//This method will open the settings file and walk through it line for line.
	void parseFile();
	//This method will write the settings in the map to the settings file.
	void save();
	
	//Gets the value that belongs to a given key.
	//key: The name of teh setting.
	//Returns: The value if the key exists and an empty string if the key doesn't exist.
	std::string getValue(const std::string &key);
	//Gets the value, as a bool, that belongs to a given key.
	//key: The name of teh setting.
	//Returns: The (bool)value if the key exists and false if the key doesn't exist.
	bool getBoolValue(const std::string &key);
	
	//This will set the value of a given key.
	//Note: If the key doesn't exist it won't be set. //FIXME: If the key doesn't exist why not create it?
	//key: The key of the setting to change.
	//value: The new value of the key.
	void setValue(const std::string &key, const std::string &value);
};

#endif