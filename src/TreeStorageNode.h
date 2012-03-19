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
#ifndef TREESTORAGENODE_H
#define TREESTORAGENODE_H

#include "ITreeStorage.h"
#include <map>
#include <vector>
#include <string>

//This class is used to store data in a tree-structured way.
//Every (TreeStorage)Node has a vector with subNodes and every Node contains a hashmap with attributes.
class TreeStorageNode:public ITreeStorageBuilder,public ITreeStorageReader{
private:
	//Iterator used to iterate the hashmap with attributes.
	//Used by the methods getNextAttribute and getNextNode.
	std::map<std::string,std::vector<std::string> >::iterator objAttrIterator;
public:
	//Vector containing the subnodes of the TreeStorageNode.
	std::vector<TreeStorageNode*> subNodes;
	//String containing the name of the TreeStorageNode.
	std::string name;
	//Vector containing the value(s) of the TreeStorageNode.
	std::vector<std::string> value;
	//Hashmap containing the attributes of the TreeStorageNode.
	std::map<std::string,std::vector<std::string> > attributes;
	
	//Constructor.
	TreeStorageNode(){}
	//Destructor.
	virtual ~TreeStorageNode();
	//This method is used to destroy the TreeStorageNode.
	//Also called when the deconstructor is called.
	void destroy();

	//Set the name of the TreeStorageNode.
	//name: The name to give.
	virtual void setName(std::string& name);
	//Sets the parameter name to the name of the TreeStorageNode.
	//name: The string to fill with the name;
 	virtual void getName(std::string& name);
	
	//Set the value of the TreeStorageNode.
	//value: The value to give.
	virtual void setValue(std::vector<std::string>& value);
	//Sets the parameter value to the value of the TreeStorageNode.
	//value: The string to fill with the name;
	virtual void getValue(std::vector<std::string>& value);
	
	//Creates a new node in the TreeStorageNode.
	//The new node will be added to the subnodes.
	//Returns: a pointer to the new node.
	virtual ITreeStorageBuilder* newNode();
	//Creates a new attribute in the TreeStorageNode.
	//The attribute will be added to the attributes map.
	//name: The name for the new attribute.
	//value: The value for the new attribute.
	virtual void newAttribute(std::string& name,std::vector<std::string>& value);

	
	//Method used for iterating through the attributes of the TreeStorageNode.
	//pUserData: Pointer TODO???
	//name: The string fill with the name of the attribute.
	//value: Vector to fill with the value(s) of the attribute.
	virtual void* getNextAttribute(void* pUserData,std::string& name,std::vector<std::string>& value);
	//Method used for iterating through the subnodes of the TreeStorageNode.
	//pUserData: Pointer TODO???
	//obj: Pointer that will be pointed to the nextNode, if present.
	virtual void* getNextNode(void* pUserData,ITreeStorageReader*& obj);

	//Calculate the MD5 of node based on the data structure.
	//places the message digest in md,
	//which must have space for 16 bytes of output (or NULL).
	unsigned char* calcMD5(unsigned char* md);
};
#endif
