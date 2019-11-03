
#include "util.h"

#include "contrib/minizip/unzip.h"

#if __windows
#define __DirFlag FILE_ATTRIBUTE_DIRECTORY
#else
#define __DirFlag S_IFDIR
#define _mkdir(x) mkdir(x, 0777)
#endif

long CZipFile::_readCurrent(void *buf, size_t len)
{
	int nRet = unzOpenCurrentFile(m_unzfile);
	if (nRet != UNZ_OK)
	{
		return -1;
	}

	long nCount = (long)unzReadCurrentFile(m_unzfile, buf, len);
	(void)unzCloseCurrentFile(m_unzfile);
	
	return nCount;
}

long CZipFile::read(const tagUnzFileInfo& unzFileInfo, void *buf, size_t len)
{
	unz_file_pos file_pos{ unzFileInfo.pos_in_zip_directory, unzFileInfo.num_of_file };
	int nRet = unzGoToFilePos(m_unzfile, &file_pos);
	if (nRet != UNZ_OK)
	{
		return -1;
	}

	return _readCurrent(buf, len);
}

bool CZipFile::_open(const char *szFile, void* pzlib_filefunc_def)
{
	unzFile unzfile = NULL;
	if (pzlib_filefunc_def)
	{
		unzfile = unzOpen2(szFile, (zlib_filefunc_def*)pzlib_filefunc_def);
	}
	else
	{
		unzfile = unzOpen(szFile);
	}
	if (NULL == unzfile)
	{
		return false;
	}
	
	/*unz_global_info zGlobalInfo;
	int nRet = unzGetGlobalInfo(unzfile, &zGlobalInfo);
	if (nRet != UNZ_OK)
	{
		(void)unzClose(t_unzfile);
		return false;
	}*/

	unz_file_info file_info;
	memzero(file_info);

	TD_CharBuffer lpFileName(MAX_PATH + 1);

	unz_file_pos file_pos{ 0,0 };

	do {
		int nRet = unzGetCurrentFileInfo(unzfile, &file_info, lpFileName, MAX_PATH, NULL, 0, NULL, 0);
		if (nRet != UNZ_OK)
		{
			(void)unzClose(unzfile);
			return false;
		}

		tagUnzFileInfo unzFileInfo;
		unzFileInfo.uFileSize = file_info.uncompressed_size;

		unzFileInfo.strPath = lpFileName;
		unzFileInfo.bDir = ((char)__wcSlant == unzFileInfo.strPath.back());  // (unzFileInfo.external_fa & __DirFlag);

		if (unzFileInfo.bDir)
		{
			m_lstSubDirInfo.push_back(unzFileInfo);
			m_lstSubInfo.push_back(&m_lstSubDirInfo.back());
		}
		else
		{
			nRet = unzGetFilePos(unzfile, &file_pos);
			if (nRet != UNZ_OK)
			{
				(void)unzClose(unzfile);
				return false;
			}
			unzFileInfo.pos_in_zip_directory = file_pos.pos_in_zip_directory;
			unzFileInfo.num_of_file = file_pos.num_of_file;

			m_lstSubFileInfo.push_back(unzFileInfo);
			m_lstSubInfo.push_back(&m_lstSubFileInfo.back());
		}

	} while (unzGoToNextFile(unzfile) == UNZ_OK);

	(void)unzGoToFirstFile(unzfile);
	
	m_unzfile = unzfile;

	return true;
}

static voidpf ZCALLBACK zopen_file(voidpf opaque, const char* filename, int mode)
{
	(void)filename;
	(void)mode;
    return opaque;
}

static uLong ZCALLBACK zread_file(voidpf opaque, voidpf stream, void* buf, uLong size)
{
	(void)opaque;
	if (!((Instream*)stream)->readex(buf, size))
	{
		return 0;
	}

	return size;
}

static long ZCALLBACK ztell_file(voidpf opaque, voidpf stream)
{
    (void)opaque;
    return ((Instream*)stream)->pos();
}

static long ZCALLBACK zseek_file(voidpf opaque, voidpf stream, uLong offset, int origin)
{
    (void)opaque;
	if (!((Instream*)stream)->seek(offset, origin))
	{
		return -1;
	}

	return 0;
}

static int ZCALLBACK zclose_file(voidpf opaque, voidpf stream)
{
	(void)opaque;
    (void)stream;
	return 0;
}

/*static int ZCALLBACK ztesterror_file(voidpf opaque, voidpf stream)
{
	return 0;
}*/

bool CZipFile::open(Instream& ins)
{
	zlib_filefunc_def zfunc;
	memzero(zfunc);

	zfunc.zopen_file = zopen_file;
	zfunc.zread_file = zread_file;
	zfunc.ztell_file = ztell_file;
	zfunc.zseek_file = zseek_file;
	zfunc.zclose_file = zclose_file;
	//zfunc.zerror_file = ztesterror_file;

	zfunc.opaque = &ins;

	return _open("", &zfunc);
}

bool CZipFile::unzip(const wstring& strDstDir)
{
	if (NULL == m_unzfile)
	{
		return false;
	}

	if (!fsutil::createDir(strDstDir))
	{
		return false;
	}

	string t_strDstDir(strutil::toStr(strDstDir));
	if (strDstDir.empty() || !fsutil::checkPathTail(strDstDir.back()))
	{
		t_strDstDir.push_back((char)__wcDirSeparator);
	}

	(void)unzGoToFirstFile(m_unzfile);

	for (cauto pInfo : m_lstSubInfo)
	{
		if (pInfo->bDir)
		{
			if (_mkdir((t_strDstDir + pInfo->strPath).c_str()))
			{
				return false;
			}
		}
		else
		{
			TD_ByteBuffer buff(pInfo->uFileSize);
            if (_readCurrent(buff, pInfo->uFileSize) != (long)pInfo->uFileSize)
			{
				return false;
			}
            OFStream ofs(t_strDstDir + pInfo->strPath, true);
			__EnsureReturn(ofs, false);
            if (!ofs.writex(buff, pInfo->uFileSize))
			{
				return false;
			}
		}

		(void)unzGoToNextFile(m_unzfile);
	}

	return true;
}

void CZipFile::close()
{
	if (m_unzfile)
	{
		(void)unzClose(m_unzfile);
		m_unzfile = NULL;
	}
}

static int _zcompressFile(const wstring& strSrcFile, const wstring& strDstFile
	, const function<int(const CByteBuffer&, CByteBuffer&)>& cb)
{
    IFStream ifs(strSrcFile);
    __EnsureReturn(ifs, -1);
    CByteBuffer bbfData;
	__EnsureReturn(ifs.readex(bbfData) > 0, 0);

	CByteBuffer bbfOutput;
	int len = cb(bbfData, bbfOutput);
	if (len < 0)
	{
		return -1;
	}
	if (0 == len)
	{
		return 0;
	}

	OFStream ofs(strDstFile, true);
	__EnsureReturn(ofs, -1);
    if (!ofs.writex(bbfOutput, (size_t)len))
	{
		return -1;
	}

	return len;
}

int zutil::zCompress(const void* pData, size_t len, CByteBuffer& bbfBuff, int level)
{
	auto ptr = bbfBuff.resizeMore(len);
	uLongf destLen = len;
	int nRet = compress2(ptr, &destLen, (const Bytef*)pData, len, level);
	if (nRet != Z_OK)
	{
		return -1;
	}

	if (destLen < len)
	{
		bbfBuff.resizeLess(len - destLen);
	}

	return destLen;
}

long zutil::zCompressFile(const wstring& strSrcFile, const wstring& strDstFile, int level) // Z_BEST_COMPRESSION
{
	return _zcompressFile(strSrcFile, strDstFile, [&](const CByteBuffer& bbfData, CByteBuffer& bbfOutput) {
		return zCompress(bbfData, bbfData->size(), bbfOutput, level);
	});
}

long zutil::zUncompressFile(const wstring& strSrcFile, const wstring& strDstFile)
{
	return _zcompressFile(strSrcFile, strDstFile, [&](const CByteBuffer& bbfData, CByteBuffer& bbfOutput) {
		size_t srcLen = bbfData->size();
		bbfOutput->resize(srcLen * 2);

		uLongf destLen = 0;
		int nRet = uncompress(bbfOutput, &destLen, bbfData, srcLen);
		if (nRet != 0)
		{
			return 0ul;
		}

		return destLen;
	});
}

#if !__winvc
long zutil::qCompressFile(const wstring& strSrcFile, const wstring& strDstFile, int nCompressLecvel)
{
    IFStream ifs(strSrcFile);
    __EnsureReturn(ifs, -1);
    CByteBuffer bbfData;
    __EnsureReturn(ifs.readex(bbfData), -1);
    __EnsureReturn(bbfData, 0);

    cauto baOutput = qCompress(bbfData, bbfData->size(), nCompressLecvel);

    OFStream ofs(strDstFile, true);
    __EnsureReturn(ofs, -1);
    if (!ofs.writex(baOutput.data(), (size_t)baOutput.size()))
	{
		return -1;
	}

	return baOutput.size();
}

long zutil::qUncompressFile(const wstring& strSrcFile, const wstring& strDstFile)
{
    IFStream ifs(strSrcFile);
    __EnsureReturn(ifs, -1);
    CByteBuffer bbfData;
    __EnsureReturn(ifs.readex(bbfData), -1);
    __EnsureReturn(bbfData, 0);

    cauto baOutput = qUncompress(bbfData, bbfData->size());

    OFStream ofs(strDstFile, true);
    __EnsureReturn(ofs, -1);
    if (!ofs.writex(baOutput.data(), (size_t)baOutput.size()))
	{
		return -1;
	}

	return baOutput.size();
}
#endif
