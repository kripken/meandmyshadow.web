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

#include "ImageManager.h"
#include <stdio.h>

SDL_Surface* ImageManager::loadImage(std::string file){
	//Pointer to an SDL_Surface to which the image will be loaded.
	SDL_Surface* load=NULL;
	//This SDL_Surface is used to set the alpha correct.
	SDL_Surface* opt=NULL;

	//First check if the image isn't already in the imageCollection.
	opt=imageCollection[file];
	if(opt!=NULL)
		return opt;

	//Now load the image to load.
	load=IMG_Load(file.c_str());

	//Check if loading didn't fail.
	if(load!=NULL){
		//Check if there's an alpha mask.
		if(load->format->Amask){
			//There is so let opt use it.
			opt=SDL_DisplayFormatAlpha(load);
			//We don't need load anymore so free it.
			SDL_FreeSurface(load);
		}else{
			//We use cyan for alpha, so set the alpha color key.
			SDL_SetColorKey(load,SDL_SRCCOLORKEY,SDL_MapRGB(load->format,0,0xFF,0xFF));
			//Let opt use the alpha.
			opt=SDL_DisplayFormat(load);
			//We don't need load anymore so free it.
			SDL_FreeSurface(load);
		}
	}else{
		//We couldn't load the image.
		fprintf(stderr,"ERROR: Can't open image file %s\n",file.c_str());
		return NULL;
	}

	//Add a pointer to the imageCollection.
	imageCollection[file]=opt;
	//And return the SDL_Surface*.
	return opt;
}

ImageManager::~ImageManager(){
	//We call destroy().
	destroy();
}

void ImageManager::destroy(){
	//Loop through the imageCollection and free them.
	std::map<std::string,SDL_Surface*>::iterator i;
	for(i=imageCollection.begin();i!=imageCollection.end();i++){
		SDL_FreeSurface(i->second);
	}
	imageCollection.clear();
}
