
#include "util.h"

#include "contrib/minizip/unzip.h"

#if __windows
#define __DirFlag FILE_ATTRIBUTE_DIRECTORY
#else
#define __DirFlag S_IFDIR
#define _mkdir(x) mkdir(x, 0777)
#endif

static bool zipDecompress(unzFile zfile, const unz_file_info& zFileInfo, const string& strFilePath)
{
    int nRet = unzOpenCurrentFile(zfile);
    if (nRet != UNZ_OK)
    {
        return false;
    }

    uLong len = zFileInfo.uncompressed_size;
    vector<char> vecData(len);
    const char *buf = &vecData.front();
    nRet = unzReadCurrentFile(zfile, (voidp)buf, len);

    (void)unzCloseCurrentFile(zfile);
    if (nRet != (int)len)
    {
        return false;
    }

    obstream dstStream(strFilePath, true);
    __EnsureReturn(dstStream, false);
    if (!dstStream.write(buf, len))
    {
        return false;
    }

    return true;
}

static bool zipDecompress(unzFile zfile, const string& strDstDir)
{
    unz_global_info zGlobalInfo;
    int nRet = unzGetGlobalInfo(zfile, &zGlobalInfo);
    if (nRet != UNZ_OK)
    {
        return false;
    }

    list<pair<unz_file_info, string>> lstZFileInfo;
    for (uLong i = 0; i < zGlobalInfo.number_entry; i++)
    {
        vector<char> vecFileName(MAX_PATH + 1);
        char *szFileName = &vecFileName.front();
        unz_file_info zFileInfo;
        int nRet = unzGetCurrentFileInfo(zfile, &zFileInfo, szFileName, MAX_PATH, NULL, 0, NULL, 0);
        if (nRet != UNZ_OK)
        {
            return false;
        }
        cauto& strPath = strDstDir + szFileName;

        if (zFileInfo.external_fa & __DirFlag)
        {
            if (_mkdir(strPath.c_str()))
            {
                return false;
            }
        }

        lstZFileInfo.push_back(std::make_pair(zFileInfo, strPath));

        (void)unzGoToNextFile(zfile);
    }

    (void)unzGoToFirstFile(zfile);

    bool bRet = false;
    for (cauto& zFileInfo : lstZFileInfo)
    {
        if ((zFileInfo.first.external_fa & __DirFlag) == 0)
        {
            bRet = zipDecompress(zfile, zFileInfo.first, zFileInfo.second);
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

    bool bRet = zipDecompress(zfile, strDstDir);

    unzClose(zfile);

    return bRet;
}
