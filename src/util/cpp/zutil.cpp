
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

static bool _zipDecompress(unzFile zfile, const tagUnzFileInfo& zFileInfo)
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

    obstream obs(zFileInfo.strPath, true);
    if (!obs)
    {
        return false;
    }
    if (!obs.write(btbData, len))
    {
        return false;
    }

    return true;
}

static bool _zipDecompress(unzFile zfile, const string& strDstDir)
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
            bRet = _zipDecompress(zfile, zFileInfo);
            if (!bRet)
            {
                return false;
            }
        }

        (void)unzGoToNextFile(zfile);
    }

    return true;
}

bool _zipDecompress(const string& strZipFile, const string& strDstDir)
{
    unzFile zfile = unzOpen(strZipFile.c_str());
    if (NULL == zfile)
    {
        return false;
    }

    bool bRet = _zipDecompress(zfile, strDstDir);

    unzClose(zfile);

    return bRet;
}

#if !__winvc
long zutil::qCompressFile(const wstring& strSrcFile, const wstring& strDstFile, int nCompressLecvel)
{
    CByteBuff btbData;
    if (!fsutil::loadFile(strSrcFile, btbData))
    {
        return -1;
    }
    if (!btbData)
    {
        return 0;
    }

    cauto baOutput = qCompress(btbData, btbData->size(), nCompressLecvel);

    obstream obs(strDstFile, true);
    __EnsureReturn(obs, false);
    if (!obs.write(baOutput.data(), baOutput.size()))
    {
        return -1;
    }

    return baOutput.size();
}

long zutil::qUncompressFile(const wstring& strSrcFile, const wstring& strDstFile)
{
    CByteBuff btbData;
    if (!fsutil::loadFile(strSrcFile, btbData))
    {
        return -1;
    }
    if (!btbData)
    {
        return 0;
    }

    cauto baOutput = qUncompress(btbData, btbData->size());

    obstream obs(strDstFile, true);
    __EnsureReturn(obs, false);
    if (!obs.write(baOutput.data(), baOutput.size()))
    {
        return -1;
    }

    return baOutput.size();
}
#endif

static int _zcompressFile(const wstring& strSrcFile, const wstring& strDstFile
                     , const function<int(const CByteBuff&, CByteBuff&)>& cb)
{
    CByteBuff btbData;
    if (!fsutil::loadFile(strSrcFile, btbData))
    {
        return -1;
    }
    if (!btbData)
    {
        return 0;
    }

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

    obstream obs(strDstFile, true);
    __EnsureReturn(obs, false);
    if (!obs.write(btbOutput, len))
    {
        return -1;
    }

    return len;
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

bool zutil::zipUncompress(const string& strZipFile, const string& strDstDir)
{
    if (!fsutil::createDir(strutil::toWstr(strDstDir)))
    {
        return false;
    }

    string t_strDstDir(strDstDir);
    if (strDstDir.empty() || !fsutil::checkPathTail(strDstDir.back()))
    {
        t_strDstDir.append(1, (char)__wcDirSeparator);
    }

    return _zipDecompress(strZipFile, t_strDstDir);
}
