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

#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include "Globals.h"
#include "FileManager.h"
#include "Functions.h"
//#include <archive.h>
//#include <archive_entry.h>
using namespace std;

#ifdef WIN32
#include <windows.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <direct.h>
#pragma comment(lib,"shlwapi.lib")
#else
#include <strings.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#endif

//Under Windows there's just one userpath.
#ifdef WIN32
string userPath,dataPath,appPath,exeName;
#else
//But on other platforms we make a difference between the userPath (config files) and the userDataPath (data files).
//Finally there's the path for cache data userCachePath.
string userPath,userDataPath,userCachePath,dataPath,appPath,exeName;
#endif

bool configurePaths() {
	//Get the appPath and the exeName.
	{
		char s[4096];
		int i,m;
		#ifdef WIN32
		m=GetModuleFileNameA(NULL,s,sizeof(s));
		#else
		m=readlink("/proc/self/exe",s,sizeof(s));
		#endif
		s[m]=0;
		for(i=m-1;i>=0;i--){
			if(s[i]=='/'||s[i]=='\\'){
				s[i]=0;
				break;
			}
		}
		appPath=s;
		exeName=s+i+1;
	}
	
	//TODO: Check if the userpath is empty before setting userPath???
	//Check if the userPath is empty.
	if(getUserPath().empty()){
#ifdef WIN32
		//Get the userPath.
		char s[1024];
		SHGetSpecialFolderPathA(NULL,s,CSIDL_PERSONAL,1);
		userPath=s;
		userPath+="\\My Games\\meandmyshadow\\";		
#else
		//Temp variable that is used to prevent NULL assignement.
		char* env;
		
		//First get the $XDG_CONFIG_HOME env var.
		env=getenv("XDG_CONFIG_HOME");
		//If it's null set userPath to $HOME/.config/.
		if(env!=NULL){
			userPath=env;
		}else{
			userPath=getenv("HOME");
			userPath+="/.config";
		}
		//And add meandmyshadow to it.
		userPath+="/meandmyshadow/";
		
		//Now get the $XDG_DATA_HOME env var.
		env=getenv("XDG_DATA_HOME");
		//If it's null set userDataPath to $HOME/.local/share.
		if(env!=NULL){
			userDataPath=env;
		}else{
			userDataPath=getenv("HOME");
			userDataPath+="/.local/share";
		}
		//And add meandmyshadow to it.
		userDataPath+="/meandmyshadow/";
		
		//Now get the $XDG_CACHE_HOME env var.
		env=getenv("XDG_CACHE_HOME");
		//If it's null set userCachePath to $HOME/.cache.
		if(env!=NULL){
			userCachePath=env;
		}else{
			userCachePath=getenv("HOME");
			userCachePath+="/.cache";
		}
		//And add meandmyshadow to it.
		userCachePath+="/meandmyshadow/";
		
		//Set env null.
		env=NULL;
#endif
		
		//Print the userPath.
		cout<<"User preferences will be fetched from: "<<userPath<<endl;
#ifndef WIN32
		//In case of a non-Windows computer show the user data path.
		cout<<"User data will be fetched from: "<<userDataPath<<endl;
#endif
	}

#ifdef WIN32
	//Create the userPath folder and other subfolders.
	createDirectory(userPath.c_str());
	createDirectory((userPath+"levels").c_str());
	createDirectory((userPath+"levelpacks").c_str());
	createDirectory((userPath+"themes").c_str());
	createDirectory((userPath+"progress").c_str());
	createDirectory((userPath+"tmp").c_str());
	//The records folder for recordings.
	createDirectory((userPath+"records").c_str());
	createDirectory((userPath+"records\\autosave").c_str());
	//And the custom folder inside the userpath.
	createDirectory((userPath+"custom").c_str());
	createDirectory((userPath+"custom\\levels").c_str());
	createDirectory((userPath+"custom\\levelpacks").c_str());
#else
	//Create the userPath.
	createDirectory(userPath.c_str());
	createDirectory(userDataPath.c_str());
	createDirectory(userCachePath.c_str());
	//Also create other folders in the userpath.
	createDirectory((userDataPath+"/levels").c_str());
	createDirectory((userDataPath+"/levelpacks").c_str());
	createDirectory((userDataPath+"/themes").c_str());
	createDirectory((userDataPath+"/progress").c_str());
	createDirectory((userCachePath+"/tmp").c_str());
	//The records folder for recordings.
	createDirectory((userDataPath+"/records").c_str());
	createDirectory((userDataPath+"/records/autosave").c_str());
	//And the custom folder inside the userpath.
	createDirectory((userDataPath+"/custom").c_str());
	createDirectory((userDataPath+"/custom/levels").c_str());
	createDirectory((userDataPath+"/custom/levelpacks").c_str());
#endif

	//Get the dataPath by trying multiple relative locations.
	{
		FILE *f;
		string s;
		while(true){
			//try existing one
			if(!dataPath.empty()){
				s=dataPath+"font/knewave.ttf";
				if((f=fopen(s.c_str(),"rb"))!=NULL){
					fclose(f);
					break;
				}
			}
			//try "./"
			dataPath="./data/";
			s=dataPath+"font/knewave.ttf";
			if((f=fopen(s.c_str(),"rb"))!=NULL){
				fclose(f);
				break;
			}
			//try "../"
			dataPath="../data/";
			s=dataPath+"font/knewave.ttf";
			if((f=fopen(s.c_str(),"rb"))!=NULL){
				fclose(f);
				break;
			}
			//try App.Path
			dataPath=getAppPath()+"/data/";
			s=dataPath+"font/knewave.ttf";
			if((f=fopen(s.c_str(),"rb"))!=NULL){
				fclose(f);
				break;
			}
			//try App.Path+"/../"
			dataPath=getAppPath()+"/../data/";
			s=dataPath+"font/knewave.ttf";
			if((f=fopen(s.c_str(),"rb"))!=NULL){
				fclose(f);
				break;
			}
			//try DATA_PATH
#ifdef DATA_PATH
			dataPath=DATA_PATH;
			s=dataPath+"font/knewave.ttf";
			if((f=fopen(s.c_str(),"rb"))!=NULL){
				fclose(f);
				break;
			}
#endif
			//error: can't find file
			return false;
		}

		//Print the dataPath.
		cout<<"Data files will be fetched from: "<<dataPath<<endl;
	}
	return true;
}

std::vector<std::string> enumAllFiles(std::string path,const char* extension){
	vector<string> v;
#ifdef WIN32
	string s1;
	WIN32_FIND_DATAA f;
	if(!path.empty()){
		char c=path[path.size()-1];
		if(c!='/'&&c!='\\') path+="\\";
	}
	s1=path;
	if(extension!=NULL && *extension){
		s1+="*.";
		s1+=extension;
	}else{
		s1+="*";
	}
	HANDLE h=FindFirstFileA(s1.c_str(),&f);
	if(h==NULL||h==INVALID_HANDLE_VALUE) return v;
	do{
		if(!(f.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)){
			v.push_back(/*path+*/f.cFileName);
		}
	}while(FindNextFileA(h,&f));
	FindClose(h);
	return v;
#else
	int len=0;
	if(extension!=NULL && *extension) len=strlen(extension);
	if(!path.empty()){
		char c=path[path.size()-1];
		if(c!='/'&&c!='\\') path+="/";
	}
	DIR *pDir;
	struct dirent *pDirent;
	pDir=opendir(path.c_str());
	if(pDir==NULL) return v;
	while((pDirent=readdir(pDir))!=NULL){
		if(pDirent->d_name[0]=='.'){
			if(pDirent->d_name[1]==0||
				(pDirent->d_name[1]=='.'&&pDirent->d_name[2]==0)) continue;
		}
		string s1=path+pDirent->d_name;
		struct stat S_stat;
		lstat(s1.c_str(),&S_stat);
		if(!S_ISDIR(S_stat.st_mode)){
			if(len>0){
				if((int)s1.size()<len+1) continue;
				if(s1[s1.size()-len-1]!='.') continue;
				if(strcasecmp(&s1[s1.size()-len],extension)) continue;
			}
			v.push_back(/*s1*/string(pDirent->d_name));
		}
	}
	closedir(pDir);
	return v;
#endif
}

std::vector<std::string> enumAllDirs(std::string path){
	vector<string> v;
#ifdef WIN32
	string s1;
	WIN32_FIND_DATAA f;
	if(!path.empty()){
		char c=path[path.size()-1];
		if(c!='/'&&c!='\\') path+="\\";
	}
	s1=path+"*";
	HANDLE h=FindFirstFileA(s1.c_str(),&f);
	if(h==NULL||h==INVALID_HANDLE_VALUE) return v;
	do{
		// skip '.' and '..' and hidden folders
		if(f.cFileName[0]=='.'){
			/*if(f.cFileName[1]==0||
				(f.cFileName[1]=='.'&&f.cFileName[2]==0))*/ continue;
		}
		if(f.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
			v.push_back(/*path+*/f.cFileName);
		}
	}while(FindNextFileA(h,&f));
	FindClose(h);
	return v;
#else
	if(!path.empty()){
		char c=path[path.size()-1];
		if(c!='/'&&c!='\\') path+="/";
	}
	DIR *pDir;
	struct dirent *pDirent;
	pDir=opendir(path.c_str());
	if(pDir==NULL) return v;
	while((pDirent=readdir(pDir))!=NULL){
		if(pDirent->d_name[0]=='.'){
			if(pDirent->d_name[1]==0||
				(pDirent->d_name[1]=='.'&&pDirent->d_name[2]==0)) continue;
		}
		string s1=path+pDirent->d_name;
		struct stat S_stat;
		lstat(s1.c_str(),&S_stat);
		if(S_ISDIR(S_stat.st_mode)){
			//Skip hidden folders.
			s1=string(pDirent->d_name);
			if(s1.find('.')==0) continue;
			
			//Add result to vector.
			v.push_back(s1);
		}
	}
	closedir(pDir);
	return v;
#endif
}

std::string processFileName(const std::string& s){
	string prefix=dataPath;
  
	//FIXME: Do we still need those last three?
	//REMARK: maybe 'return prefix+s;' is not needed (?)
	// it causes some bugs such as can't save level progress
	if(s.compare(0,6,"%DATA%")==0){
		if(s.size()>6 && (s[6]=='/' || s[6]=='\\')){
			return dataPath+s.substr(7);
		}else{
			return dataPath+s.substr(6);
		}
	}else if(s.compare(0,6,"%USER%")==0){
		if(s.size()>6 && (s[6]=='/' || s[6]=='\\')){
			return getUserPath(USER_DATA)+s.substr(7);
		}else{
			return getUserPath(USER_DATA)+s.substr(6);
		}
	}else if(s.compare(0,9,"%LVLPACK%")==0){
		if(s.size()>9 && (s[9]=='/' || s[9]=='\\')){
			return prefix+"levelpacks/"+s.substr(10);
		}else{
			return prefix+"levelpacks/"+s.substr(9);
		}
	}else if(s.compare(0,5,"%LVL%")==0){
		if(s.size()>5 && (s[5]=='/' || s[5]=='\\')){
			return prefix+"levels/"+s.substr(6);
		}else{
			return prefix+"levels/"+s.substr(5);
		}
	}else if(s.compare(0,8,"%THEMES%")==0){
		if(s.size()>8 && (s[8]=='/' || s[8]=='\\')){
			return prefix+"themes/"+s.substr(9);
		}else{
			return prefix+"themes/"+s.substr(8);
		}
	}else if(s.size()>0 && (s[0]=='/' || s[0]=='\\')){
		return s;
#ifdef WIN32
	// Another fix for Windows :(
	}else if(s.size()>1 && (s[1]==':')){
		return s;
#endif
	}else{
		return prefix+s;
	}
}

std::string fileNameFromPath(const std::string &path, const bool webURL){
	std::string filename;
	size_t pos;
#ifdef WIN32
	// FIXME: '/' in string should be '/' not '\/',
	// we don't need to escape it
	if(webURL){
		pos = path.find_last_of("/");
	}else{
		// NOTE: sometimes path separator in Windows can be '/',
		// so we must check botn '\' and '/'
		pos = path.find_last_of("\\/");
	}
#else
	// FIXME: '/' in string should be '/' not '\/',
	// we don't need to escape it
	pos = path.find_last_of("/");
#endif
	if(pos != std::string::npos)
		filename.assign(path.begin() + pos + 1, path.end());
	else
		filename=path;
	
	return filename;
}

std::string pathFromFileName(const std::string &filename){
	std::string path;
	// FIXME: '/' in string should be '/' not '\/',
	// we don't need to escape it
#ifdef WIN32
	// NOTE: sometimes path separator in Windows can be '/',
	// so we must check botn '\' and '/'
	size_t pos = filename.find_last_of("\\/");
#else
	size_t pos = filename.find_last_of("/");
#endif
	if(pos != std::string::npos)
		path.assign(filename.begin(), filename.begin() + pos +1);
	else
		path=filename;
	
	return path;
}

bool downloadFile(const string &path, const string &destination) {
#ifdef __EMSCRIPTEN__
  return false;
#else
	string filename=fileNameFromPath(path,true);
	
	FILE* file = fopen((destination+filename).c_str(), "wb");
	bool status=downloadFile(path,file);
	fclose(file);
	
	//And return the status.
	return status;
#endif
}

bool downloadFile(const string &path, FILE* destination) {
#ifdef __EMSCRIPTEN__
  return false;
#else
	CURL* curl=curl_easy_init();

	// proxy test (test only)
	string internetProxy = getSettings()->getValue("internet-proxy");
	size_t pos = internetProxy.find_first_of(":");
	if(pos!=string::npos){
		curl_easy_setopt(curl,CURLOPT_PROXYPORT,atoi(internetProxy.substr(pos+1).c_str()));
		internetProxy = internetProxy.substr(0,pos);
		curl_easy_setopt(curl,CURLOPT_PROXY,internetProxy.c_str());
	}

	curl_easy_setopt(curl,CURLOPT_URL,path.c_str());
	curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,writeData);
	curl_easy_setopt(curl,CURLOPT_WRITEDATA,destination);
	CURLcode res = curl_easy_perform(curl);
	curl_easy_cleanup(curl);
	
	return (res==0);
#endif
}

size_t writeData(void *ptr, size_t size, size_t nmemb, void *stream){
	return fwrite(ptr, size, nmemb, (FILE *)stream);
}

bool extractFile(const string &fileName, const string &destination) {
#ifdef __EMSCRIPTEN__
  return false;
#else
	//Create the archive we're going to extract.
	archive* file=NULL;
	//Create the destination we're going to extract to.
	archive* dest=NULL;
	
	file=archive_read_new();
	dest=archive_write_disk_new();
	archive_write_disk_set_options(dest, ARCHIVE_EXTRACT_TIME);
	
	archive_read_support_format_zip(file);
	
	//Now read the archive.
	if(archive_read_open_file(file,fileName.c_str(),10240)) {
		cerr<<"Error while reading archive "+fileName<<endl;
		return false;
	}
	
	//Now write every entry to disk.
	int status;
	archive_entry* entry=NULL;
	while(true) {
		status=archive_read_next_header(file,&entry);
		if(status==ARCHIVE_EOF){
			break;
		}
		if(status!=ARCHIVE_OK){
			cerr<<"Error while reading archive "+fileName<<endl;
			return false;
		}
		archive_entry_copy_pathname(entry,(destination+archive_entry_pathname(entry)).c_str());
		
		status=archive_write_header(dest,entry);
		if(status!=ARCHIVE_OK){
			cerr<<"Error while extracting archive "+fileName<<endl;
			return false;
		}else{
			copyData(file, dest);
			status=archive_write_finish_entry(dest);
			if(status!=ARCHIVE_OK){
				cerr<<"Error while extracting archive "+fileName<<endl;
				return false;
			}

		}
	}
	
	//Finally close the archive.
	archive_read_close(file);
	archive_read_finish(file);
	return true;
#endif
}

bool createDirectory(const char* path){
#ifdef WIN32
	char s0[1024],s[1024];

	GetCurrentDirectoryA(sizeof(s0),s0);
	PathCombineA(s,s0,path);

	for(unsigned int i=0;i<sizeof(s);i++){
		if(s[i]=='\0') break;
		else if(s[i]=='/') s[i]='\\';
	}

	//printf("createDirectory:%s\n",s);
	return SHCreateDirectoryExA(NULL,s,NULL)!=0;
#else
	return mkdir(path,0777)==0;
#endif
}

bool removeDirectory(const char *path){
#ifdef WIN32
	WIN32_FIND_DATAA f;
	HANDLE h = FindFirstFileA((string(path)+"\\*").c_str(),&f);
#else
	//Open the directory that needs to be removed.
	DIR* d=opendir(path);
#endif
	//Get the path length
	size_t path_len = strlen(path);
	//Boolean if the directory is emptied.
	//True: succes		False: failure
	//Default is true because if the directory is empty it will never enter the while loop, but we still have success.
	bool r = true;

#ifdef WIN32
	if(h!=NULL && h!=INVALID_HANDLE_VALUE) {
#else
	//Check if the directory exists.
	if(d) {
		//Pointer to an entry of the directory.
		struct dirent* p;
#endif

#ifdef WIN32
		do{
#else
		//Loop the entries of the directory that needs to be removed as long as there's no error.
		while(r && (p=readdir(d))) {
#endif
			//Booleand if the entry is deleted.
			//True: succes		False: failure
			//Default is false.
			bool r2 = false;
			char* buf;
			size_t len;

			/* Skip the names "." and ".." as we don't want to recurse on them. */
#ifdef WIN32
			if (!strcmp(f.cFileName, ".") || !strcmp(f.cFileName, "..")) {
#else
			if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, "..")) {
#endif
				//The filename is . or .. so we continue to the next entry.
				continue;
			} else {

#ifdef WIN32
				//Get the length of the path + the directory entry name.
				len = path_len + strlen(f.cFileName) + 2; 
#else
				//Get the length of the path + the directory entry name.
				len = path_len + strlen(p->d_name) + 2; 
#endif
				buf = (char*) malloc(len);

				if(buf) {
#ifdef WIN32
					_snprintf(buf, len, "%s\\%s", path, f.cFileName);

					if(f.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
						r2 = removeDirectory(buf);
					}else{
						r2 = unlink(buf)==0;
					}
#else
					struct stat statbuf;
					snprintf(buf, len, "%s/%s", path, p->d_name);

					if(!stat(buf, &statbuf)){
						//Check if the entry is a directory or a file.
						if (S_ISDIR(statbuf.st_mode)){
							//We call ourself(removeDirectory) recursively.
							//We return true on success.
							r2 = removeDirectory(buf);
						}else{
							//unlink() returns zero on succes so we set r2 to the unlink(buf)==0.
							r2 = unlink(buf)==0;
						}
					}
#endif
					//Free the buf.
					free(buf);
				}
				//We set r to r2 since r2 contains the status of the latest deletion.
				r = r2;
			}
#ifdef WIN32
		}while(r && FindNextFileA(h,&f));
		FindClose(h);
#else
		}
		//Close the directory.
		closedir(d);
#endif
	}
	
	//The while loop has ended, meaning we (tried) cleared the directory.
	//If r is true, meaning no errors we can delete the directory.
	if(r){
		//The return value of rmdir is 0 when it succeeded.
		r = rmdir(path)==0;
	}
	
	//Return the status.
	return r;
}

bool renameDirectory(const char* oldPath,const char* newPath){
	return rename(oldPath,newPath)==0;
}


#ifndef EMSCRIPTEN
void copyData(archive* file, archive* dest) {
	int status;
	const void* buff;
	size_t size;
#if ARCHIVE_VERSION_NUMBER < 3000000
	off_t offset;
#else
	int64_t offset;
#endif


	while(true) {
		status=archive_read_data_block(file, &buff, &size, &offset);
		if(status==ARCHIVE_EOF){
			return;
		}
		if(status!=ARCHIVE_OK){
			cerr<<"Error while writing data to disk."<<endl;
			return;
		}
		status=archive_write_data_block(dest, buff, size, offset);
		if(status!=ARCHIVE_OK) {
			cerr<<"Error while writing data to disk."<<endl;
			return;
		}
	}
}
#endif

bool copyFile(const char* source,const char* dest){
	//Open the source file.
	ifstream fin(source,fstream::binary);
	if(!fin)
		return false;
	
	//Open the dest file.
	ofstream fout(dest,fstream::trunc|fstream::binary);
	if(!fout)
		return false;
	
	//Copy.
	fout<<fin.rdbuf();
	return true;
}

bool removeFile(const char* file){
	return remove(file)==0;
}

bool createFile(const char* file){
	//Open the file with write permission.
	FILE* f=fopen(file,"wb");
	
	//Check if there are no problems.
	if(f){
		//Close the file.
		fclose(f);
		return true;
	}else{
		return false;
	}
}

