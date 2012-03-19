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
#ifndef MUSICMANAGER_H
#define MUSICMANAGER_H

#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#include <string>
#include <map>
#include <vector>

//Class for loading and playing the music.
class MusicManager{
private:  
	//This structure is used to hold music information.
	struct Music{
		//Pointer to the actual music.
		Mix_Music* music;
		//Pointer to the loop music, if any.
		Mix_Music* loop;
		
		//String containing the name of the music.
		//This is the same name as in the musicCollection.
		std::string name;	
		
		//String containing the track name of the music.
		//This is the name given by the author.
		std::string trackName;
		//String containing the name of the author.
		std::string author;
		//String containing the license the music is released under.
		std::string license;
		
		//Integer containing the time where to start playing.
		int start;
		
		//The volume to play the music with.
		//Scale 0-128.
		int volume;
		
		//Integer containing the loopstart.
		int loopStart;
		//Integer containing the loopend.
		//NOTE: loopend doesn't work and is thus ignored.
		int loopEnd;
		
		//Integer used to keep track when which music was played.
		int lastTime;
	};
public:
	//Constructor.
	MusicManager();
	//Destructor.
	~MusicManager();
	
	//Destroys the music.
	void destroy();
	
	//Method that will either disable or enable music.
	//enable: Boolean if the musicManager should be enabled or not.
	void setEnabled(bool enable=true);
	
	//This method will load one music file and add it to the collection.
	//file: The filename of the music file.
	//Returns: String containing the loaded music comma sperated, it's empty if it fails.
	std::string loadMusic(const std::string &file);
	
	//This method will load from a music list.
	//file: The filename of the music list file.
	//Returns: True if no error occurs.
	bool loadMusicList(const std::string &file);
	
	//This method will start playing a music file.
	//name: The name of the song.
	//fade: Boolean if it should fade the current one out or not.
	void playMusic(const std::string &name,bool fade=true);
	
	//This method will pick music from the current music list.
	void pickMusic();
	
	//Method that will be called when a music stopped.
	void musicStopped();
	
	//Set the music list.
	//list: The name of the list.
	void setMusicList(const std::string &list);
private:
	//Boolean if the MusicManager is enabled or not.
	//The default value is false meaning that the MusicManager has to be enabled before the music starts.
	bool enabled;
	
	//Integer that is used to keep track of the last played song.
	int lastTime;
	
	//Pointer to the music struct that is currently playing.
	Music* playing;
	
	//String containing the name of the music to play when the previous one stopped.
	//This means that it will be checked in the musicStopped method.
	std::string nextMusic;
	
	//String containing the name of the current music list.
	std::string currentList;
	
	//Map containing the music.
	//The key is the name of the music and the value is a pointer to the Mix_Music.
	std::map<std::string,Music*> musicCollection;
	
	//Map containing the music lists.
	//The key is the name of the list and the value is an array of music names.
	std::map<std::string,std::vector<std::string> > musicLists;
};

#endif
