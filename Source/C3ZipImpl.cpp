// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2025, Keelan Stuart


#include "pch.h"

#include "C3ZipImpl.h"
#include <corecrt_io.h>

using namespace c3;


#define LOCK_ZIPS { WaitForSingleObject(ZipMutex, INFINITE); }
#define UNLOCK_ZIPS { ReleaseMutex(ZipMutex); }
#define UNLOCK_ZIP_RETURN(v) { UNLOCK_ZIPS return (v); }

#define MAXPATHLEN 256

#define ZIPLOCALHEADER_SIGNATURE		(0x04034b50)
#define ZIPCENTRALDIRSTRUCT_SIGNATURE	(0x02014b50)
#define ZIPCENTRALDIREND_SIGNATURE		(0x06054b50)

// ZIP version made by..
#define ZIPVER_MSDOS	(0) // 0 - MS-DOS and OS/2 (F.A.T. file systems)
#define ZIPVER_AMIGA	(1) // 1 - Amiga
#define ZIPVER_VAXVMS 	(2) // 2 - VAX/VMS
#define ZIPVER_UNIX		(3) // 3 - *nix
#define ZIPVER_VMCMS	(4) // 4 - VM/CMS
#define ZIPVER_ATARIST	(5) // 5 - Atari ST
#define ZIPVER_OS2HPFS  (6) // 6 - OS/2 H.P.F.S.
#define ZIPVER_MAC      (7) // 7 - Macintosh
#define ZIPVER_ZSYSTEM  (8) // 8 - Z-System
#define ZIPVER_CPM      (9) // 9 - CP/M

// ZIP general purpose  flags
#define ZIPFLAG_ENCRYPTED	(1<<0)	// This file is encrypted

  // Implode bits 1,2
#define ZIPFLAG_IMP8K		(1<<1)	// 8k sliding dictionary for implode 
#define ZIPFLAG_IMPTREE		(1<<2)	// 8k sliding dictionary for implode 

  // Deflate bits 1,2
#define ZIPFLAG_DEFNORM		(0)		// Normal compression
#define ZIPFLAG_DEFMAX		(2)		// Max compression
#define ZIPFLAG_DEFFAST		(4)		// Fast compression
#define ZIPFLAG_DEFSUPFAST	(6)		// Super fast compression
#define ZIPFLAG_DEFMASK		(6)		// Mask for deflate flags above flags

  // All bit 3
#define ZIPFLAG_DATADESC	(1<<3)	// Data descriptor follows local header

// Compression method
#define ZIPCOMP_STORE		(0)
#define ZIPCOMP_SHRUNK		(1)
#define ZIPCOMP_REDUCED1	(2)
#define ZIPCOMP_REDUCED2	(3)
#define ZIPCOMP_REDUCED3	(4)
#define ZIPCOMP_REDUCED4	(5)
#define ZIPCOMP_IMPLODED	(6)
#define ZIPCOMP_TOKENIZED	(7)
#define ZIPCOMP_DEFLATED	(8)

//  Files stored in arbitrary order.  Large zipfiles can span multiple
//  diskette media.
//
//  Overall zipfile format:
//
//    [local file header + file data + data_descriptor] . . .
//    [central directory] end of central directory record

// Local file header:
struct ZIPLOCALHEADER
{
	struct INMEM
	{
		uint32_t sig;				// local file header signature     4 bytes  (0x04034b50)
		uint16_t extver;			// version needed to extract       2 bytes
		uint16_t flags;				// general purpose bit flag        2 bytes
		uint16_t comp;				// compression method              2 bytes
		uint16_t time;				// last mod file time              2 bytes
		uint16_t date;				// last mod file date              2 bytes
		uint16_t crc32lo, crc32hi;	// crc-32                          4 bytes
		uint16_t csizelo, csizehi;	// compressed size                 4 bytes
		uint16_t sizelo, sizehi;	// uncompressed size               4 bytes
		uint16_t fnlen;				// filename length                 2 bytes
		uint16_t extlen;			// extra field length              2 bytes
	};

	tstring fname;					// Filename (is stored after structure above)
	tstring extra;					// Extra data (is stored after fname above)
};

#define ZIPLOCALHEADERSIZE	sizeof(ZIPLOCALHEADER::INMEM)

// Data descriptor:
struct ZIPDATADESCRIPTOR
{
	//  This descriptor exists only if bit 3 of the general
	//  purpose bit flag is set (see below).  It is byte aligned
	//  and immediately follows the last byte of compressed data.
	//  This descriptor is used only when it was not possible to
	//  seek in the output zip file, e.g., when the output zip file
	//  was standard output or a non seekable device.

	uint32_t crc32;			// crc-32                          4 bytes
	uint32_t csize;			// compressed size                 4 bytes
	uint32_t size;			// uncompressed size               4 bytes
};

#define ZIPDATADESCRIPTORSIZE sizeof(ZIPDATADESCRIPTOR)

// Central directory structure
#define ZIPCENTRALDIRSTRUCTSIZE sizeof(ZIPCENTRALDIRSTRUCT::INMEM)

// End of central dir record:
#define ZIPCENTRALDIRENDSIZE sizeof(ZIPCENTRALDIREND::INMEM)


// *************************************************
// * ZipFile - Object to read and write zip files *
// *************************************************

ZipFile::ZipFile()
{
	Clear();
}

ZipFile::~ZipFile()
{
	Close();
}

void ZipFile::Clear()
{
	m_hFile = INVALID_HANDLE_VALUE;
	m_hFileMap = INVALID_HANDLE_VALUE;
	m_MappedAddr = nullptr;
	m_Flags = 0;
	m_Error = ZIPERR_OK;
	m_Files.clear();
}

bool ZipFile::Open(const TCHAR *filename, props::TFlags32 openflags)
{
	bool ret = false;

	// Make sure zipfile is currently closed
	Close();

	m_Flags = openflags.Get() | ZFLAGS_OPENING;

	// Open zipfile for reading
	if (m_Flags.IsSet(ZIPOPEN_READ))
	{
		m_hFile = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, 0);
		if (m_hFile != INVALID_HANDLE_VALUE)
		{
			if (ReadZipEntries())
			{
				SetFilePointer(m_hFile, 0, nullptr, FILE_BEGIN);

				LARGE_INTEGER sz;
				GetFileSizeEx(m_hFile, &sz);

				m_hFileMap = CreateFileMapping(m_hFile, nullptr, PAGE_READONLY /* | SEC_IMAGE_NO_EXECUTE | SEC_LARGE_PAGES */, sz.HighPart, sz.LowPart, nullptr);
				if (m_hFileMap != INVALID_HANDLE_VALUE)
				{
					m_MappedAddr = MapViewOfFile(m_hFileMap, FILE_MAP_READ /* | FILE_MAP_LARGE_PAGES */, 0, 0, sz.QuadPart);

					ret = (m_MappedAddr != nullptr);
				}
			}
		}
	}
	else
	{
		m_Error = ZIPERR_BADOPENFLAGS;
	}

	// Clear opening flag
	m_Flags.Clear(ZFLAGS_OPENING);

	return ret;
}

void ZipFile::Close()
{
	if (m_MappedAddr)
		UnmapViewOfFile(m_MappedAddr);

	if (m_hFileMap == INVALID_HANDLE_VALUE)
		CloseHandle(m_hFileMap);

	if (m_hFile == INVALID_HANDLE_VALUE)
		CloseHandle(m_hFile);

	Clear();
}

void ZipFile::SetError(ZIPERROR newerror)
{
	// Make sure zip file is closed when open error occurs (exit gracefully)
	if (m_Flags.IsSet(ZFLAGS_OPENING))
		Close();

	// Set error code
	m_Error = newerror;
}

#define TRYREAD(d) { if (!ReadBlock(&(d), sizeof(d))) return 0; }
#define TRYREADBUF(d, l) { if (!ReadBlock((d), (l))) return 0; }


bool ZipFile::ReadBlock(void *data, size_t len)
{
	DWORD br;
	if (!ReadFile(m_hFile, data, (DWORD)len, &br, nullptr) || (br != (DWORD)len))
	{
		SetError(ZIPERR_READ);
		return false;
	}

	return true;
}


bool ZipFile::ReadZipEntry()
{
	ZIPCENTRALDIRSTRUCT::INMEM cds;

	// Read dir struct data... start with the signature
	TRYREADBUF(&cds, 4)
	if (cds.sig != ZIPCENTRALDIRSTRUCT_SIGNATURE)
	{
		if (cds.sig != ZIPCENTRALDIREND_SIGNATURE)
		{
			SetError(ZIPERR_ERRORINZIP);
			return false;
		}
		else
			return false;
	}

	// now read the rest
	TRYREADBUF(&(cds.ver), ZIPCENTRALDIRSTRUCTSIZE - 4)
	if (cds.comp != ZIPCOMP_STORE)
	{
		SetError(ZIPERR_UNSUPPORTEDCOMPFORMAT);
		return false;
	}		

	// Read file name...
	char *rpath = (char *)alloca(cds.fnlen + 1);
	TRYREADBUF(rpath, cds.fnlen);
	rpath[cds.fnlen] = '\0';

	TCHAR *tpath;
	CONVERT_MBCS2TCS(rpath, tpath);

	// Skip file extra data..
	if (cds.extlen > 0)
		SetFilePointer(m_hFile, cds.extlen, nullptr, FILE_CURRENT);

	// Skip file comment..
	if (cds.comlen > 0)
		SetFilePointer(m_hFile, cds.comlen, nullptr, FILE_CURRENT);

	m_Files.emplace_back();

	// Set zipentry stuff
	m_Files.back().size = cds.size;
	m_Files.back().offset = (cds.offsethi << 16) | cds.offsetlo;
	m_Files.back().fname = tpath;

	return true;
}

bool ZipFile::ReadCentralDirEnd(ZIPINFO *entry)
{
	memset(entry, 0, sizeof(ZIPINFO::INMEM));

	// Read dir end struct
	TRYREADBUF(entry, ZIPCENTRALDIRENDSIZE)

	// Read file comment
	if (entry->read_me.comlen > 0)
	{
		char *c = (char *)alloca(entry->read_me.comlen + 1);
		TRYREADBUF(c, entry->read_me.comlen);
		c[entry->read_me.comlen] = '\0';

		TCHAR *t;
		CONVERT_MBCS2TCS(c, t);
		entry->comment = t;
	}

	return true;
}

bool ZipFile::ReadZipEntries()
{
	DWORD sz = GetFileSize(m_hFile, nullptr);

	DWORD start = SetFilePointer(m_hFile, 0, nullptr, FILE_CURRENT);

	// Check file signature..
	uint32_t sig = 0;
	TRYREAD(sig);
	if (sig != ZIPLOCALHEADER_SIGNATURE)
		return false;

	// Jump directly to central dir struct	
	//	char *buf = new char[5000];
	//	memset(buf, 0, 5000);
	//	fseek(f, size - 4096, SEEK_SET); 
	//	TRYREADBUF(buf, 4096);

	SetFilePointer(m_hFile, sz - ZIPCENTRALDIRENDSIZE, nullptr, FILE_BEGIN);
	TRYREAD(sig);

	// Check if we're at end.. if not, skip back over comment
	int back = 0;

	while (sig != ZIPCENTRALDIREND_SIGNATURE && back < 1048) // Oops.. try going backwards to skip over comment
	{
		back++;
		SetFilePointer(m_hFile, sz - ZIPCENTRALDIRENDSIZE - back, 0, FILE_BEGIN);
		TRYREAD(sig);
	}

	if (sig != ZIPCENTRALDIREND_SIGNATURE)
	{
		SetError(ZIPERR_ERRORINZIP);
		return FALSE;
	}

	// Now go to beginning of central dir struct!!
	SetFilePointer(m_hFile, -4, nullptr, FILE_CURRENT);

	ReadCentralDirEnd(&m_Header);

	if (m_Error)
		return false;

	// See if we're at beginning of directory?
	SetFilePointer(m_hFile, m_Header.read_me.diskoffset, nullptr, FILE_BEGIN);
	TRYREAD(sig);
	if (sig != ZIPCENTRALDIRSTRUCT_SIGNATURE)
	{
		SetError(ZIPERR_ERRORINZIP);
		return false;
	}
	SetFilePointer(m_hFile, -4, nullptr, FILE_CURRENT);

	// Now load current headers 
	while (ReadZipEntry()) { }

	std::sort(m_Files.begin(), m_Files.end(), [](const ZipEntry &a, const ZipEntry &b) -> bool
	{
		return !_tcsicmp(a.fname.c_str(), b.fname.c_str());
	});
	
	// Everything seemed to go okay
	return true;
}


const ZipEntry *ZipFile::GetContentInfo(size_t file_idx) const
{
	if (file_idx < m_Files.size())
		return &(m_Files[file_idx]);

	return nullptr;
}


#pragma pack(push, 1)
struct ENTRYHEADER
{
	DWORD sig;				// local file header signature     4 bytes  (0x04034b50)
	WORD extver;			// version needed to extract       2 bytes
	WORD flags;				// general purpose bit flag        2 bytes
	WORD comp;				// compression method              2 bytes
	WORD time;				// last mod file time              2 bytes
	WORD date;				// last mod file date              2 bytes
	WORD crc32lo, crc32hi;	// crc-32                          4 bytes
	WORD csizelo, csizehi;	// compressed size                 4 bytes
	WORD sizelo, sizehi;	// uncompressed size               4 bytes
	WORD fnlen;				// filename length                 2 bytes
	WORD extlen;			// extra field length              2 bytes
};
#pragma pack(pop)

// Finds a file in zip archive... places the starting address of the content in *content_addr and its length in content_len
bool ZipFile::GetContent(size_t file_idx, void **content_addr, size_t *content_len) const
{
	if (!m_MappedAddr)
		return false;

	if (file_idx >= m_Files.size())
		return false;

	ZipEntry &pze = (ZipEntry &)(m_Files.at(file_idx));
	if (pze.size)
	{
		const ENTRYHEADER *peh = (ENTRYHEADER *)((BYTE *)m_MappedAddr + pze.offset);
		if (content_addr)
			*content_addr = (BYTE *)m_MappedAddr + pze.offset + sizeof(ENTRYHEADER) + peh->fnlen + peh->extlen;

		if (content_len)
			*content_len = pze.size;

		return true;
	}

	return false;
}


size_t ZipFile::FindFileIndex(const TCHAR *filename) const
{
	if (filename)
	{
		const TCHAR *tmp = _tcschr(filename, _T('/'));
		if (tmp)
		{
			tmp++;
			filename = tmp;
		}

		for (size_t i = 0, maxi = m_Files.size(); i < maxi; i++)
		{
			if (!_tcsicmp(m_Files[i].fname.c_str(), filename))
				return i;
		}
	}

	return -1;
}
