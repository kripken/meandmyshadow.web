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

#ifndef ITREESERIALIZER_H
#define ITREESERIALIZER_H

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include "ITreeStorage.h"

class ITreeSerializer{
public:
	//Method used to read a node from an inputstream.
	//fin: The inputstream to read from.
	//objOut: The TreeStorageNode in which the result will come.
	//loadSubNodeOnly: Boolean if only the subNodes should be loaded.
	//Returns: False if there's an error while reading the node.
	virtual bool readNode(std::istream& fin,ITreeStorageBuilder* objOut,bool loadSubNodeOnly=false)=0;
	//Method used to write to an outputstream.
	//obj: Pointer to the TreeStorageNode containing the data.
	//fout: The output stream to write to.
	//writeHeader: TODO: ???
	//saveSubNodeOnly: Boolean if only the subNodes should be saved.
	virtual void writeNode(ITreeStorageReader* obj,std::ostream& fout,bool writeHeader=true,bool saveSubNodeOnly=false)=0;
public:
	//Method used to load a node from a file.
	//fileName: The file to load from.
	//objOut: The object to place the result in.
	//loadSubNodeOnly: Boolean if only the subNodes should be loaded.
	//Returns: False if there's an error while reading the node.
	bool loadNodeFromFile(const char* fileName,ITreeStorageBuilder* objOut,bool loadSubNodeOnly=false){
		//Open an inputstream.
		std::ifstream f(fileName);
		//If it failed then we return.
		if(!f)
			return false;
		
		//It didn't fail so let the readNode method handle the rest.
		return readNode(f,objOut,loadSubNodeOnly);
	}
	//Method used to write a node to a file.
	//fileName: The file to save to.
	//obj: Pointer to the TreeStorageNode containing the data.
	//writeHeader: TODO: ???
	//saveSubNodeOnly: Boolean if only the subNodes should be saved.
	bool saveNodeToFile(const char* fileName,ITreeStorageReader* obj,bool writeHeader=true,bool saveSubNodeOnly=false){
		//Open an outputstream.
		std::ofstream f(fileName);
		//If it failed then we return.
		if(!f)
			return false;
		
		//It didn't fail so let the writeNode method handle the rest.
		writeNode(obj,f,writeHeader,saveSubNodeOnly);
		return true;
	}
};

#endif
