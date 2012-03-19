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

#ifndef MD5_H
#define MD5_H

#define MD5_CTX_SIZE 128

class Md5{
public:
	//compute the MD5 message digest of the n bytes at d and place it in md
	//(which must have space for 16 bytes of output). If md is NULL,
	//the digest is placed in a static array. 
	static unsigned char *calc(const void *d, unsigned long n, unsigned char *md);

	//convert a 16-byte digest to string representation.
	//the return value is in a static array.
	static char *toString(unsigned char *md);

	//Following are lower-level functions.

	//initializes the class for calculating MD5.
	void init();
	//add chunks of the message to be hashed (len bytes at data). 
	void update(const void *data, unsigned long len);
	//finished the caluclation, places the message digest in md,
	//which must have space for 16 bytes of output (or NULL).
	unsigned char *final(unsigned char *md);
private:
	//byte array, should be large enough to holding MD5_CTX.
	char md5_ctx[MD5_CTX_SIZE];
};

#endif