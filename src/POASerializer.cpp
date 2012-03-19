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

#include "POASerializer.h"
#include <sstream>
using namespace std;

static void readString(std::istream& fin,std::string& string){
	//This method is used for reading a string from an input stream.
	//fin: The input stream to read from.
	//string: String to place the result in.
	
	//The current character.
	int c;
	c=fin.get();
	
	//Check if there's a '"'.
	if(c=='\"'){
		//There's a '"' so place every character we encounter in the string without parsing.
		while(!fin.eof() & !fin.fail()){
			//First we get the next character to prevent putting the '"' in the string.
			c=fin.get();
			
			//Check if there's a '"' since that could mean the end of the string.
			if(c=='\"'){
				//Get the next character and check if that's also an '"'.
				c=fin.get();
				if(c!='\"'){
					//We have two '"' after each other meaning an escaped '"'.
					//We unget one so there will be one '"' placed in the string.
					fin.unget();
					return;
				}
			}
			
			//Every other character can be put in the string.
			string.push_back(c);
		}
	}else{
		//There are no quotes around the string so we need to be carefull detecting if the string has ended.
		do{
			switch(c){
			//Check for characters that mean the end of the string.
			case EOF:
			case ' ':
			case '\r':
			case '\n':
			case '\t':
				return;
			//Check for characters that are part of the POA file format.
			//If so we first unget one character to prevent problems parsing the rest of the file.
			case ',':
			case '=':
			case '(':
			case ')':
			case '{':
			case '}':
			case '#':
				fin.unget();
				return;
			default:
				//In any other case the character is normal so we put it in the string.
				string.push_back(c);
			}
			
			//Get the next character.
			c=fin.get();
		}while(!fin.eof() & !fin.fail());
	}
}

static void skipWhitespaces(std::istream& fin){
	//This function will read from the input stream until there's something else than whitespaces.
	//fin: The input stream to read from.
	
	//The current character.
	int c;
	while(!fin.eof() & !fin.fail()){
		//Get the character.
		c=fin.get();
		
		//Check if it's one of the whitespace characters.
		switch(c){
		case EOF:
		case ' ':
		case '\r':
		case '\n':
		case '\t':
			break;
		default:
			//Anything other means that the whitespaces have ended.
			//Unget the last character and return.
			fin.unget();
			return;
		}
	}
}

static void skipComment(std::istream& fin){
	//This function will read from the input stream until the end of a line (also end of the comment).
	//fin: The input stream to read from.
	
	//The current character.
	int c;
	while(!fin.eof() & !fin.fail()){
		//Get the character.
		c=fin.get();
		
		//Check if it's a new line (end of comment).
		if(c=='\r'||c=='\n'){
			fin.unget();
			break;
		}
	}
}

bool POASerializer::readNode(std::istream& fin,ITreeStorageBuilder* objOut,bool loadSubNodeOnly){
	//The current character.
	int c;
	//The current mode of reading.
	//0=read name
	//1=read attribute value
	//2=read subnode value
	//16=add attribute
	//17=add subnode
	int mode;

	//Before reading make sure that the input stream isn't null.
	if(!fin) return false;
	
	//Vector containing the stack of TreeStorageNodes.
	vector<ITreeStorageBuilder*> stack;
	//A vector for the names and a vector for the values.
	vector<string> names,values;

	//Check if we only need to load subNodes.
	//If so then put the objOut as the first TreeStorageNode.
	if(loadSubNodeOnly) stack.push_back(objOut);

	//Loop through the files.
	while(!fin.eof() && !fin.fail()){
		//Get a character.
		c=fin.get();
		
		//Check what it is and what to do with that character.
		switch(c){
		case EOF:
		case ' ':
		case '\r':
		case '\n':
		case '\t':
			//We skip whitespaces.
			break;
		case '#':
			//A comment so skip it.
			skipComment(fin);
			break;
		case '}':
			//A closing bracket so do one step back in the stack.
			//There must be a TreeStorageNode left if not return false.
			if(stack.size()==0) return false;
			
			//Remove the last entry of the stack.
			stack.pop_back();
			//Check if the stack is empty, if so than the reading of the node is done.
			if(stack.size()==0) return true;
			objOut=stack.back();
			break;
		default:
			//It isn't a special character but part of a name/value, so unget it.
			fin.unget();
			
			{
				//Clear the names and values vectors, start reading new names/values.
				names.clear();
				values.clear();
				
				//Set the mode to the read name mode.
				mode=0;
				
				//Keep reading characters, until we break out the while loop or there's an error.
				while(!fin.eof() & !fin.fail()){
					//The string containing the name.
					string s;
					
					//First skip the whiteSpaces.
					skipWhitespaces(fin);
					//Now get the string.
					readString(fin,s);
					
					//Check the mode.
					switch(mode){
					case 0:
						//Mode is 0(read names) so put the string in the names vector.
						names.push_back(s);
						break;
					case 1:
					case 2:
						//Mode is 1 or 2 so put the string in the values vector.
						values.push_back(s);
						break;
					}
					//Again skip whitespaces.
					skipWhitespaces(fin);
					
					//Now read the next character.
					c=fin.get();
					switch(c){
					case ',':
						//A comma means one more name or value.
						break;
					case '=':
						//An '=' can only occur after a name (mode=0).
						if(mode==0){
	  						//The next string will be a value so set mode to 1.
							mode=1;
						}else{
							//In any other case there's something wrong so return false.
							return false;
						}
						break;
					case '(':
						//An '(' can only occur after a name (mode=0).
						if(mode==0){
							//The next string will be a value of a block so set mode to 2.
							mode=2;
						}else{
							//In any other case there's something wrong so return false.
							return false;
						}
						break;
					case ')':
						//A ')' can only occur after an attribute (mode=2).
						if(mode==2){
							//The next will be a new subNode so set mode to 17.
							mode=17;
						}else{
							//In any other case there's something wrong so return false.
							return false;
						}
						break;
					case '{':
						//A '{' can only mean a new subNode (mode=17).
						fin.unget();
						mode=17;
						break;
					default:
						//The character is not special so unget it.
						fin.unget();
						mode=16;
						break;
					}
					
					//We only need to break out if the mode is 16(add attribute) or 17(add subnode)
					if(mode>=16) break;
				}
				
				//Check the mode.
				switch(mode){
				case 16:
					//The mode is 16 so we need to change the names and values into attributes.
					//The stack mustn't be empty.
					if(stack.size()==0) return false;
					
					//Make sure that the result TreeStorageNode isn't null.
					if(objOut!=NULL){
						//Check if the names vector is empty, if so add an empty name.
						if(names.size()==0) names.push_back("");
						
						//Put an empty value for every valueless name.
						while(values.size()<names.size()) values.push_back("");
						
						//Now loop through the names.
						for(unsigned int i=0;i<names.size()-1;i++){
							//Temp vector that will contain the values.
							vector<string> v;
							v.push_back(values[i]);
							
							//And add the attribute.
							objOut->newAttribute(names[i],v);
						}
						
						if(names.size()>1) values.erase(values.begin(),values.begin()+(names.size()-1));
						objOut->newAttribute(names.back(),values);
					}
					break;
				case 17:
					//The mode is 17 so we need to add a subNode.
					{
						//Check if the names vector is empty, if so add an empty name.
						if(names.size()==0) names.push_back("");
						else if(names.size()>1){
							if(stack.size()==0) return false;
							while(values.size()<names.size()) values.push_back("");
							for(unsigned int i=0;i<names.size()-1;i++){
								vector<string> v;
								v.push_back(values[i]);
								objOut->newAttribute(names[i],v);
							}
							values.erase(values.begin(),values.begin()+(names.size()-1));
						}
						
						//Create a new subNode.
						ITreeStorageBuilder* objNew=NULL;
						
						//If the stack is empty the new subNode will be the result TreeStorageNode.
						if(stack.size()==0) objNew=objOut;
						//If not the new subNode will be a subNode of the result TreeStorageNode.
						else if(objOut!=NULL) objNew=objOut->newNode();
						
						//Add it to the stack.
						stack.push_back(objNew);
						if(objNew!=NULL){
							//Add the name and the values.
							objNew->setName(names.back());
							objNew->setValue(values);
						}
						objOut=objNew;

						//Skip the whitespaces.
						skipWhitespaces(fin);
						//And get the next character.
						c=fin.get();
						if(c!='{'){
							//The character isn't a '{' meaning the block hasn't got a body.
							fin.unget();
							stack.pop_back();
							
							//Check if perhaps we're done, stack=empty.
							if(stack.size()==0) return true;
							objOut=stack.back();
						}
					}
					break;
				default:
					//The mode isn't 16 or 17 but still broke out the while loop.
					//Something's wrong so return false.
					return false;
				}
			}
			break;
		}
	}
	return true;
}

static void writeString(std::ostream& fout,std::string& s){
	//This method will write a string.
	//fout: The output stream to write to.
	//s: The string to write.
	
	//The current character.
	int c;
	
	//new: check if the string is empty
	if(s.empty()){
		//because of the new changes of loader, we should output 2 quotes '""'
		fout<<"\"\"";
	}else
	//Check if the string contains any special character that needs escaping.
	if(s.find_first_of(" \r\n\t,=(){}#\"")!=string::npos){
		//It does so we put '"' around them.
		fout<<'\"';
		
		//Loop through the characters.
		for(unsigned int i=0;i<s.size();i++){
			c=s[i];
			
			//If there's a '"' character it needs to be counter escaped. ("")
			if(c=='\"'){
				fout<<"\"\"";
			}else{
				//If it isn't we can just write away the character.
				fout<<(char)c;
			}
		}
		fout<<'\"';
	}else{
		//It doesn't contain any special characters so we can write it away.
		fout<<s;
	}
}

static void writeStringArray(std::ostream& fout,std::vector<std::string>& s){
	//This method will write a away an array of strings.
	//fout: The output stream to write to.
	//s: Vector containing the strings to write.
	
	//Loop the strings.
	for(unsigned int i=0;i<s.size();i++){
		//If it's the second or more there must be a ",".
		if(i>0) fout<<',';
		//Now write the string.
		writeString(fout,s[i]);
	}
}

static void pWriteNode(ITreeStorageReader* obj,std::ostream& fout,int indent,bool saveSubNodeOnly){
	//Write the TreeStorageNode to the given output stream.
	//obj: The TreeStorageNode to write away.
	//fout: The output stream to write to.
	//indent: Integer containing the number of indentations are needed.
	//saveSubNodeOnly: Boolean if only the subNodes need to be saved.
  
	//Boolean if the node has subNodes.
	bool haveSubNodes=false;
	void* lpUserData=NULL;
	ITreeStorageReader* objSubNode=NULL;
	string s;
	vector<string> v;
	//---
	if(obj==NULL) return;
	//---
	if(!saveSubNodeOnly){
		for(int i=0;i<indent;i++) fout<<'\t';
		s.clear();
		obj->getName(s);
		writeString(fout,s);
		fout<<'(';
		v.clear();
		obj->getValue(v);
		writeStringArray(fout,v);
		fout<<')';
		indent++;
	}
	//attributes
	lpUserData=NULL;
	for(;;){
		s.clear();
		v.clear();
		lpUserData=obj->getNextAttribute(lpUserData,s,v);
		if(lpUserData==NULL) break;
		if(!haveSubNodes && !saveSubNodeOnly) fout<<"{\n";
		haveSubNodes=true;
		for(int i=0;i<indent;i++) fout<<'\t';
		writeString(fout,s);
		fout<<'=';
		writeStringArray(fout,v);
		fout<<'\n';
	}
	//subnodes
	lpUserData=NULL;
	for(;;){
		lpUserData=obj->getNextNode(lpUserData,objSubNode);
		if(lpUserData==NULL) break;
		if(objSubNode!=NULL){
			if(!haveSubNodes && !saveSubNodeOnly) fout<<"{\n";
			haveSubNodes=true;
			pWriteNode(objSubNode,fout,indent,false);
		}
	}
	//---
	if(!saveSubNodeOnly){
		indent--;
		if(haveSubNodes){
			for(int i=0;i<indent;i++) fout<<'\t';
			fout<<'}';
		}
		fout<<'\n';
	}
}

void POASerializer::writeNode(ITreeStorageReader* obj,std::ostream& fout,bool writeHeader,bool saveSubNodeOnly){
	//Make sure that the output stream isn't null.
	if(!fout) return;
	
	//It isn't so start writing the node.
	pWriteNode(obj,fout,0,saveSubNodeOnly);
}
