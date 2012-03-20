#ifndef EMSCRIPTEN
/****************************************************************************
** Copyright (C) 2012 me and my shadow developers
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

#include "MD5.h"

#ifdef WIN32

#include <windows.h>

typedef struct MD5_CTX {
  ULONG         i[2];
  ULONG         buf[4];
  unsigned char in[64];
  unsigned char digest[16];
} MD5_CTX;

typedef void (WINAPI* F_MD5Init)(MD5_CTX *context);
typedef void (WINAPI* F_MD5Update)(MD5_CTX *context, const unsigned char *input, unsigned int inlen);
typedef void (WINAPI* F_MD5Final)(MD5_CTX *context);

//first time calling API will be redirect to these functions
static void WINAPI myMD5Init(MD5_CTX *context);
static void WINAPI myMD5Update(MD5_CTX *context, const unsigned char *input, unsigned int inlen);
static void WINAPI myMD5Final(MD5_CTX *context);

static F_MD5Init MD5Init=myMD5Init;
static F_MD5Update MD5Update=myMD5Update;
static F_MD5Final MD5Final=myMD5Final;

#else

#include <openssl/md5.h>

#endif

#include <assert.h>
#include <string.h>

#ifdef WIN32

static void loadMD5Functions(){
	HMODULE h;

	h=LoadLibraryA("cryptdll.dll");
	if(h!=NULL){
		MD5Init=(F_MD5Init)GetProcAddress(h,"MD5Init");
		MD5Update=(F_MD5Update)GetProcAddress(h,"MD5Update");
		MD5Final=(F_MD5Final)GetProcAddress(h,"MD5Final");
		if(MD5Init==NULL || MD5Update==NULL || MD5Final==NULL) FreeLibrary(h);
		else return;
	}

	h=LoadLibraryA("advapi32.dll");
	if(h!=NULL){
		MD5Init=(F_MD5Init)GetProcAddress(h,"MD5Init");
		MD5Update=(F_MD5Update)GetProcAddress(h,"MD5Update");
		MD5Final=(F_MD5Final)GetProcAddress(h,"MD5Final");
		if(MD5Init==NULL || MD5Update==NULL || MD5Final==NULL) FreeLibrary(h);
		else return;
	}

	FatalAppExitA(-1,"Can't find MD5 functions!!!");
}

static void WINAPI myMD5Init(MD5_CTX *context){
	loadMD5Functions();
	MD5Init(context);
}

static void WINAPI myMD5Update(MD5_CTX *context, const unsigned char *input, unsigned int inlen){
	loadMD5Functions();
	MD5Update(context,input,inlen);
}

static void WINAPI myMD5Final(MD5_CTX *context){
	loadMD5Functions();
	MD5Final(context);
}

#endif

//compute the MD5 message digest of the n bytes at d and place it in md
//(which must have space for 16 bytes of output). If md is NULL,
//the digest is placed in a static array. 
unsigned char *Md5::calc(const void *d, unsigned long n, unsigned char *md){
#ifdef WIN32
	static MD5_CTX ctx;

	MD5Init(&ctx);
	MD5Update(&ctx,(const unsigned char*)d,n);
	MD5Final(&ctx);

	if(md){
		memcpy(md,ctx.digest,16);
		return md;
	}else{
		return ctx.digest;
	}
#else
	return MD5((const unsigned char*)d,n,md);
#endif
}

char *Md5::toString(unsigned char *md){
	static char s[40];
	const char* hex="0123456789abcdef";
	
	for(int i=0;i<16;i++){
		s[i*2]=hex[(md[i]&0xF0)>>4];
		s[i*2+1]=hex[md[i]&0xF];
	}
	s[32]='\0';

	return s;
}

//initializes the class for calculating MD5.
void Md5::init(){
	//First check the size
	assert(sizeof(MD5_CTX)<=MD5_CTX_SIZE);

#ifdef WIN32
	MD5Init((MD5_CTX*)md5_ctx);
#else
	MD5_Init((MD5_CTX*)md5_ctx);
#endif
}

//add chunks of the message to be hashed (len bytes at data). 
void Md5::update(const void *data, unsigned long len){
	//First check the size
	assert(sizeof(MD5_CTX)<=MD5_CTX_SIZE);

#ifdef WIN32
	MD5Update((MD5_CTX*)md5_ctx,(const unsigned char*)data,len);
#else
	MD5_Update((MD5_CTX*)md5_ctx,data,len);
#endif
}

//finished the caluclation, places the message digest in md,
//which must have space for 16 bytes of output (or NULL).
unsigned char *Md5::final(unsigned char *md){
	static unsigned char digest[16];
	//First check the size
	assert(sizeof(MD5_CTX)<=MD5_CTX_SIZE);

#ifdef WIN32
	MD5_CTX *ctx=(MD5_CTX*)md5_ctx;
	MD5Final(ctx);
	if(md==NULL) md=digest;
	memcpy(md,ctx->digest,16);
	return md;
#else
	if(md==NULL) md=digest;
	MD5_Final(md,(MD5_CTX*)md5_ctx);
	return md;
#endif
}
#endif

