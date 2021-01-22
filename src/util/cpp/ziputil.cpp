
#include "util.h"

#if __winvc
#include "dirent.h"
#else
#include "dirent.h"
#endif

#include <fstream>
#include <sys/stat.h>
#include <sys/types.h>

extern "C"
{
#include "zlib.h"
#include "contrib/minizip/unzip.h"
#include "contrib/minizip/zip.h"
}

/*#if !__windows
#define mkdir(x) mkdir(x, 0777)
#endif*/

inline static wstring _subPath(wstring& strSubPath)
{
    wstring strRet;
    auto pos = strSubPath.find('/');
    if (0 == pos)
    {
        pos = strSubPath.find('/', 1);
        if (__npos != pos)
        {
            strRet = strSubPath.substr(1, pos-1);
        }
        else
        {
            strRet = strSubPath.substr(1);
        }
    }
    else
    {
        if (__npos != pos)
        {
            strRet = strSubPath.substr(0, pos);
        }
        else
        {
            strRet = strSubPath;
        }
    }

    strSubPath.erase(0, pos);

    strutil::lowerCase(strRet);
    return strRet;
}

tagUnzDir& tagUnzDir::addDir(wstring strSubDir)
{
    cauto strName = _subPath(strSubDir);

    tagUnzDir *pSubDir = NULL;
    auto itr = mapSubDir.find(strName);
    if (itr == mapSubDir.end())
    {
        pSubDir = &_addDir(strName);
    }
    else
    {
        pSubDir = &itr->second;
    }

    if (strSubDir.empty())
    {
        return *pSubDir;
    }
    return pSubDir->addDir(strSubDir);
}

tagUnzSubFile& tagUnzDir::addFile(wstring strSubFile)
{
    cauto strName = _subPath(strSubFile);
    if (strSubFile.empty())
    {
        return mapSubFile[strName];
    }

    tagUnzDir *pSubDir = NULL;
    auto itr = mapSubDir.find(strName);
    if (itr == mapSubDir.end())
    {
        pSubDir = &_addDir(strName);
    }
    else
    {
        pSubDir = &itr->second;
    }

    return pSubDir->addFile(strSubFile);
}

const tagUnzDir* tagUnzDir::subDir(wstring strSubDir) const
{
    cauto strName = _subPath(strSubDir);
    auto itr = mapSubDir.find(strName);
    if (itr == mapSubDir.end())
    {
        return NULL;
    }

    if (strSubDir.empty() || L"/" == strSubDir)
    {
        return &itr->second;
    }
    return itr->second.subDir(strSubDir);
}

const tagUnzSubFile* tagUnzDir::subFile(wstring strSubFile) const
{
    cauto strName = _subPath(strSubFile);
    if (strSubFile.empty())
    {
        auto itr = mapSubFile.find(strName);
        if (itr == mapSubFile.end())
        {
            return NULL;
        }
        return &itr->second;
    }

    auto itr = mapSubDir.find(strName);
    if (itr == mapSubDir.end())
    {
        return NULL;
    }
    return itr->second.subFile(strSubFile);
}

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

bool CZipFile::unzOpen(const tagUnzSubFile& unzSubFile) const
{
    unz_file_pos file_pos { unzSubFile.pos_in_zip_directory, unzSubFile.num_of_file };
    int nRet = unzGoToFilePos(m_pfile, &file_pos);
    if (nRet != UNZ_OK)
    {
        return false;
    }

    if (!_unzOpen())
    {
        return false;
    }

   ((tagUnzSubFile&)unzSubFile).data_pos = unzGetCurrentFileZStreamPos64(m_pfile);

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
    _unzClose();
}

long CZipFile::_read(const tagUnzSubFile& unzSubFile, void *buf, size_t len) const
{
    if (!unzOpen(unzSubFile))
	{
		return -1;
	}

    auto nCount = _unzRead(buf, len);
    _unzClose();

    return nCount;
}

bool CZipFile::_open(const char *szFile, void *pzlib_filefunc_def, const string& strPwd)
{
    m_strPwd = strPwd;

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

#if __windows
#define __DirFlag S_IFDIR //FILE_ATTRIBUTE_DIRECTORY //windows也有S_IFDIR
#else
#define __DirFlag S_IFDIR
#endif
        cauto strPath = strutil::fromGbk(pszFileName);
        if (__cSlant == strPath.back())  // (unzFile.external_fa & __DirFlag);
        {
            m_mapSubDir[strPath] = &m_root.addDir(strPath);
		}
		else
        {
            nRet = unzGetFilePos(pfile, &file_pos);
			if (nRet != UNZ_OK)
			{
                (void)::unzClose(pfile);
				return false;
			}

            tagUnzSubFile& unzSubFile = m_root.addFile(strPath);
            unzSubFile.strPath = strPath;
            unzSubFile.compression_method = file_info.compression_method;
            unzSubFile.compressed_size = file_info.compressed_size;
            unzSubFile.uncompressed_size = file_info.uncompressed_size;

            unzSubFile.pos_in_zip_directory = file_pos.pos_in_zip_directory;
            unzSubFile.num_of_file = file_pos.num_of_file;

            m_mapSubfile[strPath] = &unzSubFile;
		}
    } while (unzGoToNextFile(pfile) == UNZ_OK);

    (void)unzGoToFirstFile(pfile);
	
    m_pfile = pfile;

	return true;
}

static voidpf ZCALLBACK zopen_opaque(voidpf opaque, const char* filename, int mode)
{
	(void)filename;
	(void)mode;
    return opaque;
}

static int ZCALLBACK ztesterror_opaque(voidpf opaque, voidpf stream)
{
    (void)opaque;
    (void)stream;
    return 0;
}

bool CZipFile::_open(void *opaque, void *zread, void *ztell, void *zseek, void *zclose, const string& strPwd)
{
    zlib_filefunc_def zfunc;
    memzero(zfunc);
    zfunc.opaque = opaque;

    zfunc.zopen_file = zopen_opaque;
    zfunc.zread_file = (decltype(zfunc.zread_file))zread;
    zfunc.ztell_file = (decltype(zfunc.ztell_file))ztell;
    zfunc.zseek_file = (decltype(zfunc.zseek_file))zseek;
    zfunc.zclose_file = (decltype(zfunc.zclose_file))zclose;
    zfunc.zerror_file = ztesterror_opaque;

    return _open("", &zfunc, strPwd);
}

static uLong ZCALLBACK zread_file(voidpf opaque, voidpf stream, void* buf, uLong size)
{
    (void)stream;
    return fread(buf, 1, size, (FILE*)opaque); //fread(buf, size, 1, (FILE*)opaque);
}

static long ZCALLBACK ztell_file(voidpf opaque, voidpf stream)
{
    (void)stream;
    return ftell((FILE*)opaque);
}

static long ZCALLBACK zseek_file(voidpf opaque, voidpf stream, uLong offset, int origin)
{
    (void)stream;
    return fseek((FILE*)opaque, offset, origin);
}

static int ZCALLBACK zclose_file(voidpf opaque, voidpf stream)
{
    fclose((FILE*)opaque);
    (void)stream;
    return 0;
}

bool CZipFile::open(FILE *pf, const string& strPwd)
{
    return _open(pf, (void*)zread_file, (void*)ztell_file, (void*)zseek_file, (void*)zclose_file, strPwd);
}

static uLong ZCALLBACK zread_ins(voidpf opaque, voidpf stream, void* buf, uLong size)
{
    (void)stream;

    return ((Instream*)opaque)->read(buf, size);
    /*if (!((Instream*)opaque)->readex(buf, size))
    {
        return 0;
    }
    return size;*/
}

static long ZCALLBACK ztell_ins(voidpf opaque, voidpf stream)
{
    (void)stream;
    return (long)((Instream*)opaque)->pos();
}

static long ZCALLBACK zseek_ins(voidpf opaque, voidpf stream, uLong offset, int origin)
{
    (void)stream;

    if (!((Instream*)opaque)->seek(offset, origin))
    {
        return -1;
    }
    return 0;
}

static int ZCALLBACK zclose_ins(voidpf opaque, voidpf stream)
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

    return _open(&ins, (void*)zread_ins, (void*)ztell_ins, (void*)zseek_ins, (void*)zclose_ins, strPwd);
}

static uLong ZCALLBACK zread_ifs(voidpf opaque, voidpf stream, void* buf, uLong size)
{
    (void)stream;
    return ((IFStream*)opaque)->read(buf, size);
}

static long ZCALLBACK ztell_ifs(voidpf opaque, voidpf stream)
{
    (void)stream;
    return (long)((IFStream*)opaque)->pos();
}

static long ZCALLBACK zseek_ifs(voidpf opaque, voidpf stream, uLong offset, int origin)
{
    (void)stream;

    if (!((IFStream*)opaque)->seek(offset, origin))
    {
        return -1;
    }
    return 0;
}

static int ZCALLBACK zclose_ifs(voidpf opaque, voidpf stream)
{
    (void)stream;

    ((IFStream*)opaque)->close();
    return 0;
}

bool CZipFile::open(IFStream& ifs, const string& strPwd)
{
    if (!ifs.is_open())
    {
        return false;
    }

    return _open(&ifs, (void*)zread_ifs, (void*)ztell_ifs, (void*)zseek_ifs, (void*)zclose_ifs, strPwd);
}

long CZipFile::unzip(const tagUnzSubFile& unzSubFile, cwstr strDstFile) const
{
    if (!unzOpen(unzSubFile))
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

bool CZipFile::unzip(cwstr strDstDir) const
{
    if (NULL == m_pfile)
	{
		return false;
	}

	if (!fsutil::createDir(strDstDir))
	{
		return false;
	}

    auto t_strDstDir(strDstDir);
    if (!fsutil::checkPathTail(t_strDstDir))
	{
        t_strDstDir.push_back(__cPathSeparator);
	}

    (void)unzGoToFirstFile(m_pfile);

    for (cauto pr : m_mapSubDir)
    {
        if (!fsutil::createDir((t_strDstDir + pr.first).c_str()))
        {
            return false;
        }
    }

    for (cauto pr : m_mapSubfile)
    {
        auto& unzSubFile = *pr.second;
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

        m_strPwd.clear();
        m_root.clear();
        m_mapSubDir.clear();
        m_mapSubfile.clear();
    }
}

static void EnumDirFiles(const string& dirPrefix,const string& dirName,vector<string>& vFiles)
{
    if (dirPrefix.empty() || dirName.empty())
        return;
    string dirNameTmp = dirName;
    string dirPre = dirPrefix;

    if (dirNameTmp.find_last_of(__cPathSeparator) != dirNameTmp.length() - 1)
        dirNameTmp.push_back(__cPathSeparator);
    if (dirNameTmp[0] == __cPathSeparator)
        dirNameTmp = dirNameTmp.substr(1);
    if (dirPre.find_last_of(__cPathSeparator) != dirPre.length() - 1)
        dirPre.push_back(__cPathSeparator);

    string path;

    path = dirPre + dirNameTmp;


    struct stat fileStat;
    DIR* pDir = opendir(path.c_str());
    if (!pDir) return;

    struct dirent* pDirEnt = NULL;
    while ( (pDirEnt = readdir(pDir)) != NULL )
    {
        if (strcmp(pDirEnt->d_name,".") == 0 || strcmp(pDirEnt->d_name,"..") == 0)
            continue;

        string tmpDir = dirPre + dirNameTmp + pDirEnt->d_name;
        if (stat(tmpDir.c_str(),&fileStat) != 0)
            continue;

        string innerDir = dirNameTmp + pDirEnt->d_name;
        if ((fileStat.st_mode & S_IFDIR) == S_IFDIR)
        {
            EnumDirFiles(dirPrefix,innerDir,vFiles);
            continue;
        }

        vFiles.push_back(innerDir);
    }

    if (pDir)
        closedir(pDir);
}

static int WriteInZipFile(zipFile zFile,const string& file)
{
    fstream f(file.c_str(),std::ios::binary | std::ios::in);
    f.seekg(0, std::ios::end);
    auto size = (unsigned long)f.tellg();
    f.seekg(0, std::ios::beg);
    if ( size <= 0 )
    {
        return zipWriteInFileInZip(zFile,NULL,0);
    }
    char* buf = new char[size];
    f.read(buf,size);
    int ret = zipWriteInFileInZip(zFile,buf,size);
    delete[] buf;
    return ret;
}

static int Minizip(string src, const string& dest, E_ZMethod method = E_ZMethod::ZM_Deflated, int level = 0)
{
    if (src.find_last_of(__cPathSeparator) == src.length() - 1)
        src = src.substr(0,src.length()-1);

	int t_method = 0;
	if (E_ZMethod::ZM_Deflated == method)
	{
		t_method = Z_DEFLATED;
	}
	else if (E_ZMethod::ZM_Deflated == method)
	{
		t_method = Z_BZIP2ED;
	}

    struct stat fileInfo;
    stat(src.c_str(), &fileInfo);
    if (S_ISREG(fileInfo.st_mode)) {
        zipFile zFile = zipOpen(dest.c_str(),APPEND_STATUS_CREATE);
        if (zFile == NULL) {
            //cout<<"openfile failed"<<endl;
            return -1;
        }

        zip_fileinfo zFileInfo;
        memset(&zFileInfo, 0, sizeof (zFileInfo));
        int ret = zipOpenNewFileInZip(zFile,src.c_str(),&zFileInfo,NULL,0,NULL,0,NULL,t_method,level);
        if (ret != ZIP_OK) {
            //cout<<"openfile in zip failed"<<endl;
            zipClose(zFile, NULL);
            return -1;
        }

        ret = WriteInZipFile(zFile,src);
        if (ret != ZIP_OK) {
            //cout<<"write in zip failed"<<endl;
            zipClose(zFile,NULL);
            return -1;
        }
        zipClose(zFile, NULL);
        //cout<<"zip ok"<<endl;
    }
    else if (S_ISDIR(fileInfo.st_mode)) {
        zipFile zFile = zipOpen(dest.c_str(), APPEND_STATUS_CREATE);
        if (zFile == NULL) {
            //cout<<"openfile failed"<<endl;
            return -1;
        }

		size_t pos = src.find_last_of(__cPathSeparator);
		string dirName = src.substr(pos + 1);
		string dirPrefix = src.substr(0, pos);
        vector<string> vFiles;
        EnumDirFiles(dirPrefix, dirName, vFiles);

        for (cauto strFile : vFiles)
		{
			zip_fileinfo zFileInfo;
            memset(&zFileInfo, 0, sizeof (zFileInfo));

			auto t_strFile = strFile;
			t_strFile.erase(0, t_strFile.find(__cPathSeparator)+1);
			strutil::replaceChar(t_strFile, '\\', '/');
            int ret = zipOpenNewFileInZip(zFile,t_strFile.c_str(),&zFileInfo,NULL,0,NULL,0,NULL,t_method,level);
            if (ret != ZIP_OK) {
                //cout<<"openfile in zip failed"<<endl;
                zipClose(zFile,NULL);
                return -1;
            }
            ret = WriteInZipFile(zFile, dirPrefix + __cPathSeparator + strFile);
            if (ret != ZIP_OK) {
                //cout<<"write in zip failed"<<endl;
                zipClose(zFile,NULL);
                return -1;
            }
        }

        zipClose(zFile,NULL);
    }
    return 0;
}

bool ziputil::zipDir(const string& strSrcDir, const string& strDstFile, E_ZMethod method, int level)
{
	//(void)::remove(strDstFile.c_str());
	auto nRet = Minizip(strSrcDir, strDstFile, method, level);
	if (nRet != 0)
	{
		return false;
	}
	return true;
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
	uLongf destLen = len * 2;
	auto ptr = bbfBuff.resizeMore(destLen);
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

int ziputil::zUncompress(const void* pData, size_t len, CByteBuffer& bbfBuff)
{
    uLongf destLen = len * 3;
	auto ptr = bbfBuff.resizeMore(destLen);
	int nRet = uncompress(ptr, &destLen, (const Bytef*)pData, len);
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
