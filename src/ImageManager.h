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
#ifndef IMAGEMANAGER_H
#define IMAGEMANAGER_H

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <string>
#include <map>

//Class for loading images.
class ImageManager{
public:
	//Constructor.
	ImageManager(){}
	//Destructor.
	~ImageManager();
	
	//Loads an image.
	//file: The image file to load.
	//Returns: The SDL_Surface containing the image.
	SDL_Surface* loadImage(std::string file);
	
	//Destroys the images
	void destroy();
private:
	//Map containing the images.
	//The key is the name of the image and the value is a pointer to the SDL_Surface.
	std::map<std::string,SDL_Surface*> imageCollection;
};

#endif
