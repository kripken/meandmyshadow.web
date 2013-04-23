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

#include "TreeStorageNode.h"
#include "MD5.h"
#include <string.h>
using namespace std;

TreeStorageNode::~TreeStorageNode(){
	//The deconstructor will just calls destroy().
	destroy();
}

void TreeStorageNode::destroy(){
	//Loop through the subnodes and delete them.
	for(unsigned int i=0;i<subNodes.size();i++){
		delete subNodes[i];
	}
	
	//Now clear some stuff.
	name.clear();
	value.clear();
	attributes.clear();
	subNodes.clear();
}

void TreeStorageNode::setName(std::string& name){
	this->name=name;
}
void TreeStorageNode::getName(std::string& name){
	name=this->name;
}

void TreeStorageNode::setValue(std::vector<std::string>& value){
	this->value=value;
}
void TreeStorageNode::getValue(std::vector<std::string>& value){
	value=this->value;
}

ITreeStorageBuilder* TreeStorageNode::newNode(){
	TreeStorageNode* obj=new TreeStorageNode;
	subNodes.push_back(obj);
	return obj;
}

void TreeStorageNode::newAttribute(std::string& name,std::vector<std::string>& value){
	//Put the attribute in the attributes map.
	attributes[name]=value;
}

void* TreeStorageNode::getNextAttribute(void* pUserData,std::string& name,std::vector<std::string>& value){
	if(pUserData==NULL) objAttrIterator=attributes.begin();
	if(objAttrIterator!=attributes.end()){
		name=objAttrIterator->first;
		value=objAttrIterator->second;
		objAttrIterator++;
		return &objAttrIterator;
	}else{
		return NULL;
	}
}

void* TreeStorageNode::getNextNode(void* pUserData,ITreeStorageReader*& obj){
	intptr_t i=(intptr_t)pUserData;
	
	//Check if the pointer is in range of the subNodes vector.
	if(i<(int)subNodes.size()){
		obj=subNodes[i];
		return (void*)(i+1);
	}else{
		return NULL;
	}
}

static void md5AppendString(Md5& md5,const string& s){
	unsigned int sz=s.size();
	unsigned char c[4];
	c[0]=sz;
	c[1]=sz>>8;
	c[2]=sz>>16;
	c[3]=sz>>24;
	md5.update(c,4);

	if(sz>0) md5.update(s.c_str(),sz);
}

static void md5AppendVector(Md5& md5,const vector<string>& v){
	unsigned int sz=v.size();
	unsigned char c[4];
	c[0]=sz;
	c[1]=sz>>8;
	c[2]=sz>>16;
	c[3]=sz>>24;
	md5.update(c,4);

	for(unsigned int i=0;i<sz;i++){
		md5AppendString(md5,v[i]);
	}
}

static void md5AppendMap(Md5& md5,const map<string,vector<string> >& m){
	unsigned int sz=m.size();
	unsigned char c[4];
	c[0]=sz;
	c[1]=sz>>8;
	c[2]=sz>>16;
	c[3]=sz>>24;
	md5.update(c,4);

	for(map<string,vector<string> >::const_iterator it=m.begin();it!=m.end();it++){
		md5AppendString(md5,it->first);
		md5AppendVector(md5,it->second);
	}
}

unsigned char* TreeStorageNode::calcMD5(unsigned char* md){
#if EMSCRIPTEN
  memset(md, 0, 16);
  return md;
#else
	unsigned char digest[16];
	Md5 md5;

	md5.init();
	md5AppendString(md5,name);
	md5AppendVector(md5,value);
	md5AppendMap(md5,attributes);
	for(unsigned int i=0;i<subNodes.size();i++){
		TreeStorageNode *node=subNodes[i];
		if(node==NULL){
			memset(digest,0,16);
		}else{
			node->calcMD5(digest);
		}
		md5.update(digest,16);
	}

	return md5.final(md);
#endif
}
