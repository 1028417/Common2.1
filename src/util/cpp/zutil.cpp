
#include "util.h"

#include "contrib/minizip/unzip.h"

#if __windows
#define __DirFlag FILE_ATTRIBUTE_DIRECTORY
#else
#define __DirFlag S_IFDIR
#define _mkdir(x) mkdir(x, 0777)
#endif

int CZipFile::_readCurrent(void *buf, UINT len)
{
	int nRet = unzOpenCurrentFile(m_unzfile);
	if (nRet != UNZ_OK)
	{
		return -1;
	}

	int nCount = unzReadCurrentFile(m_unzfile, buf, len);
	(void)unzCloseCurrentFile(m_unzfile);
	
	return nCount;
}

int CZipFile::read(const tagUnzFileInfo& unzFileInfo, void *buf, UINT len)
{
	unz_file_pos file_pos{ unzFileInfo.pos_in_zip_directory, unzFileInfo.num_of_file };
	int nRet = unzGoToFilePos(m_unzfile, &file_pos);
	if (nRet != UNZ_OK)
	{
		return -1;
	}

	return _readCurrent(buf, len);
}

static unzFile _unzOpen(list<tagUnzFileInfo>& lstUnzFileInfo, const char *szFile, zlib_filefunc_def* pzlib_filefunc_def=NULL)
{
	unzFile t_unzfile = NULL;
	if (pzlib_filefunc_def)
	{
		t_unzfile = unzOpen2(szFile, pzlib_filefunc_def);
	}
	else
	{
		t_unzfile = unzOpen(szFile);
	}
	if (NULL == t_unzfile)
	{
		return NULL;
	}
	
	/*unz_global_info zGlobalInfo;
	int nRet = unzGetGlobalInfo(t_unzfile, &zGlobalInfo);
	if (nRet != UNZ_OK)
	{
		(void)unzClose(t_unzfile);
		return NULL;
	}*/

	unz_file_info file_info;
	memzero(file_info);

	TBuffer<char> lpFileName(MAX_PATH + 1);

	unz_file_pos file_pos{ 0,0 };

	do {
		int nRet = unzGetCurrentFileInfo(t_unzfile, &file_info, lpFileName, MAX_PATH, NULL, 0, NULL, 0);
		if (nRet != UNZ_OK)
		{
			(void)unzClose(t_unzfile);
			return NULL;
		}

		tagUnzFileInfo unzFileInfo;
		unzFileInfo.strPath = lpFileName;
		unzFileInfo.bDir = ((char)__wcSlant == unzFileInfo.strPath.back());  // (unzFileInfo.external_fa & __DirFlag);

		unzFileInfo.uFileSize = file_info.uncompressed_size;

		nRet = unzGetFilePos(t_unzfile, &file_pos);
		if (nRet != UNZ_OK)
		{
			(void)unzClose(t_unzfile);
			return NULL;
		}
		unzFileInfo.pos_in_zip_directory = file_pos.pos_in_zip_directory;
		unzFileInfo.num_of_file = file_pos.num_of_file;

		lstUnzFileInfo.push_back(unzFileInfo);

	} while (unzGoToNextFile(t_unzfile) == UNZ_OK);

	(void)unzGoToFirstFile(t_unzfile);

	return t_unzfile;
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

bool CZipFile::open(const string& strFile)
{
	m_unzfile = _unzOpen(m_lstUnzFileInfo, strFile.c_str());
	return m_unzfile != NULL;
}

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

	m_unzfile = _unzOpen(m_lstUnzFileInfo, "", &zfunc);
	return m_unzfile != NULL;
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

	for (cauto unzFileInfo : m_lstUnzFileInfo)
	{
		if (unzFileInfo.bDir)
		{
			if (_mkdir((t_strDstDir + unzFileInfo.strPath).c_str()))
			{
				return false;
			}
		}
		else
		{
			UINT len = unzFileInfo.uFileSize;
			TBuffer<byte_t> buff(len);
            if (_readCurrent(buff, len) != (int)len)
			{
				return false;
			}
            OBStream obs(t_strDstDir + unzFileInfo.strPath, true);
			__EnsureReturn(obs, false);
			if (!obs.writeex(buff, len))
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
	, const function<int(const CByteBuff&, CByteBuff&)>& cb)
{
	CByteBuff btbData;
	__EnsureReturn(fsutil::loadFile(strSrcFile, btbData), -1);
	__EnsureReturn(btbData, 0);

	CByteBuff btbOutput;
	int len = cb(btbData, btbOutput);
	if (len < 0)
	{
		return -1;
	}
	if (0 == len)
	{
		return 0;
	}

	OBStream obs(strDstFile, true);
	__EnsureReturn(obs, -1);
	if (!obs.writeex(btbOutput, (size_t)len))
	{
		return -1;
	}

	return len;
}

int zutil::zCompress(const void* pData, size_t len, CByteBuff& btbBuff, int level)
{
	uLongf destLen = len;
	btbBuff.resizeMore(destLen);
	int nRet = compress2(btbBuff, &destLen, (const Bytef*)pData, len, level);
	if (nRet != Z_OK)
	{
		return -1;
	}

	return destLen;
}

long zutil::zCompressFile(const wstring& strSrcFile, const wstring& strDstFile, int level) // Z_BEST_COMPRESSION
{
	return _zcompressFile(strSrcFile, strDstFile, [&](const CByteBuff& btbData, CByteBuff& btbOutput) {
		return zCompress(btbData, btbData->size(), btbOutput, level);
	});
}

long zutil::zUncompressFile(const wstring& strSrcFile, const wstring& strDstFile)
{
	return _zcompressFile(strSrcFile, strDstFile, [&](const CByteBuff& btbData, CByteBuff& btbOutput) {
		size_t srcLen = btbData->size();
		btbOutput->resize(srcLen * 2);

		uLongf destLen = 0;
		int nRet = uncompress(btbOutput, &destLen, btbData, srcLen);
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
	CByteBuff btbData;
	__EnsureReturn(fsutil::loadFile(strSrcFile, btbData), -1);
	__EnsureReturn(btbData, 0);

	cauto baOutput = qCompress(btbData, btbData->size(), nCompressLecvel);

    OBStream obs(strDstFile, true);
	__EnsureReturn(obs, -1);
	if (!obs.writeex(baOutput.data(), (size_t)baOutput.size()))
	{
		return -1;
	}

	return baOutput.size();
}

long zutil::qUncompressFile(const wstring& strSrcFile, const wstring& strDstFile)
{
	CByteBuff btbData;
	__EnsureReturn(fsutil::loadFile(strSrcFile, btbData), -1);
	__EnsureReturn(btbData, 0);

	cauto baOutput = qUncompress(btbData, btbData->size());

    OBStream obs(strDstFile, true);
	__EnsureReturn(obs, -1);
	if (!obs.writeex(baOutput.data(), (size_t)baOutput.size()))
	{
		return -1;
	}

	return baOutput.size();
}
#endif
