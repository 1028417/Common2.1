
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

static bool zipDecompress(unzFile zfile, const tagUnzFileInfo& zFileInfo)
{
    int nRet = unzOpenCurrentFile(zfile);
    if (nRet != UNZ_OK)
    {
        return false;
    }

    uLong len = zFileInfo.uncompressed_size;
	CByteVector vecData(len);
    nRet = unzReadCurrentFile(zfile, vecData, len);
    (void)unzCloseCurrentFile(zfile);
    if (nRet != (int)len)
    {
        return false;
    }

	FILE *pf = fsutil::fopen(zFileInfo.strPath, "wb");
	if (NULL == pf)
	{
		return false;
	}
    obstream dstStream(pf);
    if (!dstStream.write(vecData, len))
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

    list<tagUnzFileInfo> lstZFileInfo;
    for (uLong i = 0; i < zGlobalInfo.number_entry; i++)
    {        
        tagUnzFileInfo zFileInfo;
		CByteVector vecFileName(MAX_PATH + 1);
		char *szFileName = vecFileName;
        int nRet = unzGetCurrentFileInfo(zfile, &zFileInfo, szFileName, MAX_PATH, NULL, 0, NULL, 0);
        if (nRet != UNZ_OK)
        {
            return false;
        }

        zFileInfo.strPath = strDstDir + szFileName;
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
            bRet = zipDecompress(zfile, zFileInfo);
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
