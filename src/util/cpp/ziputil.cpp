
#include "util.h"

#include "contrib/minizip/unzip.h"

#if __windows
#define __DirFlag FILE_ATTRIBUTE_DIRECTORY
#else
#define __DirFlag S_IFDIR
//#define _mkdir(x) mkdir(x, 0777)
#endif

long CZipFile::_readCurrent(void *buf, size_t len) const
{
	if (m_strPwd.empty())
	{
		int nRet = unzOpenCurrentFile(m_unzfile);
		if (nRet != UNZ_OK)
		{
			return -1;
		}
	}
	else
	{
		int nRet = unzOpenCurrentFilePassword(m_unzfile, m_strPwd.c_str());
		if (nRet != UNZ_OK)
		{
			return -1;
		}
	}

	long nCount = (long)unzReadCurrentFile(m_unzfile, buf, len);
	(void)unzCloseCurrentFile(m_unzfile);
	
	return nCount;
}

long CZipFile::_read(const tagUnzfile& unzFile, void *buf, size_t len) const
{
	unz_file_pos file_pos{ unzFile.pos_in_zip_directory, unzFile.num_of_file };
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
    memzero(zGlobalInfo);
    int nRet = unzGetGlobalInfo(unzfile, &zGlobalInfo);
	if (nRet != UNZ_OK)
	{
		(void)unzClose(t_unzfile);
		return false;
	}*/

	unz_file_info file_info;
	memzero(file_info);

	char pszFileName[MAX_PATH + 1];
	memzero(pszFileName);

	unz_file_pos file_pos{ 0,0 };

	do {
		int nRet = unzGetCurrentFileInfo(unzfile, &file_info, pszFileName, MAX_PATH, NULL, 0, NULL, 0);
		if (nRet != UNZ_OK)
		{
			(void)unzClose(unzfile);
			return false;
		}

        tagUnzfile unzFile;
        unzFile.uFileSize = file_info.uncompressed_size;
        unzFile.strPath = pszFileName;

        bool bDir = (__cSlant == unzFile.strPath.back());  // (unzFile.external_fa & __DirFlag);
        if (bDir)
        {
            m_lstUnzdirInfo.push_back(unzFile);

            m_lstUnzfile.emplace_back(true, &m_lstUnzdirInfo.back());
		}
		else
        {
			nRet = unzGetFilePos(unzfile, &file_pos);
			if (nRet != UNZ_OK)
			{
				(void)unzClose(unzfile);
				return false;
			}

			unzFile.pos_in_zip_directory = file_pos.pos_in_zip_directory;
			unzFile.num_of_file = file_pos.num_of_file;

            auto pFileInfo = &(m_mapUnzfile[unzFile.strPath] = unzFile);
            m_lstUnzfile.emplace_back(false, pFileInfo);
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
    return (long)((Instream*)stream)->pos();
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

static int ZCALLBACK ztesterror_file(voidpf opaque, voidpf stream)
{
    (void)opaque;
    (void)stream;
	return 0;
}

bool CZipFile::open(Instream& ins, const string& strPwd)
{
    if (ins.size() == 0)
    {
        return false;
    }

	m_strPwd = strPwd;

	zlib_filefunc_def zfunc;
	memzero(zfunc);

	zfunc.zopen_file = zopen_file;
	zfunc.zread_file = zread_file;
	zfunc.ztell_file = ztell_file;
	zfunc.zseek_file = zseek_file;
	zfunc.zclose_file = zclose_file;
    zfunc.zerror_file = ztesterror_file;

	zfunc.opaque = &ins;

	return _open("", &zfunc);
}

long CZipFile::unzip(const tagUnzfile& unzFile, const string& strDstFile)
{
    CByteBuffer bbfFile;
    int nCount = this->read(unzFile, bbfFile);
    if (nCount <= 0)
    {
        return nCount;
    }

    return OFStream::writefile(strDstFile, true, bbfFile);
}

bool CZipFile::unzip(const string& strDstDir) const
{
	if (NULL == m_unzfile)
	{
		return false;
	}

	if (!fsutil::createDir(strDstDir))
	{
		return false;
	}

    string t_strDstDir(strDstDir);
    if (!fsutil::checkPathTail(t_strDstDir))
	{
		t_strDstDir.push_back((char)__wcPathSeparator);
	}

	(void)unzGoToFirstFile(m_unzfile);

    for (cauto pr : m_lstUnzfile)
    {
        auto& unzfile = *pr.second;
        if (pr.first)
        {
            if (!fsutil::createDir((t_strDstDir + unzfile.strPath).c_str()))
			{
				return false;
			}
		}
		else
		{
            TD_ByteBuffer buff(unzfile.uFileSize);
            if (_readCurrent(buff, unzfile.uFileSize) != (long)unzfile.uFileSize)
			{
				return false;
			}
            if (!OFStream::writefilex(t_strDstDir + unzfile.strPath, true, buff))
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
    CByteBuffer bbfData;
    __EnsureReturn(IFStream::readfile(strSrcFile, bbfData)>0 && bbfData, -1);
    
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

	if (!OFStream::writefilex(strDstFile, true, bbfOutput, (size_t)len))
	{
		return -1;
	}

	return len;
}

int ziputil::zCompress(const void* pData, size_t len, CByteBuffer& bbfBuff, int level)
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

long ziputil::zCompressFile(const wstring& strSrcFile, const wstring& strDstFile, int level) // Z_BEST_COMPRESSION
{
	return _zcompressFile(strSrcFile, strDstFile, [&](const CByteBuffer& bbfData, CByteBuffer& bbfOutput) {
		return zCompress(bbfData, bbfData->size(), bbfOutput, level);
	});
}

long ziputil::zUncompressFile(const wstring& strSrcFile, const wstring& strDstFile)
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
long ziputil::qCompressFile(const wstring& strSrcFile, const wstring& strDstFile, int nCompressLecvel)
{
    CByteBuffer bbfData;
    __EnsureReturn(IFStream::readfile(strSrcFile, bbfData)>0 && bbfData, -1);

    cauto baOutput = qCompress(bbfData, bbfData->size(), nCompressLecvel);

    if (!OFStream::writefilex(strDstFile, true, baOutput.data(), (size_t)baOutput.size()))
	{
		return -1;
	}

	return baOutput.size();
}

long ziputil::qUncompressFile(const wstring& strSrcFile, const wstring& strDstFile)
{
    CByteBuffer bbfData;
    __EnsureReturn(IFStream::readfile(strSrcFile, bbfData)>0 && bbfData, -1);

    cauto baOutput = qUncompress(bbfData, bbfData->size());

    if (!OFStream::writefilex(strDstFile, true, baOutput.data(), (size_t)baOutput.size()))
	{
		return -1;
	}

	return baOutput.size();
}
#endif
