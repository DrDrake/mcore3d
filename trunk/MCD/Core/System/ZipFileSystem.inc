#include "../../../3Party/zlib/zlib.h"

// A very easy to use zip file library from:
// The CodeProject: http://www.codeproject.com/KB/files/zip_utils.aspx
// Author's homepage: http://www.wischik.com/lu/programmer/zip_utils_src.zip

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <time.h>
#ifdef _MSC_VER
#include <sys/utime.h> // microsoft puts it here
#else
#include <utime.h>
#endif
#if defined(_MSC_VER) || defined(__BORLANDC__) || defined(__MINGW32__)
#include <direct.h>
#else
#include <unistd.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>

// I have move the original unzip.h merge with this file
// #include "unzip.h"
// Begin of unzip.h"

#include <time.h>
#define DECLARE_HANDLE(name) struct name##__ { int unused; }; typedef struct name##__ *name
#ifndef MAX_PATH
#	define MAX_PATH 1024
#endif
typedef unsigned long DWORD;
typedef char TCHAR;
typedef FILE* HANDLE;
typedef time_t FILETIME;

// UNZIPPING functions -- for unzipping.
// This file is a repackaged form of extracts from the zlib code available
// at www.gzip.org/zlib, by Jean-Loup Gailly and Mark Adler. The original
// copyright notice may be found in unzip.cpp. The repackaging was done
// by Lucian Wischik to simplify and extend its use in Windows/C++. Also
// encryption and unicode filenames have been added.


#ifndef _zip_H
DECLARE_HANDLE(HZIP);
#endif
// An HZIP identifies a zip file that has been opened

typedef DWORD ZRESULT;
// return codes from any of the zip functions. Listed later.

typedef struct
{ int index;                 // index of this file within the zip
  TCHAR name[MAX_PATH];      // filename within the zip
  DWORD attr;                // attributes, as in GetFileAttributes.
  FILETIME atime,ctime,mtime;// access, create, modify filetimes
  long comp_size;            // sizes of item, compressed and uncompressed. These
  long unc_size;             // may be -1 if not yet known (e.g. being streamed in)
} ZIPENTRY;


HZIP OpenZip(const TCHAR *fn, const char *password);
HZIP OpenZip(void *z,unsigned int len, const char *password);
HZIP OpenZipHandle(HANDLE h, const char *password);
// OpenZip - opens a zip file and returns a handle with which you can
// subsequently examine its contents. You can open a zip file from:
// from a pipe:             OpenZipHandle(hpipe_read,0);
// from a file (by handle): OpenZipHandle(hfile,0);
// from a file (by name):   OpenZip("c:\\test.zip","password");
// from a memory block:     OpenZip(bufstart, buflen,0);
// If the file is opened through a pipe, then items may only be
// accessed in increasing order, and an item may only be unzipped once,
// although GetZipItem can be called immediately before and after unzipping
// it. If it's opened in any other way, then full random access is possible.
// Note: pipe input is not yet implemented.
// Note: zip passwords are ascii, not unicode.
// Note: for windows-ce, you cannot close the handle until after CloseZip.
// but for real windows, the zip makes its own copy of your handle, so you
// can close yours anytime.

ZRESULT GetZipItem(HZIP hz, int index, ZIPENTRY *ze);
// GetZipItem - call this to get information about an item in the zip.
// If index is -1 and the file wasn't opened through a pipe,
// then it returns information about the whole zipfile
// (and in particular ze.index returns the number of index items).
// Note: the item might be a directory (ze.attr & FILE_ATTRIBUTE_DIRECTORY)
// See below for notes on what happens when you unzip such an item.
// Note: if you are opening the zip through a pipe, then random access
// is not possible and GetZipItem(-1) fails and you can't discover the number
// of items except by calling GetZipItem on each one of them in turn,
// starting at 0, until eventually the call fails. Also, in the event that
// you are opening through a pipe and the zip was itself created into a pipe,
// then then comp_size and sometimes unc_size as well may not be known until
// after the item has been unzipped.

ZRESULT FindZipItem(HZIP hz, const TCHAR *name, bool ic, int *index, ZIPENTRY *ze);
// FindZipItem - finds an item by name. ic means 'insensitive to case'.
// It returns the index of the item, and returns information about it.
// If nothing was found, then index is set to -1 and the function returns
// an error code.

ZRESULT UnzipItem(HZIP hz, int index, const TCHAR *fn);
ZRESULT UnzipItem(HZIP hz, int index, void *z,unsigned int len);
ZRESULT UnzipItemHandle(HZIP hz, int index, HANDLE h);
// UnzipItem - given an index to an item, unzips it. You can unzip to:
// to a pipe:             UnzipItemHandle(hz,i, hpipe_write);
// to a file (by handle): UnzipItemHandle(hz,i, hfile);
// to a file (by name):   UnzipItem(hz,i, ze.name);
// to a memory block:     UnzipItem(hz,i, buf,buflen);
// In the final case, if the buffer isn't large enough to hold it all,
// then the return code indicates that more is yet to come. If it was
// large enough, and you want to know precisely how big, GetZipItem.
// Note: zip files are normally stored with relative pathnames. If you
// unzip with ZIP_FILENAME a relative pathname then the item gets created
// relative to the current directory - it first ensures that all necessary
// subdirectories have been created. Also, the item may itself be a directory.
// If you unzip a directory with ZIP_FILENAME, then the directory gets created.
// If you unzip it to a handle or a memory block, then nothing gets created
// and it emits 0 bytes.


ZRESULT CloseZip(HZIP hz);
// CloseZip - the zip handle must be closed with this function.

unsigned int FormatZipMessage(ZRESULT code, TCHAR *buf,unsigned int len);
// FormatZipMessage - given an error code, formats it as a string.
// It returns the length of the error message. If buf/len points
// to a real buffer, then it also writes as much as possible into there.


// These are the result codes:
#define ZR_OK         0x00000000     // nb. the pseudo-code zr-recent is never returned,
#define ZR_RECENT     0x00000001     // but can be passed to FormatZipMessage.
// The following come from general system stuff (e.g. files not openable)
#define ZR_GENMASK    0x0000FF00
#define ZR_NODUPH     0x00000100     // couldn't duplicate the handle
#define ZR_NOFILE     0x00000200     // couldn't create/open the file
#define ZR_NOALLOC    0x00000300     // failed to allocate some resource
#define ZR_WRITE      0x00000400     // a general error writing to the file
#define ZR_NOTFOUND   0x00000500     // couldn't find that file in the zip
#define ZR_MORE       0x00000600     // there's still more data to be unzipped
#define ZR_CORRUPT    0x00000700     // the zipfile is corrupt or not a zipfile
#define ZR_READ       0x00000800     // a general error reading the file
#define ZR_PASSWORD   0x00001000     // we didn't get the right password to unzip the file
// The following come from mistakes on the part of the caller
#define ZR_CALLERMASK 0x00FF0000
#define ZR_ARGS       0x00010000     // general mistake with the arguments
#define ZR_NOTMMAP    0x00020000     // tried to ZipGetMemory, but that only works on mmap zipfiles, which yours wasn't
#define ZR_MEMSIZE    0x00030000     // the memory size is too small
#define ZR_FAILED     0x00040000     // the thing was already failed when you called this function
#define ZR_ENDED      0x00050000     // the zip creation has already been closed
#define ZR_MISSIZE    0x00060000     // the indicated input file size turned out mistaken
#define ZR_PARTIALUNZ 0x00070000     // the file had already been partially unzipped
#define ZR_ZMODE      0x00080000     // tried to mix creating/opening a zip
// The following come from bugs within the zip library itself
#define ZR_BUGMASK    0xFF000000
#define ZR_NOTINITED  0x01000000     // initialisation didn't work
#define ZR_SEEK       0x02000000     // trying to seek in an unseekable file
#define ZR_NOCHANGE   0x04000000     // changed its mind on storage, but not allowed
#define ZR_FLATE      0x05000000     // an internal error in the de/inflation code



// e.g.
//
// SetCurrentDirectory("c:\\docs\\stuff");
// HZIP hz = OpenZip("c:\\stuff.zip",0);
// ZIPENTRY ze; GetZipItem(hz,-1,&ze); int numitems=ze.index;
// for (int i=0; i<numitems; i++)
// { GetZipItem(hz,i,&ze);
//   UnzipItem(hz,i,ze.name);
// }
// CloseZip(hz);
//
//
// HRSRC hrsrc = FindResource(hInstance,MAKEINTRESOURCE(1),RT_RCDATA);
// HANDLE hglob = LoadResource(hInstance,hrsrc);
// void *zipbuf=LockResource(hglob);
// unsigned int ziplen=SizeofResource(hInstance,hrsrc);
// HZIP hz = OpenZip(zipbuf, ziplen, 0);
//   - unzip to a membuffer -
// ZIPENTRY ze; int i; FindZipItem(hz,"file.dat",true,&i,&ze);
// char *ibuf = new char[ze.unc_size];
// UnzipItem(hz,i, ibuf, ze.unc_size);
// delete[] ibuf;
//   - unzip to a fixed membuff -
// ZIPENTRY ze; int i; FindZipItem(hz,"file.dat",true,&i,&ze);
// char ibuf[1024]; ZRESULT zr=ZR_MORE; unsigned long totsize=0;
// while (zr==ZR_MORE)
// { zr = UnzipItem(hz,i, ibuf,1024);
//   unsigned long bufsize=1024; if (zr==ZR_OK) bufsize=ze.unc_size-totsize;
//   totsize+=bufsize;
// }
//   - unzip to a pipe -
// HANDLE hwrite; HANDLE hthread=CreateWavReaderThread(&hwrite);
// int i; ZIPENTRY ze; FindZipItem(hz,"sound.wav",true,&i,&ze);
// UnzipItemHandle(hz,i, hwrite);
// CloseHandle(hwrite);
// WaitForSingleObject(hthread,INFINITE);
// CloseHandle(hwrite); CloseHandle(hthread);
//   - finished -
// CloseZip(hz);
// // note: no need to free resources obtained through Find/Load/LockResource
//
//
// SetCurrentDirectory("c:\\docs\\pipedzipstuff");
// HANDLE hread,hwrite; CreatePipe(&hread,&hwrite,0,0);
// CreateZipWriterThread(hwrite);
// HZIP hz = OpenZipHandle(hread,0);
// for (int i=0; ; i++)
// { ZIPENTRY ze;
//   ZRESULT zr=GetZipItem(hz,i,&ze); if (zr!=ZR_OK) break; // no more
//   UnzipItem(hz,i, ze.name);
// }
// CloseZip(hz);
//
//


// Now we indulge in a little skullduggery so that the code works whether
// the user has included just zip or both zip and unzip.
// Idea: if header files for both zip and unzip are present, then presumably
// the cpp files for zip and unzip are both present, so we will call
// one or the other of them based on a dynamic choice. If the header file
// for only one is present, then we will bind to that particular one.
ZRESULT CloseZipU(HZIP hz);
unsigned int FormatZipMessageU(ZRESULT code, TCHAR *buf,unsigned int len);
bool IsZipHandleU(HZIP hz);
#ifdef _zip_H
#undef CloseZip
#define CloseZip(hz) (IsZipHandleU(hz)?CloseZipU(hz):CloseZipZ(hz))
#else
#define CloseZip CloseZipU
#define FormatZipMessage FormatZipMessageU
#endif

// End of unzip.h

//
typedef unsigned short WORD;
#define _tcslen strlen
#define _tcsicmp stricmp
#define _tcsncpy strncpy
#define _tcsstr strstr
#define INVALID_HANDLE_VALUE 0
#ifndef _T
#define _T(s) s
#endif
#ifndef S_IWUSR
#define S_IWUSR 0000200
#define S_ISDIR(m) (((m) & _S_IFMT) == _S_IFDIR)
#define S_ISREG(m) (((m) & _S_IFMT) == _S_IFREG)
#endif
//


// THIS FILE is almost entirely based upon code by Jean-loup Gailly
// and Mark Adler. It has been modified by Lucian Wischik.
// The modifications were: incorporate the bugfixes of 1.1.4, allow
// unzipping to/from handles/pipes/files/memory, encryption, unicode,
// a windowsish api, and putting everything into a single .cpp file.
// The original code may be found at http://www.gzip.org/zlib/
// The original copyright text follows.
//
//
//
// zlib.h -- interface of the 'zlib' general purpose compression library
//  version 1.1.3, July 9th, 1998
//
//  Copyright (C) 1995-1998 Jean-loup Gailly and Mark Adler
//
//  This software is provided 'as-is', without any express or implied
//  warranty.  In no event will the authors be held liable for any damages
//  arising from the use of this software.
//
//  Permission is granted to anyone to use this software for any purpose,
//  including commercial applications, and to alter it and redistribute it
//  freely, subject to the following restrictions:
//
//  1. The origin of this software must not be misrepresented; you must not
//     claim that you wrote the original software. If you use this software
//     in a product, an acknowledgment in the product documentation would be
//     appreciated but is not required.
//  2. Altered source versions must be plainly marked as such, and must not be
//     misrepresented as being the original software.
//  3. This notice may not be removed or altered from any source distribution.
//
//  Jean-loup Gailly        Mark Adler
//  jloup@gzip.org          madler@alumni.caltech.edu
//
//
//  The data format used by the zlib library is described by RFCs (Request for
//  Comments) 1950 to 1952 in the files ftp://ds.internic.net/rfc/rfc1950.txt
//  (zlib format), rfc1951.txt (deflate format) and rfc1952.txt (gzip format).
//
//
//     The 'zlib' compression library provides in-memory compression and
//  decompression functions, including integrity checks of the uncompressed
//  data.  This version of the library supports only one compression method
//  (deflation) but other algorithms will be added later and will have the same
//  stream interface.
//
//     Compression can be done in a single step if the buffers are large
//  enough (for example if an input file is mmap'ed), or can be done by
//  repeated calls of the compression function.  In the latter case, the
//  application must provide more input and/or consume the output
//  (providing more output space) before each call.
//
//     The library also supports reading and writing files in gzip (.gz) format
//  with an interface similar to that of stdio.
//
//     The library does not install any signal handler. The decoder checks
//  the consistency of the compressed data, so the library should never
//  crash even in case of corrupted input.
//
// for more info about .ZIP format, see ftp://ftp.cdrom.com/pub/infozip/doc/appnote-970311-iz.zip
//   PkWare has also a specification at ftp://ftp.pkware.com/probdesc.zip

#define ZIP_HANDLE   1
#define ZIP_FILENAME 2
#define ZIP_MEMORY   3


#define zmalloc(len) malloc(len)

#define zfree(p) free(p)

typedef unsigned long lutime_t;       // define it ourselves since we don't include time.h

/*
void *zmalloc(unsigned int len)
{ char *buf = new char[len+32];
  for (int i=0; i<16; i++)
  { buf[i]=i;
    buf[len+31-i]=i;
  }
  *((unsigned int*)buf) = len;
  char c[1000]; wsprintf(c,"malloc 0x%lx  - %lu",buf+16,len);
  OutputDebugString(c);
  return buf+16;
}

void zfree(void *buf)
{ char c[1000]; wsprintf(c,"free   0x%lx",buf);
  OutputDebugString(c);
  char *p = ((char*)buf)-16;
  unsigned int len = *((unsigned int*)p);
  bool blown=false;
  for (int i=0; i<16; i++)
  { char lo = p[i];
    char hi = p[len+31-i];
    if (hi!=i || (lo!=i && i>4)) blown=true;
  }
  if (blown)
  { OutputDebugString("BLOWN!!!");
  }
  delete[] p;
}
*/


typedef struct tm_unz_s
{ unsigned int tm_sec;            // seconds after the minute - [0,59]
  unsigned int tm_min;            // minutes after the hour - [0,59]
  unsigned int tm_hour;           // hours since midnight - [0,23]
  unsigned int tm_mday;           // day of the month - [1,31]
  unsigned int tm_mon;            // months since January - [0,11]
  unsigned int tm_year;           // years - [1980..2044]
} tm_unz;


// ----------------------------------------------------------------------
// some windows<->linux portability things
DWORD GetFilePosU(HANDLE hfout)
{ struct stat st; fstat(fileno(hfout),&st);
  if ((st.st_mode&S_IFREG)==0) return 0xFFFFFFFF;
  return ftell(hfout);
}

FILETIME dosdatetime2filetime(WORD dosdate,WORD dostime)
{ struct tm t;
  t.tm_year = (WORD)(((dosdate>>9)&0x7f) + 1980 - 1900);
  t.tm_isdst = -1;
  t.tm_mon = (WORD)((dosdate>>5)&0xf - 1);
  t.tm_mday = (WORD)(dosdate&0x1f);
  t.tm_hour = (WORD)((dostime>>11)&0x1f);
  t.tm_min = (WORD)((dostime>>5)&0x3f);
  t.tm_sec = (WORD)((dostime&0x1f)*2);
  time_t t2 = mktime(&t);
  return t2;
}

void LocalFileTimeToFileTime(FILETIME *lft, FILETIME *ft)
{ *ft = *lft;
}

FILETIME timet2filetime(const lutime_t t)
{ return t;
}

// ----------------------------------------------------------------------


// unz_global_info structure contain global data about the ZIPfile
typedef struct unz_global_info_s
{ unsigned long number_entry;         // total number of entries in the central dir on this disk
  unsigned long size_comment;         // size of the global comment of the zipfile
} unz_global_info;

// unz_file_info contain information about a file in the zipfile
typedef struct unz_file_info_s
{ unsigned long version;              // version made by                 2 bytes
  unsigned long version_needed;       // version needed to extract       2 bytes
  unsigned long flag;                 // general purpose bit flag        2 bytes
  unsigned long compression_method;   // compression method              2 bytes
  unsigned long dosDate;              // last mod file date in Dos fmt   4 bytes
  unsigned long crc;                  // crc-32                          4 bytes
  unsigned long compressed_size;      // compressed size                 4 bytes
  unsigned long uncompressed_size;    // uncompressed size               4 bytes
  unsigned long size_filename;        // filename length                 2 bytes
  unsigned long size_file_extra;      // extra field length              2 bytes
  unsigned long size_file_comment;    // file comment length             2 bytes
  unsigned long disk_num_start;       // disk number start               2 bytes
  unsigned long internal_fa;          // internal file attributes        2 bytes
  unsigned long external_fa;          // external file attributes        4 bytes
  tm_unz tmu_date;
} unz_file_info;


#define UNZ_OK                  (0)
#define UNZ_END_OF_LIST_OF_FILE (-100)
#define UNZ_ERRNO               (Z_ERRNO)
#define UNZ_EOF                 (0)
#define UNZ_PARAMERROR          (-102)
#define UNZ_BADZIPFILE          (-103)
#define UNZ_INTERNALERROR       (-104)
#define UNZ_CRCERROR            (-105)
#define UNZ_PASSWORD            (-106)


// case sensitivity when searching for filenames
#define CASE_SENSITIVE 1
#define CASE_INSENSITIVE 2


// basic functions


// Table of CRC-32's of all single-byte values (made by make_crc_table)
const uLong crc_table[256] = {
  0x00000000L, 0x77073096L, 0xee0e612cL, 0x990951baL, 0x076dc419L,
  0x706af48fL, 0xe963a535L, 0x9e6495a3L, 0x0edb8832L, 0x79dcb8a4L,
  0xe0d5e91eL, 0x97d2d988L, 0x09b64c2bL, 0x7eb17cbdL, 0xe7b82d07L,
  0x90bf1d91L, 0x1db71064L, 0x6ab020f2L, 0xf3b97148L, 0x84be41deL,
  0x1adad47dL, 0x6ddde4ebL, 0xf4d4b551L, 0x83d385c7L, 0x136c9856L,
  0x646ba8c0L, 0xfd62f97aL, 0x8a65c9ecL, 0x14015c4fL, 0x63066cd9L,
  0xfa0f3d63L, 0x8d080df5L, 0x3b6e20c8L, 0x4c69105eL, 0xd56041e4L,
  0xa2677172L, 0x3c03e4d1L, 0x4b04d447L, 0xd20d85fdL, 0xa50ab56bL,
  0x35b5a8faL, 0x42b2986cL, 0xdbbbc9d6L, 0xacbcf940L, 0x32d86ce3L,
  0x45df5c75L, 0xdcd60dcfL, 0xabd13d59L, 0x26d930acL, 0x51de003aL,
  0xc8d75180L, 0xbfd06116L, 0x21b4f4b5L, 0x56b3c423L, 0xcfba9599L,
  0xb8bda50fL, 0x2802b89eL, 0x5f058808L, 0xc60cd9b2L, 0xb10be924L,
  0x2f6f7c87L, 0x58684c11L, 0xc1611dabL, 0xb6662d3dL, 0x76dc4190L,
  0x01db7106L, 0x98d220bcL, 0xefd5102aL, 0x71b18589L, 0x06b6b51fL,
  0x9fbfe4a5L, 0xe8b8d433L, 0x7807c9a2L, 0x0f00f934L, 0x9609a88eL,
  0xe10e9818L, 0x7f6a0dbbL, 0x086d3d2dL, 0x91646c97L, 0xe6635c01L,
  0x6b6b51f4L, 0x1c6c6162L, 0x856530d8L, 0xf262004eL, 0x6c0695edL,
  0x1b01a57bL, 0x8208f4c1L, 0xf50fc457L, 0x65b0d9c6L, 0x12b7e950L,
  0x8bbeb8eaL, 0xfcb9887cL, 0x62dd1ddfL, 0x15da2d49L, 0x8cd37cf3L,
  0xfbd44c65L, 0x4db26158L, 0x3ab551ceL, 0xa3bc0074L, 0xd4bb30e2L,
  0x4adfa541L, 0x3dd895d7L, 0xa4d1c46dL, 0xd3d6f4fbL, 0x4369e96aL,
  0x346ed9fcL, 0xad678846L, 0xda60b8d0L, 0x44042d73L, 0x33031de5L,
  0xaa0a4c5fL, 0xdd0d7cc9L, 0x5005713cL, 0x270241aaL, 0xbe0b1010L,
  0xc90c2086L, 0x5768b525L, 0x206f85b3L, 0xb966d409L, 0xce61e49fL,
  0x5edef90eL, 0x29d9c998L, 0xb0d09822L, 0xc7d7a8b4L, 0x59b33d17L,
  0x2eb40d81L, 0xb7bd5c3bL, 0xc0ba6cadL, 0xedb88320L, 0x9abfb3b6L,
  0x03b6e20cL, 0x74b1d29aL, 0xead54739L, 0x9dd277afL, 0x04db2615L,
  0x73dc1683L, 0xe3630b12L, 0x94643b84L, 0x0d6d6a3eL, 0x7a6a5aa8L,
  0xe40ecf0bL, 0x9309ff9dL, 0x0a00ae27L, 0x7d079eb1L, 0xf00f9344L,
  0x8708a3d2L, 0x1e01f268L, 0x6906c2feL, 0xf762575dL, 0x806567cbL,
  0x196c3671L, 0x6e6b06e7L, 0xfed41b76L, 0x89d32be0L, 0x10da7a5aL,
  0x67dd4accL, 0xf9b9df6fL, 0x8ebeeff9L, 0x17b7be43L, 0x60b08ed5L,
  0xd6d6a3e8L, 0xa1d1937eL, 0x38d8c2c4L, 0x4fdff252L, 0xd1bb67f1L,
  0xa6bc5767L, 0x3fb506ddL, 0x48b2364bL, 0xd80d2bdaL, 0xaf0a1b4cL,
  0x36034af6L, 0x41047a60L, 0xdf60efc3L, 0xa867df55L, 0x316e8eefL,
  0x4669be79L, 0xcb61b38cL, 0xbc66831aL, 0x256fd2a0L, 0x5268e236L,
  0xcc0c7795L, 0xbb0b4703L, 0x220216b9L, 0x5505262fL, 0xc5ba3bbeL,
  0xb2bd0b28L, 0x2bb45a92L, 0x5cb36a04L, 0xc2d7ffa7L, 0xb5d0cf31L,
  0x2cd99e8bL, 0x5bdeae1dL, 0x9b64c2b0L, 0xec63f226L, 0x756aa39cL,
  0x026d930aL, 0x9c0906a9L, 0xeb0e363fL, 0x72076785L, 0x05005713L,
  0x95bf4a82L, 0xe2b87a14L, 0x7bb12baeL, 0x0cb61b38L, 0x92d28e9bL,
  0xe5d5be0dL, 0x7cdcefb7L, 0x0bdbdf21L, 0x86d3d2d4L, 0xf1d4e242L,
  0x68ddb3f8L, 0x1fda836eL, 0x81be16cdL, 0xf6b9265bL, 0x6fb077e1L,
  0x18b74777L, 0x88085ae6L, 0xff0f6a70L, 0x66063bcaL, 0x11010b5cL,
  0x8f659effL, 0xf862ae69L, 0x616bffd3L, 0x166ccf45L, 0xa00ae278L,
  0xd70dd2eeL, 0x4e048354L, 0x3903b3c2L, 0xa7672661L, 0xd06016f7L,
  0x4969474dL, 0x3e6e77dbL, 0xaed16a4aL, 0xd9d65adcL, 0x40df0b66L,
  0x37d83bf0L, 0xa9bcae53L, 0xdebb9ec5L, 0x47b2cf7fL, 0x30b5ffe9L,
  0xbdbdf21cL, 0xcabac28aL, 0x53b39330L, 0x24b4a3a6L, 0xbad03605L,
  0xcdd70693L, 0x54de5729L, 0x23d967bfL, 0xb3667a2eL, 0xc4614ab8L,
  0x5d681b02L, 0x2a6f2b94L, 0xb40bbe37L, 0xc30c8ea1L, 0x5a05df1bL,
  0x2d02ef8dL
};


// =============================================================
// some decryption routines
#define CRC32(c, b) (crc_table[((int)(c)^(b))&0xff]^((c)>>8))
void Uupdate_keys(unsigned long *keys, char c)
{ keys[0] = CRC32(keys[0],c);
  keys[1] += keys[0] & 0xFF;
  keys[1] = keys[1]*134775813L +1;
  keys[2] = CRC32(keys[2], keys[1] >> 24);
}
char Udecrypt_byte(unsigned long *keys)
{ unsigned temp = ((unsigned)keys[2] & 0xffff) | 2;
  return (char)(((temp * (temp ^ 1)) >> 8) & 0xff);
}
char zdecode(unsigned long *keys, char c)
{ c^=Udecrypt_byte(keys);
  Uupdate_keys(keys,c);
  return c;
}


// unzip.c -- IO on .zip files using zlib
// Version 0.15 beta, Mar 19th, 1998,
// Read unzip.h for more info


#define UNZ_BUFSIZE (16384)
#define UNZ_MAXFILENAMEINZIP (256)
#define SIZECENTRALDIRITEM (0x2e)
#define SIZEZIPLOCALHEADER (0x1e)


const char unz_copyright[] = " unzip 0.15 Copyright 1998 Gilles Vollant ";

// unz_file_info_interntal contain internal info about a file in zipfile
typedef struct unz_file_info_internal_s
{
    uLong offset_curfile;// relative offset of local header 4 bytes
} unz_file_info_internal;


typedef struct
{ bool is_handle; // either a handle or memory
  bool canseek;
  // for handles:
  HANDLE h; bool herr; unsigned long initial_offset; bool mustclosehandle;
  // for memory:
  void *buf; unsigned int len,pos; // if it's a memory block
} LUFILE;


LUFILE *lufopen(void *z,unsigned int len,DWORD flags,ZRESULT *err)
{ if (flags!=ZIP_HANDLE && flags!=ZIP_FILENAME && flags!=ZIP_MEMORY) {*err=ZR_ARGS; return NULL;}
  //
  HANDLE h=0; bool canseek=false; *err=ZR_OK;
  bool mustclosehandle=false;
  if (flags==ZIP_HANDLE||flags==ZIP_FILENAME)
  { if (flags==ZIP_HANDLE)
    { HANDLE hf = (HANDLE)z;
      h=hf; mustclosehandle=false;
#ifdef DuplicateHandle
      BOOL res = DuplicateHandle(GetCurrentProcess(),hf,GetCurrentProcess(),&h,0,FALSE,DUPLICATE_SAME_ACCESS);
      if (!res) mustclosehandle=true;
#endif
    }
    else
    {
      h=fopen((const char*)z,"rb");
      if (h==0) {*err=ZR_NOFILE; return NULL;}
      mustclosehandle=true;
    }
    // test if we can seek on it. We can't use GetFileType(h)==FILE_TYPE_DISK since it's not on CE.
    DWORD res = GetFilePosU(h);
    canseek = (res!=0xFFFFFFFF);
  }
  LUFILE *lf = new LUFILE;
  if (flags==ZIP_HANDLE||flags==ZIP_FILENAME)
  { lf->is_handle=true; lf->mustclosehandle=mustclosehandle;
    lf->canseek=canseek;
    lf->h=h; lf->herr=false;
    lf->initial_offset=0;
    if (canseek) lf->initial_offset = GetFilePosU(h);
  }
  else
  { lf->is_handle=false;
    lf->canseek=true;
    lf->mustclosehandle=false;
    lf->buf=z; lf->len=len; lf->pos=0; lf->initial_offset=0;
  }
  *err=ZR_OK;
  return lf;
}


int lufclose(LUFILE *stream)
{ if (stream==NULL) return EOF;
  if (stream->mustclosehandle) fclose(stream->h);
  delete stream;
  return 0;
}

int luferror(LUFILE *stream)
{ if (stream->is_handle && stream->herr) return 1;
  else return 0;
}

long int luftell(LUFILE *stream)
{ if (stream->is_handle && stream->canseek) return GetFilePosU(stream->h)-stream->initial_offset;
  else if (stream->is_handle) return 0;
  else return stream->pos;
}

int lufseek(LUFILE *stream, long offset, int whence)
{ if (stream->is_handle && stream->canseek)
  {
    return fseek(stream->h,stream->initial_offset+offset,whence);
  }
  else if (stream->is_handle) return 29; // ESPIPE
  else
  { if (whence==SEEK_SET) stream->pos=offset;
    else if (whence==SEEK_CUR) stream->pos+=offset;
    else if (whence==SEEK_END) stream->pos=stream->len+offset;
    return 0;
  }
}


size_t lufread(void *ptr,size_t size,size_t n,LUFILE *stream)
{ unsigned int toread = (unsigned int)(size*n);
  if (stream->is_handle)
  {
    return fread(ptr,size,n,stream->h);
  }
  if (stream->pos+toread > stream->len) toread = stream->len-stream->pos;
  memcpy(ptr, (char*)stream->buf + stream->pos, toread); DWORD red = toread;
  stream->pos += red;
  return red/size;
}


// file_in_zip_read_info_s contain internal information about a file in zipfile,
//  when reading and decompress it
typedef struct
{
	char  *read_buffer;         // internal buffer for compressed data
	z_stream stream;            // zLib stream structure for inflate

	uLong pos_in_zipfile;       // position in byte on the zipfile, for fseek
	uLong stream_initialised;   // flag set if stream structure is initialised

	uLong offset_local_extrafield;// offset of the local extra field
	uInt  size_local_extrafield;// size of the local extra field
	uLong pos_local_extrafield;   // position in the local extra field in read

	uLong crc32;                // crc32 of all data uncompressed
	uLong crc32_wait;           // crc32 we must obtain after decompress all
	uLong rest_read_compressed; // number of byte to be decompressed
	uLong rest_read_uncompressed;//number of byte to be obtained after decomp
	LUFILE* file;                 // io structore of the zipfile
	uLong compression_method;   // compression method (0==store)
	uLong byte_before_the_zipfile;// byte before the zipfile, (>0 for sfx)
  bool encrypted;               // is it encrypted?
  unsigned long keys[3];        // decryption keys, initialized by unzOpenCurrentFile
  int encheadleft;              // the first call(s) to unzReadCurrentFile will read this many encryption-header bytes first
  char crcenctest;              // if encrypted, we'll check the encryption buffer against this
} file_in_zip_read_info_s;


// unz_s contain internal information about the zipfile
typedef struct
{
	LUFILE* file;               // io structore of the zipfile
	unz_global_info gi;         // public global information
	uLong byte_before_the_zipfile;// byte before the zipfile, (>0 for sfx)
	uLong num_file;             // number of the current file in the zipfile
	uLong pos_in_central_dir;   // pos of the current file in the central dir
	uLong current_file_ok;      // flag about the usability of the current file
	uLong central_pos;          // position of the beginning of the central dir

	uLong size_central_dir;     // size of the central directory
	uLong offset_central_dir;   // offset of start of central directory with respect to the starting disk number

	unz_file_info cur_file_info; // public info about the current file in zip
	unz_file_info_internal cur_file_info_internal; // private info about it
    file_in_zip_read_info_s* pfile_in_zip_read; // structure about the current file if we are decompressing it
} unz_s, *unzFile;


int unzStringFileNameCompare (const char* fileName1,const char* fileName2,int iCaseSensitivity);
//   Compare two filename (fileName1,fileName2).

z_off_t unztell (unzFile file);
//  Give the current position in uncompressed data

int unzeof (unzFile file);
//  return 1 if the end of file was reached, 0 elsewhere

int unzGetLocalExtrafield (unzFile file, voidp buf, unsigned len);
//  Read extra field from the current file (opened by unzOpenCurrentFile)
//  This is the local-header version of the extra field (sometimes, there is
//    more info in the local-header version than in the central-header)
//
//  if buf==NULL, it return the size of the local extra field
//
//  if buf!=NULL, len is the size of the buffer, the extra header is copied in
//	buf.
//  the return value is the number of bytes copied in buf, or (if <0)
//	the error code



// ===========================================================================
//   Read a byte from a gz_stream; update next_in and avail_in. Return EOF
// for end of file.
// IN assertion: the stream s has been sucessfully opened for reading.

int unzlocal_getByte(LUFILE *fin,int *pi)
{ unsigned char c;
  int err = (int)lufread(&c, 1, 1, fin);
  if (err==1)
  { *pi = (int)c;
    return UNZ_OK;
  }
  else
  { if (luferror(fin)) return UNZ_ERRNO;
    else return UNZ_EOF;
  }
}


// ===========================================================================
// Reads a long in LSB order from the given gz_stream. Sets
int unzlocal_getShort (LUFILE *fin,uLong *pX)
{
    uLong x ;
    int i;
    int err;

    err = unzlocal_getByte(fin,&i);
    x = (uLong)i;

    if (err==UNZ_OK)
        err = unzlocal_getByte(fin,&i);
    x += ((uLong)i)<<8;

    if (err==UNZ_OK)
        *pX = x;
    else
        *pX = 0;
    return err;
}

int unzlocal_getLong (LUFILE *fin,uLong *pX)
{
    uLong x ;
    int i;
    int err;

    err = unzlocal_getByte(fin,&i);
    x = (uLong)i;

    if (err==UNZ_OK)
        err = unzlocal_getByte(fin,&i);
    x += ((uLong)i)<<8;

    if (err==UNZ_OK)
        err = unzlocal_getByte(fin,&i);
    x += ((uLong)i)<<16;

    if (err==UNZ_OK)
        err = unzlocal_getByte(fin,&i);
    x += ((uLong)i)<<24;

    if (err==UNZ_OK)
        *pX = x;
    else
        *pX = 0;
    return err;
}


// My own strcmpi / strcasecmp
int strcmpcasenosensitive_internal (const char* fileName1,const char *fileName2)
{
	for (;;)
	{
		char c1=*(fileName1++);
		char c2=*(fileName2++);
		if ((c1>='a') && (c1<='z'))
			c1 -= (char)0x20;
		if ((c2>='a') && (c2<='z'))
			c2 -= (char)0x20;
		if (c1=='\0')
			return ((c2=='\0') ? 0 : -1);
		if (c2=='\0')
			return 1;
		if (c1<c2)
			return -1;
		if (c1>c2)
			return 1;
	}
}


//
// Compare two filename (fileName1,fileName2).
// If iCaseSenisivity = 1, comparision is case sensitivity (like strcmp)
// If iCaseSenisivity = 2, comparision is not case sensitivity (like strcmpi or strcasecmp)
//
int unzStringFileNameCompare (const char*fileName1,const char*fileName2,int iCaseSensitivity)
{ if (iCaseSensitivity==1) return strcmp(fileName1,fileName2);
  else return strcmpcasenosensitive_internal(fileName1,fileName2);
}

#define BUFREADCOMMENT (0x400)


//  Locate the Central directory of a zipfile (at the end, just before
// the global comment). Lu bugfix 2005.07.26 - returns 0xFFFFFFFF if not found,
// rather than 0, since 0 is a valid central-dir-location for an empty zipfile.
uLong unzlocal_SearchCentralDir(LUFILE *fin)
{ if (lufseek(fin,0,SEEK_END) != 0) return 0xFFFFFFFF;
  uLong uSizeFile = luftell(fin);

  uLong uMaxBack=0xffff; // maximum size of global comment
  if (uMaxBack>uSizeFile) uMaxBack = uSizeFile;

  unsigned char *buf = (unsigned char*)zmalloc(BUFREADCOMMENT+4);
  if (buf==NULL) return 0xFFFFFFFF;
  uLong uPosFound=0xFFFFFFFF;

  uLong uBackRead = 4;
  while (uBackRead<uMaxBack)
  { uLong uReadSize,uReadPos ;
    int i;
    if (uBackRead+BUFREADCOMMENT>uMaxBack) uBackRead = uMaxBack;
    else uBackRead+=BUFREADCOMMENT;
    uReadPos = uSizeFile-uBackRead ;
    uReadSize = ((BUFREADCOMMENT+4) < (uSizeFile-uReadPos)) ? (BUFREADCOMMENT+4) : (uSizeFile-uReadPos);
    if (lufseek(fin,uReadPos,SEEK_SET)!=0) break;
    if (lufread(buf,(uInt)uReadSize,1,fin)!=1) break;
    for (i=(int)uReadSize-3; (i--)>=0;)
    { if (((*(buf+i))==0x50) && ((*(buf+i+1))==0x4b) &&	((*(buf+i+2))==0x05) && ((*(buf+i+3))==0x06))
      { uPosFound = uReadPos+i;	break;
      }
    }
    if (uPosFound!=0) break;
  }
  if (buf) zfree(buf);
  return uPosFound;
}


int unzGoToFirstFile (unzFile file);
int unzCloseCurrentFile (unzFile file);

// Open a Zip file.
// If the zipfile cannot be opened (file don't exist or in not valid), return NULL.
// Otherwise, the return value is a unzFile Handle, usable with other unzip functions
unzFile unzOpenInternal(LUFILE *fin)
{ if (fin==NULL) return NULL;
  if (unz_copyright[0]!=' ') {lufclose(fin); return NULL;}

  int err=UNZ_OK;
  unz_s us;
  memset(&us, 0, sizeof(us));
  uLong central_pos=0,uL=0;
  central_pos = unzlocal_SearchCentralDir(fin);
  if (central_pos==0xFFFFFFFF) err=UNZ_ERRNO;
  if (err==UNZ_OK && lufseek(fin,central_pos,SEEK_SET)!=0) err=UNZ_ERRNO;
  // the signature, already checked
  if (err==UNZ_OK && unzlocal_getLong(fin,&uL)!=UNZ_OK) err=UNZ_ERRNO;
  // number of this disk
  uLong number_disk=0;          // number of the current dist, used for spanning ZIP, unsupported, always 0
  if (err==UNZ_OK && unzlocal_getShort(fin,&number_disk)!=UNZ_OK) err=UNZ_ERRNO;
  // number of the disk with the start of the central directory
  uLong number_disk_with_CD=0;  // number the the disk with central dir, used for spaning ZIP, unsupported, always 0
  if (err==UNZ_OK && unzlocal_getShort(fin,&number_disk_with_CD)!=UNZ_OK) err=UNZ_ERRNO;
  // total number of entries in the central dir on this disk
  if (err==UNZ_OK && unzlocal_getShort(fin,&us.gi.number_entry)!=UNZ_OK) err=UNZ_ERRNO;
  // total number of entries in the central dir
  uLong number_entry_CD=0;      // total number of entries in the central dir (same than number_entry on nospan)
  if (err==UNZ_OK && unzlocal_getShort(fin,&number_entry_CD)!=UNZ_OK) err=UNZ_ERRNO;
  if (err==UNZ_OK && ((number_entry_CD!=us.gi.number_entry) || (number_disk_with_CD!=0) || (number_disk!=0))) err=UNZ_BADZIPFILE;
  // size of the central directory
  if (err==UNZ_OK && unzlocal_getLong(fin,&us.size_central_dir)!=UNZ_OK) err=UNZ_ERRNO;
  // offset of start of central directory with respect to the starting disk number
  if (err==UNZ_OK && unzlocal_getLong(fin,&us.offset_central_dir)!=UNZ_OK) err=UNZ_ERRNO;
  // zipfile comment length
  if (err==UNZ_OK && unzlocal_getShort(fin,&us.gi.size_comment)!=UNZ_OK) err=UNZ_ERRNO;
  if (err==UNZ_OK && ((central_pos+fin->initial_offset<us.offset_central_dir+us.size_central_dir) && (err==UNZ_OK))) err=UNZ_BADZIPFILE;
  if (err!=UNZ_OK) {lufclose(fin);return NULL;}

  us.file=fin;
  us.byte_before_the_zipfile = central_pos+fin->initial_offset - (us.offset_central_dir+us.size_central_dir);
  us.central_pos = central_pos;
  us.pfile_in_zip_read = NULL;
  fin->initial_offset = 0; // since the zipfile itself is expected to handle this

  unz_s *s = (unz_s*)zmalloc(sizeof(unz_s));
  if(!s) return NULL;
  *s=us;
  unzGoToFirstFile((unzFile)s);
  return (unzFile)s;
}


//  Close a ZipFile opened with unzipOpen.
//  If there is files inside the .Zip opened with unzipOpenCurrentFile (see later),
//    these files MUST be closed with unzipCloseCurrentFile before call unzipClose.
//  return UNZ_OK if there is no problem.
int unzClose (unzFile file)
{
	unz_s* s;
	if (file==NULL)
		return UNZ_PARAMERROR;
	s=(unz_s*)file;

    if (s->pfile_in_zip_read!=NULL)
        unzCloseCurrentFile(file);

	lufclose(s->file);
	if (s) zfree(s); // unused s=0;
	return UNZ_OK;
}


//  Write info about the ZipFile in the *pglobal_info structure.
//  No preparation of the structure is needed
//  return UNZ_OK if there is no problem.
int unzGetGlobalInfo (unzFile file,unz_global_info *pglobal_info)
{
	unz_s* s;
	if (file==NULL)
		return UNZ_PARAMERROR;
	s=(unz_s*)file;
	*pglobal_info=s->gi;
	return UNZ_OK;
}


//   Translate date/time from Dos format to tm_unz (readable more easilty)
void unzlocal_DosDateToTmuDate (uLong ulDosDate, tm_unz* ptm)
{
    uLong uDate;
    uDate = (uLong)(ulDosDate>>16);
    ptm->tm_mday = (uInt)(uDate&0x1f) ;
    ptm->tm_mon =  (uInt)((((uDate)&0x1E0)/0x20)-1) ;
    ptm->tm_year = (uInt)(((uDate&0x0FE00)/0x0200)+1980) ;

    ptm->tm_hour = (uInt) ((ulDosDate &0xF800)/0x800);
    ptm->tm_min =  (uInt) ((ulDosDate&0x7E0)/0x20) ;
    ptm->tm_sec =  (uInt) (2*(ulDosDate&0x1f)) ;
}

//  Get Info about the current file in the zipfile, with internal only info
int unzlocal_GetCurrentFileInfoInternal (
	unzFile file,
    unz_file_info *pfile_info,
    unz_file_info_internal
    *pfile_info_internal,
    char *szFileName,
	uLong fileNameBufferSize,
    void *extraField,
	uLong extraFieldBufferSize,
    char *szComment,
	uLong commentBufferSize);

int unzlocal_GetCurrentFileInfoInternal (unzFile file, unz_file_info *pfile_info,
   unz_file_info_internal *pfile_info_internal, char *szFileName,
   uLong fileNameBufferSize, void *extraField, uLong extraFieldBufferSize,
   char *szComment, uLong commentBufferSize)
{
	unz_s* s;
	unz_file_info file_info;
	unz_file_info_internal file_info_internal;
	int err=UNZ_OK;
	uLong uMagic;
	long lSeek=0;

	if (file==NULL)
		return UNZ_PARAMERROR;
	s=(unz_s*)file;
	if (lufseek(s->file,s->pos_in_central_dir+s->byte_before_the_zipfile,SEEK_SET)!=0)
		err=UNZ_ERRNO;


	// we check the magic
	if (err==UNZ_OK)
		if (unzlocal_getLong(s->file,&uMagic) != UNZ_OK)
			err=UNZ_ERRNO;
		else if (uMagic!=0x02014b50)
			err=UNZ_BADZIPFILE;

	if (unzlocal_getShort(s->file,&file_info.version) != UNZ_OK)
		err=UNZ_ERRNO;

	if (unzlocal_getShort(s->file,&file_info.version_needed) != UNZ_OK)
		err=UNZ_ERRNO;

	if (unzlocal_getShort(s->file,&file_info.flag) != UNZ_OK)
		err=UNZ_ERRNO;

	if (unzlocal_getShort(s->file,&file_info.compression_method) != UNZ_OK)
		err=UNZ_ERRNO;

	if (unzlocal_getLong(s->file,&file_info.dosDate) != UNZ_OK)
		err=UNZ_ERRNO;

    unzlocal_DosDateToTmuDate(file_info.dosDate,&file_info.tmu_date);

	if (unzlocal_getLong(s->file,&file_info.crc) != UNZ_OK)
		err=UNZ_ERRNO;

	if (unzlocal_getLong(s->file,&file_info.compressed_size) != UNZ_OK)
		err=UNZ_ERRNO;

	if (unzlocal_getLong(s->file,&file_info.uncompressed_size) != UNZ_OK)
		err=UNZ_ERRNO;

	if (unzlocal_getShort(s->file,&file_info.size_filename) != UNZ_OK)
		err=UNZ_ERRNO;

	if (unzlocal_getShort(s->file,&file_info.size_file_extra) != UNZ_OK)
		err=UNZ_ERRNO;

	if (unzlocal_getShort(s->file,&file_info.size_file_comment) != UNZ_OK)
		err=UNZ_ERRNO;

	if (unzlocal_getShort(s->file,&file_info.disk_num_start) != UNZ_OK)
		err=UNZ_ERRNO;

	if (unzlocal_getShort(s->file,&file_info.internal_fa) != UNZ_OK)
		err=UNZ_ERRNO;

	if (unzlocal_getLong(s->file,&file_info.external_fa) != UNZ_OK)
		err=UNZ_ERRNO;

	if (unzlocal_getLong(s->file,&file_info_internal.offset_curfile) != UNZ_OK)
		err=UNZ_ERRNO;

	lSeek+=file_info.size_filename;
	if ((err==UNZ_OK) && (szFileName!=NULL))
	{
		uLong uSizeRead ;
		if (file_info.size_filename<fileNameBufferSize)
		{
			*(szFileName+file_info.size_filename)='\0';
			uSizeRead = file_info.size_filename;
		}
		else
			uSizeRead = fileNameBufferSize;

		if ((file_info.size_filename>0) && (fileNameBufferSize>0))
			if (lufread(szFileName,(uInt)uSizeRead,1,s->file)!=1)
				err=UNZ_ERRNO;
		lSeek -= uSizeRead;
	}


	if ((err==UNZ_OK) && (extraField!=NULL))
	{
		uLong uSizeRead ;
		if (file_info.size_file_extra<extraFieldBufferSize)
			uSizeRead = file_info.size_file_extra;
		else
			uSizeRead = extraFieldBufferSize;

		if (lSeek!=0)
			if (lufseek(s->file,lSeek,SEEK_CUR)==0)
				lSeek=0;
			else
				err=UNZ_ERRNO;
		if ((file_info.size_file_extra>0) && (extraFieldBufferSize>0))
			if (lufread(extraField,(uInt)uSizeRead,1,s->file)!=1)
				err=UNZ_ERRNO;
		lSeek += file_info.size_file_extra - uSizeRead;
	}
	else
		lSeek+=file_info.size_file_extra;


	if ((err==UNZ_OK) && (szComment!=NULL))
	{
		uLong uSizeRead ;
		if (file_info.size_file_comment<commentBufferSize)
		{
			*(szComment+file_info.size_file_comment)='\0';
			uSizeRead = file_info.size_file_comment;
		}
		else
			uSizeRead = commentBufferSize;

		if (lSeek!=0)
			if (lufseek(s->file,lSeek,SEEK_CUR)==0)
				{} // unused lSeek=0;
			else
				err=UNZ_ERRNO;
		if ((file_info.size_file_comment>0) && (commentBufferSize>0))
			if (lufread(szComment,(uInt)uSizeRead,1,s->file)!=1)
				err=UNZ_ERRNO;
		//unused lSeek+=file_info.size_file_comment - uSizeRead;
	}
	else {} //unused lSeek+=file_info.size_file_comment;

	if ((err==UNZ_OK) && (pfile_info!=NULL))
		*pfile_info=file_info;

	if ((err==UNZ_OK) && (pfile_info_internal!=NULL))
		*pfile_info_internal=file_info_internal;

	return err;
}


//  Write info about the ZipFile in the *pglobal_info structure.
//  No preparation of the structure is needed
//  return UNZ_OK if there is no problem.
int unzGetCurrentFileInfo (unzFile file, unz_file_info *pfile_info,
  char *szFileName, uLong fileNameBufferSize, void *extraField, uLong extraFieldBufferSize,
  char *szComment, uLong commentBufferSize)
{ return unzlocal_GetCurrentFileInfoInternal(file,pfile_info,NULL,szFileName,fileNameBufferSize,
      extraField,extraFieldBufferSize, szComment,commentBufferSize);
}


//  Set the current file of the zipfile to the first file.
//  return UNZ_OK if there is no problem
int unzGoToFirstFile (unzFile file)
{
	int err;
	unz_s* s;
	if (file==NULL) return UNZ_PARAMERROR;
	s=(unz_s*)file;
	s->pos_in_central_dir=s->offset_central_dir;
	s->num_file=0;
	err=unzlocal_GetCurrentFileInfoInternal(file,&s->cur_file_info,
											 &s->cur_file_info_internal,
											 NULL,0,NULL,0,NULL,0);
	s->current_file_ok = (err == UNZ_OK);
	return err;
}


//  Set the current file of the zipfile to the next file.
//  return UNZ_OK if there is no problem
//  return UNZ_END_OF_LIST_OF_FILE if the actual file was the latest.
int unzGoToNextFile (unzFile file)
{
	unz_s* s;
	int err;

	if (file==NULL)
		return UNZ_PARAMERROR;
	s=(unz_s*)file;
	if (!s->current_file_ok)
		return UNZ_END_OF_LIST_OF_FILE;
	if (s->num_file+1==s->gi.number_entry)
		return UNZ_END_OF_LIST_OF_FILE;

	s->pos_in_central_dir += SIZECENTRALDIRITEM + s->cur_file_info.size_filename +
			s->cur_file_info.size_file_extra + s->cur_file_info.size_file_comment ;
	s->num_file++;
	err = unzlocal_GetCurrentFileInfoInternal(file,&s->cur_file_info,
											   &s->cur_file_info_internal,
											   NULL,0,NULL,0,NULL,0);
	s->current_file_ok = (err == UNZ_OK);
	return err;
}


//  Try locate the file szFileName in the zipfile.
//  For the iCaseSensitivity signification, see unzStringFileNameCompare
//  return value :
//  UNZ_OK if the file is found. It becomes the current file.
//  UNZ_END_OF_LIST_OF_FILE if the file is not found
int unzLocateFile (unzFile file, const char *szFileName, int iCaseSensitivity)
{
	unz_s* s;
	int err;


	uLong num_fileSaved;
	uLong pos_in_central_dirSaved;


	if (file==NULL)
		return UNZ_PARAMERROR;

    if (strlen(szFileName)>=UNZ_MAXFILENAMEINZIP)
        return UNZ_PARAMERROR;

	s=(unz_s*)file;
	if (!s->current_file_ok)
		return UNZ_END_OF_LIST_OF_FILE;

	num_fileSaved = s->num_file;
	pos_in_central_dirSaved = s->pos_in_central_dir;

	err = unzGoToFirstFile(file);

	while (err == UNZ_OK)
	{
		char szCurrentFileName[UNZ_MAXFILENAMEINZIP+1];
		unzGetCurrentFileInfo(file,NULL,
								szCurrentFileName,sizeof(szCurrentFileName)-1,
								NULL,0,NULL,0);
		if (unzStringFileNameCompare(szCurrentFileName,szFileName,iCaseSensitivity)==0)
			return UNZ_OK;
		err = unzGoToNextFile(file);
	}

	s->num_file = num_fileSaved ;
	s->pos_in_central_dir = pos_in_central_dirSaved ;
	return err;
}


//  Read the local header of the current zipfile
//  Check the coherency of the local header and info in the end of central
//        directory about this file
//  store in *piSizeVar the size of extra info in local header
//        (filename and size of extra field data)
int unzlocal_CheckCurrentFileCoherencyHeader (unz_s *s,uInt *piSizeVar,
  uLong *poffset_local_extrafield, uInt  *psize_local_extrafield)
{
	uLong uMagic,uData,uFlags;
	uLong size_filename;
	uLong size_extra_field;
	int err=UNZ_OK;

	*piSizeVar = 0;
	*poffset_local_extrafield = 0;
	*psize_local_extrafield = 0;

	if (lufseek(s->file,s->cur_file_info_internal.offset_curfile + s->byte_before_the_zipfile,SEEK_SET)!=0)
		return UNZ_ERRNO;


	if (err==UNZ_OK)
		if (unzlocal_getLong(s->file,&uMagic) != UNZ_OK)
			err=UNZ_ERRNO;
		else if (uMagic!=0x04034b50)
			err=UNZ_BADZIPFILE;

	if (unzlocal_getShort(s->file,&uData) != UNZ_OK)
		err=UNZ_ERRNO;
//	else if ((err==UNZ_OK) && (uData!=s->cur_file_info.wVersion))
//		err=UNZ_BADZIPFILE;
	if (unzlocal_getShort(s->file,&uFlags) != UNZ_OK)
		err=UNZ_ERRNO;

	if (unzlocal_getShort(s->file,&uData) != UNZ_OK)
		err=UNZ_ERRNO;
	else if ((err==UNZ_OK) && (uData!=s->cur_file_info.compression_method))
		err=UNZ_BADZIPFILE;

    if ((err==UNZ_OK) && (s->cur_file_info.compression_method!=0) &&
                         (s->cur_file_info.compression_method!=Z_DEFLATED))
        err=UNZ_BADZIPFILE;

	if (unzlocal_getLong(s->file,&uData) != UNZ_OK) // date/time
		err=UNZ_ERRNO;

	if (unzlocal_getLong(s->file,&uData) != UNZ_OK) // crc
		err=UNZ_ERRNO;
	else if ((err==UNZ_OK) && (uData!=s->cur_file_info.crc) &&
		                      ((uFlags & 8)==0))
		err=UNZ_BADZIPFILE;

	if (unzlocal_getLong(s->file,&uData) != UNZ_OK) // size compr
		err=UNZ_ERRNO;
	else if ((err==UNZ_OK) && (uData!=s->cur_file_info.compressed_size) &&
							  ((uFlags & 8)==0))
		err=UNZ_BADZIPFILE;

	if (unzlocal_getLong(s->file,&uData) != UNZ_OK) // size uncompr
		err=UNZ_ERRNO;
	else if ((err==UNZ_OK) && (uData!=s->cur_file_info.uncompressed_size) &&
							  ((uFlags & 8)==0))
		err=UNZ_BADZIPFILE;


	if (unzlocal_getShort(s->file,&size_filename) != UNZ_OK)
		err=UNZ_ERRNO;
	else if ((err==UNZ_OK) && (size_filename!=s->cur_file_info.size_filename))
		err=UNZ_BADZIPFILE;

	*piSizeVar += (uInt)size_filename;

	if (unzlocal_getShort(s->file,&size_extra_field) != UNZ_OK)
		err=UNZ_ERRNO;
	*poffset_local_extrafield= s->cur_file_info_internal.offset_curfile +
									SIZEZIPLOCALHEADER + size_filename;
	*psize_local_extrafield = (uInt)size_extra_field;

	*piSizeVar += (uInt)size_extra_field;

	return err;
}


//  Open for reading data the current file in the zipfile.
//  If there is no error and the file is opened, the return value is UNZ_OK.
int unzOpenCurrentFile (unzFile file, const char *password)
{
	int err;
	int Store;
	uInt iSizeVar;
	unz_s* s;
	file_in_zip_read_info_s* pfile_in_zip_read_info;
	uLong offset_local_extrafield;  // offset of the local extra field
	uInt  size_local_extrafield;    // size of the local extra field

	if (file==NULL)
		return UNZ_PARAMERROR;
	s=(unz_s*)file;
	if (!s->current_file_ok)
		return UNZ_PARAMERROR;

    if (s->pfile_in_zip_read != NULL)
        unzCloseCurrentFile(file);

	if (unzlocal_CheckCurrentFileCoherencyHeader(s,&iSizeVar,
				&offset_local_extrafield,&size_local_extrafield)!=UNZ_OK)
		return UNZ_BADZIPFILE;

	pfile_in_zip_read_info = (file_in_zip_read_info_s*)zmalloc(sizeof(file_in_zip_read_info_s));
	if (pfile_in_zip_read_info==NULL)
		return UNZ_INTERNALERROR;

	pfile_in_zip_read_info->read_buffer=(char*)zmalloc(UNZ_BUFSIZE);
	pfile_in_zip_read_info->offset_local_extrafield = offset_local_extrafield;
	pfile_in_zip_read_info->size_local_extrafield = size_local_extrafield;
	pfile_in_zip_read_info->pos_local_extrafield=0;

	if (pfile_in_zip_read_info->read_buffer==NULL)
	{
		if (pfile_in_zip_read_info!=0) zfree(pfile_in_zip_read_info); //unused pfile_in_zip_read_info=0;
		return UNZ_INTERNALERROR;
	}

	pfile_in_zip_read_info->stream_initialised=0;

	if ((s->cur_file_info.compression_method!=0) && (s->cur_file_info.compression_method!=Z_DEFLATED))
        { // unused err=UNZ_BADZIPFILE;
        }
	Store = s->cur_file_info.compression_method==0;

	pfile_in_zip_read_info->crc32_wait=s->cur_file_info.crc;
	pfile_in_zip_read_info->crc32=0;
	pfile_in_zip_read_info->compression_method = s->cur_file_info.compression_method;
	pfile_in_zip_read_info->file=s->file;
	pfile_in_zip_read_info->byte_before_the_zipfile=s->byte_before_the_zipfile;

    pfile_in_zip_read_info->stream.total_out = 0;

	if (!Store)
	{
	  pfile_in_zip_read_info->stream.zalloc = (alloc_func)0;
	  pfile_in_zip_read_info->stream.zfree = (free_func)0;
	  pfile_in_zip_read_info->stream.opaque = (voidpf)0;

          err=inflateInit2(&pfile_in_zip_read_info->stream, -MAX_WBITS/*ricky*/);
	  if (err == Z_OK)
	    pfile_in_zip_read_info->stream_initialised=1;
        // windowBits is passed < 0 to tell that there is no zlib header.
        // Note that in this case inflate *requires* an extra "dummy" byte
        // after the compressed stream in order to complete decompression and
        // return Z_STREAM_END.
        // In unzip, i don't wait absolutely Z_STREAM_END because I known the
        // size of both compressed and uncompressed data
	}
	pfile_in_zip_read_info->rest_read_compressed = s->cur_file_info.compressed_size ;
	pfile_in_zip_read_info->rest_read_uncompressed = s->cur_file_info.uncompressed_size ;
  pfile_in_zip_read_info->encrypted = (s->cur_file_info.flag&1)!=0;
  bool extlochead = (s->cur_file_info.flag&8)!=0;
  if (extlochead) pfile_in_zip_read_info->crcenctest = (char)((s->cur_file_info.dosDate>>8)&0xff);
  else pfile_in_zip_read_info->crcenctest = (char)(s->cur_file_info.crc >> 24);
  pfile_in_zip_read_info->encheadleft = (pfile_in_zip_read_info->encrypted?12:0);
  pfile_in_zip_read_info->keys[0] = 305419896L;
  pfile_in_zip_read_info->keys[1] = 591751049L;
  pfile_in_zip_read_info->keys[2] = 878082192L;
  for (const char *cp=password; cp!=0 && *cp!=0; cp++) Uupdate_keys(pfile_in_zip_read_info->keys,*cp);

	pfile_in_zip_read_info->pos_in_zipfile =
            s->cur_file_info_internal.offset_curfile + SIZEZIPLOCALHEADER +
			  iSizeVar;

	pfile_in_zip_read_info->stream.avail_in = (uInt)0;

	s->pfile_in_zip_read = pfile_in_zip_read_info;

  return UNZ_OK;
}


//  Read bytes from the current file.
//  buf contain buffer where data must be copied
//  len the size of buf.
//  return the number of byte copied if somes bytes are copied (and also sets *reached_eof)
//  return 0 if the end of file was reached. (and also sets *reached_eof).
//  return <0 with error code if there is an error. (in which case *reached_eof is meaningless)
//    (UNZ_ERRNO for IO error, or zLib error for uncompress error)
int unzReadCurrentFile  (unzFile file, voidp buf, unsigned len, bool *reached_eof)
{ int err=UNZ_OK;
  uInt iRead = 0;
  if (reached_eof!=0) *reached_eof=false;

  unz_s *s = (unz_s*)file;
  if (s==NULL) return UNZ_PARAMERROR;

  file_in_zip_read_info_s* pfile_in_zip_read_info = s->pfile_in_zip_read;
  if (pfile_in_zip_read_info==NULL) return UNZ_PARAMERROR;
  if ((pfile_in_zip_read_info->read_buffer == NULL)) return UNZ_END_OF_LIST_OF_FILE;
  if (len==0) return 0;

  pfile_in_zip_read_info->stream.next_out = (Byte*)buf;
  pfile_in_zip_read_info->stream.avail_out = (uInt)len;

  if (len>pfile_in_zip_read_info->rest_read_uncompressed)
  { pfile_in_zip_read_info->stream.avail_out = (uInt)pfile_in_zip_read_info->rest_read_uncompressed;
  }

  while (pfile_in_zip_read_info->stream.avail_out>0)
  { if ((pfile_in_zip_read_info->stream.avail_in==0) && (pfile_in_zip_read_info->rest_read_compressed>0))
    { uInt uReadThis = UNZ_BUFSIZE;
      if (pfile_in_zip_read_info->rest_read_compressed<uReadThis) uReadThis = (uInt)pfile_in_zip_read_info->rest_read_compressed;
      if (uReadThis == 0) {if (reached_eof!=0) *reached_eof=true; return UNZ_EOF;}
      if (lufseek(pfile_in_zip_read_info->file, pfile_in_zip_read_info->pos_in_zipfile + pfile_in_zip_read_info->byte_before_the_zipfile,SEEK_SET)!=0) return UNZ_ERRNO;
      if (lufread(pfile_in_zip_read_info->read_buffer,uReadThis,1,pfile_in_zip_read_info->file)!=1) return UNZ_ERRNO;
      pfile_in_zip_read_info->pos_in_zipfile += uReadThis;
      pfile_in_zip_read_info->rest_read_compressed-=uReadThis;
      pfile_in_zip_read_info->stream.next_in = (Byte*)pfile_in_zip_read_info->read_buffer;
      pfile_in_zip_read_info->stream.avail_in = (uInt)uReadThis;
      //
      if (pfile_in_zip_read_info->encrypted)
      { char *buf2 = (char*)pfile_in_zip_read_info->stream.next_in;
        for (unsigned int i=0; i<uReadThis; i++) buf2[i]=zdecode(pfile_in_zip_read_info->keys,buf2[i]);
      }
    }

    unsigned int uDoEncHead = pfile_in_zip_read_info->encheadleft;
    if (uDoEncHead>pfile_in_zip_read_info->stream.avail_in) uDoEncHead=pfile_in_zip_read_info->stream.avail_in;
    if (uDoEncHead>0)
    { char bufcrc=pfile_in_zip_read_info->stream.next_in[uDoEncHead-1];
//      pfile_in_zip_read_info->rest_read_uncompressed-=uDoEncHead;	// Commented out by Ricky Lung. Reference:
      pfile_in_zip_read_info->stream.avail_in -= uDoEncHead;
      pfile_in_zip_read_info->stream.next_in += uDoEncHead;
      pfile_in_zip_read_info->encheadleft -= uDoEncHead;
      if (pfile_in_zip_read_info->encheadleft==0)
      { if (bufcrc!=pfile_in_zip_read_info->crcenctest) return UNZ_PASSWORD;
      }
    }

    if (pfile_in_zip_read_info->compression_method==0)
    { uInt uDoCopy,i ;
      if (pfile_in_zip_read_info->stream.avail_out < pfile_in_zip_read_info->stream.avail_in)
      { uDoCopy = pfile_in_zip_read_info->stream.avail_out ;
      }
      else
      { uDoCopy = pfile_in_zip_read_info->stream.avail_in ;
      }
      for (i=0;i<uDoCopy;i++) *(pfile_in_zip_read_info->stream.next_out+i) = *(pfile_in_zip_read_info->stream.next_in+i);
      pfile_in_zip_read_info->crc32 = crc32(pfile_in_zip_read_info->crc32,pfile_in_zip_read_info->stream.next_out,uDoCopy);
      pfile_in_zip_read_info->rest_read_uncompressed-=uDoCopy;
      pfile_in_zip_read_info->stream.avail_in -= uDoCopy;
      pfile_in_zip_read_info->stream.avail_out -= uDoCopy;
      pfile_in_zip_read_info->stream.next_out += uDoCopy;
      pfile_in_zip_read_info->stream.next_in += uDoCopy;
      pfile_in_zip_read_info->stream.total_out += uDoCopy;
      iRead += uDoCopy;
      if (pfile_in_zip_read_info->rest_read_uncompressed==0) {if (reached_eof!=0) *reached_eof=true;}
    }
    else
    { uLong uTotalOutBefore,uTotalOutAfter;
      const Byte *bufBefore;
      uLong uOutThis;
      int flush=Z_SYNC_FLUSH;
      uTotalOutBefore = pfile_in_zip_read_info->stream.total_out;
      bufBefore = pfile_in_zip_read_info->stream.next_out;
      //
      err=inflate(&pfile_in_zip_read_info->stream,flush);
      //
      uTotalOutAfter = pfile_in_zip_read_info->stream.total_out;
      uOutThis = uTotalOutAfter-uTotalOutBefore;
      pfile_in_zip_read_info->crc32 = crc32(pfile_in_zip_read_info->crc32,bufBefore,(uInt)(uOutThis));
      pfile_in_zip_read_info->rest_read_uncompressed -= uOutThis;
      iRead += (uInt)(uTotalOutAfter - uTotalOutBefore);
      if (err==Z_STREAM_END || pfile_in_zip_read_info->rest_read_uncompressed==0)
      { if (reached_eof!=0) *reached_eof=true;
        return iRead;
      }
      if (err!=Z_OK) break;
    }
  }

  if (err==Z_OK) return iRead;
  return err;
}


//  Give the current position in uncompressed data
z_off_t unztell (unzFile file)
{
	unz_s* s;
	file_in_zip_read_info_s* pfile_in_zip_read_info;
	if (file==NULL)
		return UNZ_PARAMERROR;
	s=(unz_s*)file;
    pfile_in_zip_read_info=s->pfile_in_zip_read;

	if (pfile_in_zip_read_info==NULL)
		return UNZ_PARAMERROR;

	return (z_off_t)pfile_in_zip_read_info->stream.total_out;
}


//  return 1 if the end of file was reached, 0 elsewhere
int unzeof (unzFile file)
{
	unz_s* s;
	file_in_zip_read_info_s* pfile_in_zip_read_info;
	if (file==NULL)
		return UNZ_PARAMERROR;
	s=(unz_s*)file;
    pfile_in_zip_read_info=s->pfile_in_zip_read;

	if (pfile_in_zip_read_info==NULL)
		return UNZ_PARAMERROR;

	if (pfile_in_zip_read_info->rest_read_uncompressed == 0)
		return 1;
	else
		return 0;
}


//  Read extra field from the current file (opened by unzOpenCurrentFile)
//  This is the local-header version of the extra field (sometimes, there is
//    more info in the local-header version than in the central-header)
//  if buf==NULL, it return the size of the local extra field that can be read
//  if buf!=NULL, len is the size of the buffer, the extra header is copied in buf.
//  the return value is the number of bytes copied in buf, or (if <0) the error code
int unzGetLocalExtrafield (unzFile file,voidp buf,unsigned len)
{
	unz_s* s;
	file_in_zip_read_info_s* pfile_in_zip_read_info;
	uInt read_now;
	uLong size_to_read;

	if (file==NULL)
		return UNZ_PARAMERROR;
	s=(unz_s*)file;
    pfile_in_zip_read_info=s->pfile_in_zip_read;

	if (pfile_in_zip_read_info==NULL)
		return UNZ_PARAMERROR;

	size_to_read = (pfile_in_zip_read_info->size_local_extrafield -
				pfile_in_zip_read_info->pos_local_extrafield);

	if (buf==NULL)
		return (int)size_to_read;

	if (len>size_to_read)
		read_now = (uInt)size_to_read;
	else
		read_now = (uInt)len ;

	if (read_now==0)
		return 0;

	if (lufseek(pfile_in_zip_read_info->file, pfile_in_zip_read_info->offset_local_extrafield +  pfile_in_zip_read_info->pos_local_extrafield,SEEK_SET)!=0)
		return UNZ_ERRNO;

	if (lufread(buf,(uInt)size_to_read,1,pfile_in_zip_read_info->file)!=1)
		return UNZ_ERRNO;

	return (int)read_now;
}

//  Close the file in zip opened with unzipOpenCurrentFile
//  Return UNZ_CRCERROR if all the file was read but the CRC is not good
int unzCloseCurrentFile (unzFile file)
{
	int err=UNZ_OK;

	unz_s* s;
	file_in_zip_read_info_s* pfile_in_zip_read_info;
	if (file==NULL)
		return UNZ_PARAMERROR;
	s=(unz_s*)file;
    pfile_in_zip_read_info=s->pfile_in_zip_read;

	if (pfile_in_zip_read_info==NULL)
		return UNZ_PARAMERROR;


	if (pfile_in_zip_read_info->rest_read_uncompressed == 0)
	{
		if (pfile_in_zip_read_info->crc32 != pfile_in_zip_read_info->crc32_wait)
			err=UNZ_CRCERROR;
	}


	if (pfile_in_zip_read_info->read_buffer!=0)
        { void *buf = pfile_in_zip_read_info->read_buffer;
          zfree(buf);
          pfile_in_zip_read_info->read_buffer=0;
        }
	pfile_in_zip_read_info->read_buffer = NULL;
	if (pfile_in_zip_read_info->stream_initialised)
		inflateEnd(&pfile_in_zip_read_info->stream);

	pfile_in_zip_read_info->stream_initialised = 0;
        if (pfile_in_zip_read_info!=0) zfree(pfile_in_zip_read_info); // unused pfile_in_zip_read_info=0;

    s->pfile_in_zip_read=NULL;

	return err;
}


//  Get the global comment string of the ZipFile, in the szComment buffer.
//  uSizeBuf is the size of the szComment buffer.
//  return the number of byte copied or an error code <0
int unzGetGlobalComment (unzFile file, char *szComment, uLong uSizeBuf)
{ //int err=UNZ_OK;
  unz_s* s;
  uLong uReadThis ;
  if (file==NULL) return UNZ_PARAMERROR;
  s=(unz_s*)file;
  uReadThis = uSizeBuf;
  if (uReadThis>s->gi.size_comment) uReadThis = s->gi.size_comment;
  if (lufseek(s->file,s->central_pos+22,SEEK_SET)!=0) return UNZ_ERRNO;
  if (uReadThis>0)
  { *szComment='\0';
    if (lufread(szComment,(uInt)uReadThis,1,s->file)!=1) return UNZ_ERRNO;
  }
  if ((szComment != NULL) && (uSizeBuf > s->gi.size_comment)) *(szComment+s->gi.size_comment)='\0';
  return (int)uReadThis;
}


int unzOpenCurrentFile (unzFile file, const char *password);
int unzReadCurrentFile (unzFile file, void *buf, unsigned len);
int unzCloseCurrentFile (unzFile file);


class TUnzip
{ public:
  TUnzip(const char *pwd) : uf(0), currentfile(-1), czei(-1), password(0), unzbuf(0) {if (pwd!=0) {password=new char[strlen(pwd)+1]; strcpy(password,pwd);}}
  ~TUnzip() {if (password!=0) delete[] password; password=0; if (unzbuf!=0) delete[] unzbuf; unzbuf=0;}

  unzFile uf; int currentfile; ZIPENTRY cze; int czei;
  char *password;
  char *unzbuf;            // lazily created and destroyed, used by Unzip
  TCHAR rootdir[MAX_PATH]; // includes a trailing slash

  ZRESULT Open(void *z,unsigned int len,DWORD flags);
  ZRESULT Get(int index,ZIPENTRY *ze);
  ZRESULT Find(const TCHAR *name,bool ic,int *index,ZIPENTRY *ze);
  ZRESULT Unzip(int index,void *dst,unsigned int len,DWORD flags);
  ZRESULT Close();
};


ZRESULT TUnzip::Open(void *z,unsigned int len,DWORD flags)
{ if (uf!=0 || currentfile!=-1) return ZR_NOTINITED;
  //
  if(!getcwd(rootdir,MAX_PATH-1)) return ZR_NOFILE;
  TCHAR *lastchar = &rootdir[_tcslen(rootdir)-1];
  if (*lastchar!='\\' && *lastchar!='/') {lastchar[1]='/'; lastchar[2]=0;}
  //
  if (flags==ZIP_HANDLE)
  { // test if we can seek on it. We can't use GetFileType(h)==FILE_TYPE_DISK since it's not on CE.
    DWORD res = GetFilePosU((HANDLE)z);
    bool canseek = (res!=0xFFFFFFFF);
    if (!canseek) return ZR_SEEK;
  }
  ZRESULT e; LUFILE *f = lufopen(z,len,flags,&e);
  if (f==NULL) return e;
  uf = unzOpenInternal(f);
  if (uf==0) return ZR_NOFILE;
  return ZR_OK;
}

ZRESULT TUnzip::Get(int index,ZIPENTRY *ze)
{ if (index<-1 || index>=(int)uf->gi.number_entry) return ZR_ARGS;
  if (currentfile!=-1) unzCloseCurrentFile(uf); currentfile=-1;
  if (index==czei && index!=-1) {memcpy(ze,&cze,sizeof(ZIPENTRY)); return ZR_OK;}
  if (index==-1)
  { ze->index = uf->gi.number_entry;
    ze->name[0]=0;
    ze->attr=0;
    ze->atime=0;
    ze->ctime=0;
    ze->mtime=0;
    ze->comp_size=0;
    ze->unc_size=0;
    return ZR_OK;
  }
  if (index<(int)uf->num_file) unzGoToFirstFile(uf);
  while ((int)uf->num_file<index) unzGoToNextFile(uf);
  unz_file_info ufi; char fn[MAX_PATH];
  unzGetCurrentFileInfo(uf,&ufi,fn,MAX_PATH,NULL,0,NULL,0);
  // now get the extra header. We do this ourselves, instead of
  // calling unzOpenCurrentFile &c., to avoid allocating more than necessary.
  unsigned int extralen,iSizeVar; unsigned long offset;
  int res = unzlocal_CheckCurrentFileCoherencyHeader(uf,&iSizeVar,&offset,&extralen);
  if (res!=UNZ_OK) return ZR_CORRUPT;
  if (lufseek(uf->file,offset,SEEK_SET)!=0) return ZR_READ;
  unsigned char *extra = new unsigned char[extralen];
  if (lufread(extra,1,(uInt)extralen,uf->file)!=extralen) {delete[] extra; return ZR_READ;}
  //
  ze->index=uf->num_file;
  TCHAR tfn[MAX_PATH];
#ifdef UNICODE
  MultiByteToWideChar(CP_UTF8,0,fn,-1,tfn,MAX_PATH);
#else
  strcpy(tfn,fn);
#endif
  // As a safety feature: if the zip filename had sneaky stuff
  // like "c:\windows\file.txt" or "\windows\file.txt" or "fred\..\..\..\windows\file.txt"
  // then we get rid of them all. That way, when the programmer does UnzipItem(hz,i,ze.name),
  // it won't be a problem. (If the programmer really did want to get the full evil information,
  // then they can edit out this security feature from here).
  // In particular, we chop off any prefixes that are "c:\" or "\" or "/" or "[stuff]\.." or "[stuff]/.."
  const TCHAR *sfn=tfn;
  for (;;)
  { if (sfn[0]!=0 && sfn[1]==':') {sfn+=2; continue;}
    if (sfn[0]=='\\') {sfn++; continue;}
    if (sfn[0]=='/') {sfn++; continue;}
    const TCHAR *c;
    c=_tcsstr(sfn,_T("\\..\\")); if (c!=0) {sfn=c+4; continue;}
    c=_tcsstr(sfn,_T("\\../")); if (c!=0) {sfn=c+4; continue;}
    c=_tcsstr(sfn,_T("/../")); if (c!=0) {sfn=c+4; continue;}
    c=_tcsstr(sfn,_T("/..\\")); if (c!=0) {sfn=c+4; continue;}
    break;
  }
  _tcsncpy(ze->name, sfn,MAX_PATH);


  unsigned long a = ufi.external_fa;
  // zip has an 'attribute' 32bit value. Its lower half is windows stuff
  // its upper half is standard unix stat.st_mode. We'll start trying
  // to read it in unix mode
  bool isdir  =   (a&0x40000000)!=0;
  bool readonly=  (a&0x00800000)==0;
  //bool readable=  (a&0x01000000)!=0; // unused
  //bool executable=(a&0x00400000)!=0; // unused
  bool hidden=false, system=false, archive=true;
  // but in normal hostmodes these are overridden by the lower half...
  int host = ufi.version>>8;
  if (host==0 || host==7 || host==11 || host==14)
  { readonly=  (a&0x00000001)!=0;
    hidden=    (a&0x00000002)!=0;
    system=    (a&0x00000004)!=0;
    isdir=     (a&0x00000010)!=0;
    archive=   (a&0x00000020)!=0;
  }
  ze->attr=0;
  ze->attr = (a&0xFFFF0000)>>16;
  if (isdir) ze->attr |= S_IFDIR;
  if (readonly) ze->attr &= ~S_IWUSR;
  ze->comp_size = ufi.compressed_size;
  ze->unc_size = ufi.uncompressed_size;
  //
  WORD dostime = (WORD)(ufi.dosDate&0xFFFF);
  WORD dosdate = (WORD)((ufi.dosDate>>16)&0xFFFF);
  FILETIME ftd = dosdatetime2filetime(dosdate,dostime);
  FILETIME ft; LocalFileTimeToFileTime(&ftd,&ft);
  ze->atime=ft; ze->ctime=ft; ze->mtime=ft;
  // the zip will always have at least that dostime. But if it also has
  // an extra header, then we'll instead get the info from that.
  unsigned int epos=0;
  while (epos+4<extralen)
  { char etype[3]; etype[0]=extra[epos+0]; etype[1]=extra[epos+1]; etype[2]=0;
    int size = extra[epos+2];
    if (strcmp(etype,"UT")!=0) {epos += 4+size; continue;}
    int flags = extra[epos+4];
    bool hasmtime = (flags&1)!=0;
    bool hasatime = (flags&2)!=0;
    bool hasctime = (flags&4)!=0;
    epos+=5;
    if (hasmtime)
    { lutime_t mtime = ((extra[epos+0])<<0) | ((extra[epos+1])<<8) |((extra[epos+2])<<16) | ((extra[epos+3])<<24);
	  epos+=4;
      ze->mtime = timet2filetime(mtime);
    }
    if (hasatime)
    { lutime_t atime = ((extra[epos+0])<<0) | ((extra[epos+1])<<8) |((extra[epos+2])<<16) | ((extra[epos+3])<<24);
      epos+=4;
      ze->atime = timet2filetime(atime);
    }
    if (hasctime)
    { lutime_t ctime = ((extra[epos+0])<<0) | ((extra[epos+1])<<8) |((extra[epos+2])<<16) | ((extra[epos+3])<<24);
      epos+=4;
      ze->ctime = timet2filetime(ctime);
    }
    break;
  }
  //
  if (extra!=0) delete[] extra;
  memcpy(&cze,ze,sizeof(ZIPENTRY)); czei=index;
  return ZR_OK;
}

ZRESULT TUnzip::Find(const TCHAR *tname,bool ic,int *index,ZIPENTRY *ze)
{
  if(!tname) return ZR_ARGS;
  char name[MAX_PATH];
#ifdef UNICODE
  WideCharToMultiByte(CP_UTF8,0,tname,-1,name,MAX_PATH,0,0);
#else
  strcpy(name,tname);
#endif
  int res = unzLocateFile(uf,name,ic?CASE_INSENSITIVE:CASE_SENSITIVE);
  if (res!=UNZ_OK)
  { if (index!=0) *index=-1;
    if (ze!=NULL) {memset(ze,0,sizeof(ZIPENTRY)); ze->index=-1;}
    return ZR_NOTFOUND;
  }
  if (currentfile!=-1) unzCloseCurrentFile(uf); currentfile=-1;
  int i = (int)uf->num_file;
  if (index!=NULL) *index=i;
  if (ze!=NULL)
  { ZRESULT zres = Get(i,ze);
    if (zres!=ZR_OK) return zres;
  }
  return ZR_OK;
}

ZRESULT TUnzip::Unzip(int index,void *dst,unsigned int len,DWORD flags)
{
  // Ricky: Remove the support for unzipping to file
  if (flags==ZIP_MEMORY)
  { if (index!=currentfile)
    { if (currentfile!=-1) unzCloseCurrentFile(uf); currentfile=-1;
      if (index>=(int)uf->gi.number_entry) return ZR_ARGS;
      if (index<(int)uf->num_file) unzGoToFirstFile(uf);
      while ((int)uf->num_file<index) unzGoToNextFile(uf);
      unzOpenCurrentFile(uf,password); currentfile=index;
    }
    bool reached_eof;
    int res = unzReadCurrentFile(uf,dst,len,&reached_eof);
    if (res<=0) {unzCloseCurrentFile(uf); currentfile=-1;}
    if (reached_eof) return ZR_OK;
    if (res>0) return ZR_MORE;
    if (res==UNZ_PASSWORD) return ZR_PASSWORD;
    return ZR_FLATE;
  }

  return ZR_ARGS;
}

ZRESULT TUnzip::Close()
{ if (currentfile!=-1) unzCloseCurrentFile(uf); currentfile=-1;
  if (uf!=0) unzClose(uf); uf=0;
  return ZR_OK;
}


ZRESULT lasterrorU=ZR_OK;

unsigned int FormatZipMessageU(ZRESULT code, TCHAR *buf,unsigned int len)
{ if (code==ZR_RECENT) code=lasterrorU;
  const TCHAR *msg=_T("unknown zip result code");
  switch (code)
  { case ZR_OK: msg=_T("Success"); break;
    case ZR_NODUPH: msg=_T("Culdn't duplicate handle"); break;
    case ZR_NOFILE: msg=_T("Couldn't create/open file"); break;
    case ZR_NOALLOC: msg=_T("Failed to allocate memory"); break;
    case ZR_WRITE: msg=_T("Error writing to file"); break;
    case ZR_NOTFOUND: msg=_T("File not found in the zipfile"); break;
    case ZR_MORE: msg=_T("Still more data to unzip"); break;
    case ZR_CORRUPT: msg=_T("Zipfile is corrupt or not a zipfile"); break;
    case ZR_READ: msg=_T("Error reading file"); break;
    case ZR_PASSWORD: msg=_T("Correct password required"); break;
    case ZR_ARGS: msg=_T("Caller: faulty arguments"); break;
    case ZR_PARTIALUNZ: msg=_T("Caller: the file had already been partially unzipped"); break;
    case ZR_NOTMMAP: msg=_T("Caller: can only get memory of a memory zipfile"); break;
    case ZR_MEMSIZE: msg=_T("Caller: not enough space allocated for memory zipfile"); break;
    case ZR_FAILED: msg=_T("Caller: there was a previous error"); break;
    case ZR_ENDED: msg=_T("Caller: additions to the zip have already been ended"); break;
    case ZR_ZMODE: msg=_T("Caller: mixing creation and opening of zip"); break;
    case ZR_NOTINITED: msg=_T("Zip-bug: internal initialisation not completed"); break;
    case ZR_SEEK: msg=_T("Zip-bug: trying to seek the unseekable"); break;
    case ZR_MISSIZE: msg=_T("Zip-bug: the anticipated size turned out wrong"); break;
    case ZR_NOCHANGE: msg=_T("Zip-bug: tried to change mind, but not allowed"); break;
    case ZR_FLATE: msg=_T("Zip-bug: an internal error during flation"); break;
  }
  unsigned int mlen=(unsigned int)_tcslen(msg);
  if (buf==0 || len==0) return mlen;
  unsigned int n=mlen; if (n+1>len) n=len-1;
  _tcsncpy(buf,msg,n); buf[n]=0;
  return mlen;
}


typedef struct
{ DWORD flag;
  TUnzip *unz;
} TUnzipHandleData;

HZIP OpenZipInternal(void *z,unsigned int len,DWORD flags, const char *password)
{
	std::auto_ptr<TUnzip> unz(new TUnzip(password));
	lasterrorU = unz->Open(z,len,flags);
	if (lasterrorU!=ZR_OK) { return 0;}
	TUnzipHandleData *han = new TUnzipHandleData;
	han->flag=1; han->unz=unz.release(); return (HZIP)han;
}
HZIP OpenZipHandle(HANDLE h, const char *password) {return OpenZipInternal((void*)h,0,ZIP_HANDLE,password);}
HZIP OpenZip(const TCHAR *fn, const char *password) {return OpenZipInternal((void*)fn,0,ZIP_FILENAME,password);}
HZIP OpenZip(void *z,unsigned int len, const char *password) {return OpenZipInternal(z,len,ZIP_MEMORY,password);}


ZRESULT GetZipItem(HZIP hz, int index, ZIPENTRY *ze)
{ ze->index=0; *ze->name=0; ze->unc_size=0;
  if (hz==0) {lasterrorU=ZR_ARGS;return ZR_ARGS;}
  TUnzipHandleData *han = (TUnzipHandleData*)hz;
  if (han->flag!=1) {lasterrorU=ZR_ZMODE;return ZR_ZMODE;}
  TUnzip *unz = han->unz;
  lasterrorU = unz->Get(index,ze);
  return lasterrorU;
}

ZRESULT FindZipItem(HZIP hz, const TCHAR *name, bool ic, int *index, ZIPENTRY *ze)
{ if (hz==0) {lasterrorU=ZR_ARGS;return ZR_ARGS;}
  TUnzipHandleData *han = (TUnzipHandleData*)hz;
  if (han->flag!=1) {lasterrorU=ZR_ZMODE;return ZR_ZMODE;}
  TUnzip *unz = han->unz;
  lasterrorU = unz->Find(name,ic,index,ze);
  return lasterrorU;
}

ZRESULT UnzipItemInternal(HZIP hz, int index, void *dst, unsigned int len, DWORD flags)
{ if (hz==0) {lasterrorU=ZR_ARGS;return ZR_ARGS;}
  TUnzipHandleData *han = (TUnzipHandleData*)hz;
  if (han->flag!=1) {lasterrorU=ZR_ZMODE;return ZR_ZMODE;}
  TUnzip *unz = han->unz;
  lasterrorU = unz->Unzip(index,dst,len,flags);
  return lasterrorU;
}
ZRESULT UnzipItemHandle(HZIP hz, int index, HANDLE h) {return UnzipItemInternal(hz,index,(void*)h,0,ZIP_HANDLE);}
ZRESULT UnzipItem(HZIP hz, int index, const TCHAR *fn) {return UnzipItemInternal(hz,index,(void*)fn,0,ZIP_FILENAME);}
ZRESULT UnzipItem(HZIP hz, int index, void *z,unsigned int len) {return UnzipItemInternal(hz,index,z,len,ZIP_MEMORY);}


ZRESULT CloseZipU(HZIP hz)
{ if (hz==0) {lasterrorU=ZR_ARGS;return ZR_ARGS;}
  TUnzipHandleData *han = (TUnzipHandleData*)hz;
  if (han->flag!=1) {lasterrorU=ZR_ZMODE;return ZR_ZMODE;}
  TUnzip *unz = han->unz;
  lasterrorU = unz->Close();
  delete unz;
  delete han;
  return lasterrorU;
}

bool IsZipHandleU(HZIP hz)
{ if (hz==0) return false;
  TUnzipHandleData *han = (TUnzipHandleData*)hz;
  return (han->flag==1);
}
