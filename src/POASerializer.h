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

#ifndef POASERIALIZER_H
#define POASERIALIZER_H

#include "ITreeSerializer.h"

//This class is used for reading/writing of TreeStorageNodes to/from POAformat-files.
class POASerializer:public ITreeSerializer{
public:
	//This method will read the inputstream and parses it.
	//The result will be placed in a TreeStorageNode.
	//fin: The input stream which will be read from.
	//objOut: The TreeStorageNode the result will be put in.
	//loadSubNodesOnly: Boolean if only the subNodes need to be loaded.
	//Returns: true if the reading and parsing succeeded.
	virtual bool readNode(std::istream& fin,ITreeStorageBuilder* objOut,bool loadSubNodesOnly=false);
	
	//This method will write a TreeStorageNode to a outputstream.
	//obj: The TreeStorageNode to save.
	//fout: The output stream which will be writen to.
	//saveSubNodesOnly: Boolean if only the subNodes need to be saved.
	//Returns: true if the writing succeeded.
	virtual void writeNode(ITreeStorageReader* obj,std::ostream& fout,bool bWriteHeader=true,bool saveSubNodeOnly=false);
};
#endif
