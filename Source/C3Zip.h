// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2023, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3CRC.h>
#include <PowerProps.h>
#include <deque>

namespace c3
{

	struct ZipEntry
	{
		uint32_t size;
		uint32_t offset;
		tstring fname;
	};

	typedef std::deque<ZipEntry> ZipEntryArray;

	// Zip file info (central directory structure)
	struct ZIPFILEINFO
	{

#pragma pack(push, 1)
		struct INMEM
		{
			uint32_t sig;				// central file header signature   4 bytes  (0x02014b50)
			uint16_t ver;				// version made by                 2 bytes
			uint16_t extver;			// version needed to extract       2 bytes
			props::TFlags16 flags;		// general purpose bit flag        2 bytes
			uint16_t comp;				// compression method              2 bytes
			uint16_t time;				// last mod file time              2 bytes
			uint16_t date;				// last mod file date              2 bytes
			uint32_t crc32;				// crc-32                          4 bytes
			uint32_t csize;				// compressed size                 4 bytes
			uint32_t size;				// uncompressed size               4 bytes
			uint16_t fnlen;				// filename length                 2 bytes
			uint16_t extlen;			// extra field length              2 bytes
			uint16_t comlen;			// file comment length             2 bytes
			uint16_t disk;				// disk number start               2 bytes
			uint16_t intfattr;			// internal file attributes        2 bytes
			uint16_t fattrlo;			// external file attributes        4 bytes
			uint16_t fattrhi;
			uint16_t offsetlo;			// relative offset of local header 4 bytes
			uint16_t offsethi;
		} read_me;
#pragma pack(pop)

		tstring fname;				// File name (is stored after structure above)
		tstring extra;				// Extra data (is stored after fname)
		tstring comment;			// Comment (is stored after extra)

	};

	typedef std::deque<ZIPFILEINFO *> TDirStructArray;
	typedef ZIPFILEINFO ZIPCENTRALDIRSTRUCT;

	typedef std::deque<char *> StringArray;

	// Zip info structure
	struct ZIPINFO
	{

#pragma pack(push, 1)
		struct INMEM
		{
			uint32_t sig;				// end of central dir signature    4 bytes  (0x06054b50)
			uint16_t disk;				// number of this disk             2 bytes
			uint16_t startdisk;			// number of the disk with the
										// start of the central directory  2 bytes
			uint16_t diskentries;		// total number of entries in
										// the central dir on this disk    2 bytes
			uint16_t totentries;		// total number of entries in
										// the central dir                 2 bytes
			uint32_t dirsize;			// size of the central directory   4 bytes
			uint32_t diskoffset;		// offset of start of central
										// directory with respect to
										// the starting disk number        4 bytes
			uint16_t comlen;			// zipfile comment length          2 bytes
		} read_me;
#pragma pack(pop)

		tstring comment;			// zip file comment (is stored after structure above)

	};

	typedef ZIPINFO ZIPCENTRALDIREND;

	typedef enum
	{
		ZIPERR_OK,
		ZIPERR_FAIL,
		ZIPERR_FILENOTFOUND,
		ZIPERR_CANTOPEN,
		ZIPERR_READ,
		ZIPERR_WRITE,
		ZIPERR_BADOPENFLAGS,
		ZIPERR_NOTAZIPFILE,
		ZIPERR_ERRORINZIP,
		ZIPERR_ZIPPATHOVERFLOW,
		ZIPERR_UNSUPPORTEDCOMPFORMAT
	} ZIPERROR;

#define ZIPOPEN_READ	1
#define ZIPOPEN_WRITE	2

#define ZFLAGS_READ		(ZIPOPEN_READ)
#define ZFLAGS_WRITE	(ZIPOPEN_WRITE)
#define ZFLAGS_OPENING	4
#define ZFLAGS_HASCOMP	8
#define ZFLAGS_HASENC	16


	class ZipFile
	{

	public:

		// Empty constructor (file is not opened)
		ZipFile();

		// Destroys zip file object
		~ZipFile();

		// Opens the zip file
		bool Open(const TCHAR *name, props::TFlags32 openflags);

		// Closes the zip file
		void Close();

		// Returns the current error status
		ZIPERROR Error() { return m_Error; }

		// Returns zip header block (for files opened in READ mode only!)
		const ZIPINFO *ZipInfo() const { return &m_Header; }

		size_t GetNumEntries() const { return m_Files.size(); }

		// Returns zip file info struct
		const ZipEntry *GetContentInfo(size_t file_idx) const;

		// Finds a file in zip archive... places the starting address of the content in *content_addr and its length in content_len
		bool GetContent(size_t file_idx, void **content_addr = nullptr, size_t *content_len = nullptr) const;

		size_t FindFileIndex(const TCHAR *filename) const;


	protected:
		// Inializes data when object is constructed
		void Clear();

		// Sets an error state
		void SetError(ZIPERROR newerror);

		// Reads zip directory struct entries given current file position, nested zipfile
		// level, path buffer, and file size.
		bool ReadZipEntries();

		// Read block from zip file
		bool ReadBlock(void *data, size_t len);

		// Reads a zip entry structure from the zipfiles central dir struct
		bool ReadZipEntry();

		// Read central directory structure
		//bool ReadCentralDirStruct(ZipEntry *zipentry, ZIPCENTRALDIRSTRUCT *entry);

		// Read central directory end structure (header)
		bool ReadCentralDirEnd(ZIPCENTRALDIREND *entry);

		props::TFlags32 m_Flags;			// Open flags for zip
		ZIPERROR m_Error;					// Last zip error
		HANDLE m_hFile;						// Open file for zip file
		HANDLE m_hFileMap;
		void *m_MappedAddr;
		ZipEntryArray m_Files;				// Files in zip
		ZIPINFO m_Header;					// Header of zip

	};

};
