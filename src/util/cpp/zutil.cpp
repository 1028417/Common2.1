
#include "util.h"

#include "contrib/minizip/unzip.h"

#if __windows
#define __DirFlag FILE_ATTRIBUTE_DIRECTORY
#else
#define __DirFlag S_IFDIR
#define _mkdir(x) mkdir(x, 0777)
#endif

struct tagUnzFileInfo : unz_file_info
{
    string strPath;

    bool bDir = false;
};

static bool _unzipSubFile(unzFile zfile, const tagUnzFileInfo& zFileInfo)
{
    int nRet = unzOpenCurrentFile(zfile);
    if (nRet != UNZ_OK)
    {
        return false;
    }

    uLong len = zFileInfo.uncompressed_size;
	CByteBuff btbData(len);
    nRet = unzReadCurrentFile(zfile, btbData, len);
    (void)unzCloseCurrentFile(zfile);
    if (nRet != (int)len)
    {
        return false;
    }

    OBStream obs(zFileInfo.strPath, true);
	__EnsureReturn(obs, false);
    if (!obs.writeex(btbData, len))
    {
        return false;
    }

    return true;
}

static bool _unzipSubFile(unzFile zfile, const string& strDstDir)
{
    unz_global_info zGlobalInfo;
    int nRet = unzGetGlobalInfo(zfile, &zGlobalInfo);
    if (nRet != UNZ_OK)
    {
        return false;
    }

    list<tagUnzFileInfo> lstZFileInfo;
    for (uLong i = 0; i < zGlobalInfo.number_entry; i++)
    {        
        tagUnzFileInfo zFileInfo;
        CCharBuff chbFileName(MAX_PATH + 1);
        int nRet = unzGetCurrentFileInfo(zfile, &zFileInfo, chbFileName, MAX_PATH, NULL, 0, NULL, 0);
        if (nRet != UNZ_OK)
        {
            return false;
        }

        zFileInfo.strPath = strDstDir + chbFileName.str();
        if ((char)__wcSlant == zFileInfo.strPath.back()) // if (zFileInfo.external_fa & __DirFlag)
        {
            zFileInfo.bDir = true;

            if (_mkdir(zFileInfo.strPath.c_str()))
            {
                return false;
            }
        }

        lstZFileInfo.push_back(zFileInfo);

        (void)unzGoToNextFile(zfile);
    }

    (void)unzGoToFirstFile(zfile);

    bool bRet = false;
    for (cauto zFileInfo : lstZFileInfo)
    {
        if (!zFileInfo.bDir)
        {
            bRet = _unzipSubFile(zfile, zFileInfo);
            if (!bRet)
            {
                return false;
            }
        }

        (void)unzGoToNextFile(zfile);
    }

    return true;
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

static bool _unzipFile(const wstring& strDstDir, const char *szFile, zlib_filefunc_def* pzlib_filefunc_def)
{
	if (!fsutil::createDir(strDstDir))
	{
		return false;
	}

	string t_strDstDir(strutil::toStr(strDstDir));
	if (strDstDir.empty() || !fsutil::checkPathTail(strDstDir.back()))
	{
		t_strDstDir.append(1, (char)__wcDirSeparator);
	}

	unzFile zfile = NULL;
	if (pzlib_filefunc_def)
	{
		zfile = unzOpen2(szFile, pzlib_filefunc_def);
	}
	else
	{
		zfile = unzOpen(szFile);
	}

    bool bRet = false;
	if (zfile)
	{
		bRet = _unzipSubFile(zfile, t_strDstDir);
		unzClose(zfile);
	}

    return bRet;
}

bool zutil::unzipFile(const string& strZipFile, const wstring& strDstDir)
{
	return _unzipFile(strDstDir, strZipFile.c_str(), NULL);
}

static voidpf ZCALLBACK zopen_file(voidpf opaque, const char* filename, int mode)
{
	(void)filename;
	(void)mode;
	return &((ZInopaque*)opaque)->stream();
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

//static uLong ZCALLBACK write_file_func(voidpf opaque, voidpf stream, const void* buf, uLong size);

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

static int ZCALLBACK ztesterror_file(voidpf opaque, voidpf stream)
{
	return 0;
}

bool ZInopaque::unzip(const wstring& strDstDir)
{
	zlib_filefunc_def zfunc;
	memzero(zfunc);

	zfunc.zopen_file = zopen_file;
	zfunc.zread_file = zread_file;
	zfunc.ztell_file = ztell_file;
	zfunc.zseek_file = zseek_file;
	zfunc.zclose_file = zclose_file;
	zfunc.zerror_file = ztesterror_file;

	zfunc.opaque = this;

	return _unzipFile(strDstDir, NULL, &zfunc);
}

#include <zlib.h>

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
    return _zcompressFile(strSrcFile, strDstFile, [&](const CByteBuff& btbData, CByteBuff& btbOutput){
        return zCompress(btbData, btbData->size(), btbOutput, level);
    });
}

long zutil::zUncompressFile(const wstring& strSrcFile, const wstring& strDstFile)
{
    return _zcompressFile(strSrcFile, strDstFile, [&](const CByteBuff& btbData, CByteBuff& btbOutput){
		size_t srcLen = btbData->size();
		btbOutput->resize(srcLen*2);

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
