
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

    obstream dstStream(zFileInfo.strPath, true);
    if (!dstStream)
    {
        return false;
    }
    if (!dstStream.write(btbData, len))
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

bool zipDecompress(const string& strZipFile, const string& strDstDir)
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
long ziputil::qCompressFile(const wstring& strSrcFile, const wstring& strDstFile, int nCompressLecvel)
{
    CByteBuff btbData;
    if (!fsutil::loadBinary(strSrcFile, btbData))
    {
        return -1;
    }
    if (!btbData)
    {
        return 0;
    }

    cauto baOutput = qCompress(btbData, btbData->size(), nCompressLecvel);

    obstream dstStream(strDstFile, true);
    __EnsureReturn(dstStream, false);
    if (!dstStream.write(baOutput.data(), baOutput.size()))
    {
        return -1;
    }

    return baOutput.size();
}

long ziputil::qUncompressFile(const wstring& strSrcFile, const wstring& strDstFile)
{
    CByteBuff btbData;
    if (!fsutil::loadBinary(strSrcFile, btbData))
    {
        return -1;
    }
    if (!btbData)
    {
        return 0;
    }

    cauto baOutput = qUncompress(btbData, btbData->size());

    obstream dstStream(strDstFile, true);
    __EnsureReturn(dstStream, false);
    if (!dstStream.write(baOutput.data(), baOutput.size()))
    {
        return -1;
    }

    return baOutput.size();
}
#endif

static int _zcompressFile(const wstring& strSrcFile, const wstring& strDstFile
                     , const function<UINT(const CByteBuff&, CByteBuff&)>& cb)
{
    CByteBuff btbData;
    if (!fsutil::loadBinary(strSrcFile, btbData))
    {
        return -1;
    }
    if (!btbData)
    {
        return 0;
    }

    CByteBuff btbOutput;
    UINT len = cb(btbData, btbOutput);
    if (0 == len)
    {
        return 0;
    }

    obstream dstStream(strDstFile, true);
    __EnsureReturn(dstStream, false);
    if (!dstStream.write(btbOutput, len))
    {
        return -1;
    }

    return len;
}

#include <zlib.h>

long ziputil::zCompressFile(const wstring& strSrcFile, const wstring& strDstFile, int level) // Z_BEST_COMPRESSION
{
    return _zcompressFile(strSrcFile, strDstFile, [&](const CByteBuff& btbData, CByteBuff& btbOutput){
        auto srcLen = btbData->size();

        uLongf destLen = srcLen;
		btbOutput->resize(destLen);

        int nRet = compress2(btbOutput, &destLen, btbData, srcLen, level);
        if (nRet != Z_OK)
        {
            return 0ul;
        }

        return destLen;
    });
}

long ziputil::zUncompressFile(const wstring& strSrcFile, const wstring& strDstFile)
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

bool ziputil::zUncompressZip(const string& strZipFile, const string& strDstDir)
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

    return zipDecompress(strZipFile, t_strDstDir);
}
