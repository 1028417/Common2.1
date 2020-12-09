
#include "util.h"

#include "contrib/minizip/unzip.h"

#if __windows
#define __DirFlag FILE_ATTRIBUTE_DIRECTORY
#else
#define __DirFlag S_IFDIR
//#define _mkdir(x) mkdir(x, 0777)
#endif

inline bool CZipFile::_unzOpen() const
{
    int nRet = 0;
    if (m_strPwd.empty())
    {
        nRet = unzOpenCurrentFile(m_pfile);
    }
    else
    {
        nRet = unzOpenCurrentFilePassword(m_pfile, m_strPwd.c_str());
    }
    if (nRet != UNZ_OK)
    {
        return false;
    }

    return true;
}

inline bool CZipFile::_unzOpen(const tagUnzSubFile& unzSubFile) const
{
    unz_file_pos file_pos{ unzSubFile.pos_in_zip_directory, unzSubFile.num_of_file };
    int nRet = unzGoToFilePos(m_pfile, &file_pos);
    if (nRet != UNZ_OK)
    {
        return false;
    }

    if (!_unzOpen())
    {
        return false;
    }

    return true;
}

bool CZipFile::unzOpen(const tagUnzSubFile& unzSubFile)
{
    if (!_unzOpen(unzSubFile))
    {
        return false;
    }

    m_pCurrent = &unzSubFile;

    return true;
}

#define _unzRead(buf, len) unzReadCurrentFile(m_pfile, buf, len)

long CZipFile::unzRead(void *buf, size_t len) const
{
    return _unzRead(buf, len);
}

#define _unzClose() (void)unzCloseCurrentFile(m_pfile)

void CZipFile::unzClose()
{
    m_pCurrent = NULL;
    _unzClose();
}

long CZipFile::_read(const tagUnzSubFile& unzSubFile, void *buf, size_t len) const
{
    if (!_unzOpen(unzSubFile))
	{
		return -1;
	}

    auto nCount = _unzRead(buf, len);
    _unzClose();

    return nCount;
}

bool CZipFile::_open(const char *szFile, void* pzlib_filefunc_def)
{
    unzFile pfile = NULL;
	if (pzlib_filefunc_def)
	{
        pfile = unzOpen2(szFile, (zlib_filefunc_def*)pzlib_filefunc_def);
	}
	else
	{
        pfile = ::unzOpen(szFile);
	}
    if (NULL == pfile)
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
        int nRet = unzGetCurrentFileInfo(pfile, &file_info, pszFileName, MAX_PATH, NULL, 0, NULL, 0);
		if (nRet != UNZ_OK)
		{
            (void)::unzClose(pfile);
			return false;
		}

        string strPath = pszFileName;
        if (__cSlant == strPath.back())  // (unzFile.external_fa & __DirFlag);
        {
            m_lstSubDir.push_back(strPath);
		}
		else
        {
            nRet = unzGetFilePos(pfile, &file_pos);
			if (nRet != UNZ_OK)
			{
                (void)::unzClose(pfile);
				return false;
			}

            tagUnzSubFile& unzSubFile = m_mapSubfile[strPath];
            unzSubFile.strPath = strPath;
            unzSubFile.compressed_size = file_info.compressed_size;
            unzSubFile.uncompressed_size = file_info.uncompressed_size;

            unzSubFile.pos_in_zip_directory = file_pos.pos_in_zip_directory;
            unzSubFile.num_of_file = file_pos.num_of_file;
		}
    } while (unzGoToNextFile(pfile) == UNZ_OK);

    (void)unzGoToFirstFile(pfile);
	
    m_pfile = pfile;

	return true;
}

static voidpf ZCALLBACK zopen_file(voidpf opaque, const char* filename, int mode)
{
    (void)opaque;
    // TODO if !is_open open()

	(void)filename;
	(void)mode;

    return NULL; //opaque;
}

static uLong ZCALLBACK zread_file(voidpf opaque, voidpf stream, void* buf, uLong size)
{
    if (!((Instream*)opaque)->readex(buf, size))
	{
		return 0;
	}

    (void)stream;

	return size;
}

static long ZCALLBACK ztell_file(voidpf opaque, voidpf stream)
{
    (void)stream;
    return (long)((Instream*)opaque)->pos();
}

static long ZCALLBACK zseek_file(voidpf opaque, voidpf stream, uLong offset, int origin)
{
    if (!((Instream*)opaque)->seek(offset, origin))
	{
		return -1;
	}

    (void)stream;

	return 0;
}

static int ZCALLBACK zclose_file(voidpf opaque, voidpf stream)
{
    (void)opaque;
    // TODO ((Instream*)opaque)->close();

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
    zfunc.opaque = &ins;

	zfunc.zopen_file = zopen_file;
	zfunc.zread_file = zread_file;
	zfunc.ztell_file = ztell_file;
	zfunc.zseek_file = zseek_file;
	zfunc.zclose_file = zclose_file;
    zfunc.zerror_file = ztesterror_file;

	return _open("", &zfunc);
}

long CZipFile::unzip(const tagUnzSubFile& unzSubFile, const string& strDstFile) const
{
    if (!_unzOpen(unzSubFile))
    {
        return -1;
    }

    CByteBuffer bbfFile;
    int nCount = this->read(unzSubFile, bbfFile);
    if (nCount <= 0)
    {
        return nCount;
    }

    return OFStream::writefile(strDstFile, true, bbfFile);
}

bool CZipFile::unzip(const string& strDstDir) const
{
    if (NULL == m_pfile)
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
        t_strDstDir.push_back(__cPathSeparator);
	}

    (void)unzGoToFirstFile(m_pfile);

    for (cauto strSubDir : m_lstSubDir)
    {
        if (!fsutil::createDir((t_strDstDir + strSubDir).c_str()))
        {
            return false;
        }
    }

    for (cauto pr : m_mapSubfile)
    {
        auto& unzSubFile = pr.second;
        TD_ByteBuffer buff(unzSubFile.uncompressed_size);
        if (!_unzOpen())
        {
            return false;
        }

        auto nCount = _unzRead(buff, unzSubFile.uncompressed_size);
        _unzClose();
        if (nCount != (long)unzSubFile.uncompressed_size)
        {
            return false;
        }

        if (!OFStream::writefilex(t_strDstDir + unzSubFile.strPath, true, buff))
        {
            return false;
        }

        (void)unzGoToNextFile(m_pfile);
	}

	return true;
}

void CZipFile::close()
{
    if (m_pfile)
	{
        (void)::unzClose(m_pfile);
        m_pfile = NULL;
	}
}

static int _zcompressFile(cwstr strSrcFile, cwstr strDstFile
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

long ziputil::zCompressFile(cwstr strSrcFile, cwstr strDstFile, int level) // Z_BEST_COMPRESSION
{
	return _zcompressFile(strSrcFile, strDstFile, [&](const CByteBuffer& bbfData, CByteBuffer& bbfOutput) {
		return zCompress(bbfData, bbfData->size(), bbfOutput, level);
	});
}

long ziputil::zUncompressFile(cwstr strSrcFile, cwstr strDstFile)
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
long ziputil::qCompressFile(cwstr strSrcFile, cwstr strDstFile, int nCompressLecvel)
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

long ziputil::qUncompressFile(cwstr strSrcFile, cwstr strDstFile)
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
