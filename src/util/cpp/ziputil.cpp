
#include "util.h"

#if __winvc
#include "dirent.h"
#else
#include <dirent.h>
#endif

#include <sys/stat.h>
#include <sys/types.h>

extern "C"
{
#include "zlib.h"
#include "contrib/minizip/unzip.h"
#include "contrib/minizip/zip.h"
}

/*#if __windows
#define mkdir(x) _mkdir(x)
#else
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

    return strRet;
}

CUnzDir& CUnzDir::addSubDir(wstring strSubDir)
{
    wstring strName;
    auto pos = strSubDir.find('/');
    if (pos != __wnpos)
    {
        strName = strSubDir.substr(0, pos);
        strSubDir.erase(0, pos+1);
    }
    else
    {
        strName.swap(strSubDir);
    }

    auto& subDir = _addSubDir(strName);
    if (strSubDir.empty())
    {
        return subDir;
    }
    return subDir.addSubDir(strSubDir);
}

tagUnzFile& CUnzDir::addSubFile(wstring strSubFile)
{
    cauto strName = _subPath(strSubFile);
    if (strSubFile.empty())
    {
        return m_mapSubFile[strName];
    }

    return _addSubDir(strName).addSubFile(strSubFile);
}

const CUnzDir* CUnzDir::subDir(wstring strSubDir) const
{
    cauto strName = _subPath(strSubDir);
    auto subDir = _subDir(strName);
    if (NULL == subDir)
    {
        return NULL;
    }

    if (strSubDir.empty() || L"/" == strSubDir)
    {
        return subDir;
    }
    return subDir->subDir(strSubDir);
}

const tagUnzFile* CUnzDir::subFile(wstring strSubFile) const
{
    cauto strName = _subPath(strSubFile);
    if (strSubFile.empty())
    {
        auto itr = m_mapSubFile.find(strName);
        if (itr == m_mapSubFile.end())
        {
            return NULL;
        }
        return &itr->second;
    }

    auto subDir = _subDir(strName);
    if (NULL == subDir)
    {
        return NULL;
    }
    return subDir->subFile(strSubFile);
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

bool CZipFile::unzOpen(const tagUnzFile& unzFile) const
{
    unz64_file_pos file_pos { unzFile.pos_in_zip_directory, unzFile.num_of_file };
    int nRet = unzGoToFilePos64(m_pfile, &file_pos);
    if (nRet != UNZ_OK)
    {
        return false;
    }

    if (!_unzOpen())
    {
        return false;
    }

   ((tagUnzFile&)unzFile).data_pos = unzGetCurrentFileZStreamPos64(m_pfile);

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

long CZipFile::_read(const tagUnzFile& unzFile, void *buf, size_t len) const
{
    if (!unzOpen(unzFile))
	{
		return -1;
	}

    auto nCount = _unzRead(buf, len);
    _unzClose();

    return nCount;
}

bool CZipFile::_open(const char *szFile, void *zfunc, const string& strPwd)
{
    m_strPwd = strPwd;

    unzFile pfile = NULL;
    if (zfunc)
	{
        pfile = ::unzOpen2_64(szFile, (zlib_filefunc64_def*)zfunc);
	}
	else
	{
        pfile = ::unzOpen64(szFile);
	}
    if (NULL == pfile)
    {
		return false;
    }

    unz_file_info64 file_info;
	memzero(file_info);

	char pszFileName[MAX_PATH + 1];
	memzero(pszFileName);

    unz64_file_pos file_pos{ 0,0 };

	do {
        int nRet = unzGetCurrentFileInfo64(pfile, &file_info, pszFileName, MAX_PATH, NULL, 0, NULL, 0);
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
        auto strPath = strutil::checkUtf8(pszFileName)?
                    strutil::fromUtf8(pszFileName):strutil::fromGbk(pszFileName);
        if (__cSlant == strPath.back())  // (unzFile.external_fa & __DirFlag);
        {
            strPath.pop_back();
            m_lstSubDir.emplace_back(strPath, &m_root.addSubDir(strPath));
		}
		else
        {
            nRet = unzGetFilePos64(pfile, &file_pos);
			if (nRet != UNZ_OK)
			{
                (void)::unzClose(pfile);
				return false;
			}

            tagUnzFile& unzFile = m_root.addSubFile(strPath);
            unzFile.strPath = strPath;
            unzFile.compression_method = file_info.compression_method;
            unzFile.compressed_size = file_info.compressed_size;
            unzFile.uncompressed_size = file_info.uncompressed_size;

            unzFile.pos_in_zip_directory = file_pos.pos_in_zip_directory;
            unzFile.num_of_file = file_pos.num_of_file;

            m_lstSubFile.emplace_back(strPath, &unzFile);
		}
    } while (unzGoToNextFile(pfile) == UNZ_OK);

    (void)unzGoToFirstFile(pfile);
	
    m_pfile = pfile;

	return true;
}

static voidpf ZCALLBACK zopen_opaque(voidpf opaque, const void* filename, int mode)
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
    zlib_filefunc64_def zfunc;
    memzero(zfunc);
    zfunc.opaque = opaque;

    zfunc.zopen64_file = zopen_opaque;
    zfunc.zread_file = (decltype(zfunc.zread_file))zread;
    zfunc.ztell64_file = (decltype(zfunc.ztell64_file))ztell;
    zfunc.zseek64_file = (decltype(zfunc.zseek64_file))zseek;
    zfunc.zclose_file = (decltype(zfunc.zclose_file))zclose;
    zfunc.zerror_file = ztesterror_opaque;

    return _open("", &zfunc, strPwd);
}

static uLong ZCALLBACK zread_file(voidpf opaque, voidpf stream, void* buf, uLong size)
{
    (void)stream;
    return fread(buf, 1, size, (FILE*)opaque); //fread(buf, size, 1, (FILE*)opaque);
}

static ZPOS64_T ZCALLBACK ztell_file(voidpf opaque, voidpf stream)
{
    (void)stream;
    return ftell64((FILE*)opaque);
}

static long ZCALLBACK zseek_file(voidpf opaque, voidpf stream, ZPOS64_T offset, int origin)
{
    (void)stream;
    return fseek64((FILE*)opaque, offset, origin);
}

static int ZCALLBACK zclose_file(voidpf opaque, voidpf stream)
{
    fclose((FILE*)opaque);
    (void)stream;
    return 0;
}

bool CZipFile::open(FILE *pf, const string& strPwd)
{
    tell64_file_func ztell64_file = ztell_file;
    seek64_file_func zseek64_file = zseek_file;
    return _open(pf, (void*)zread_file, (void*)ztell64_file, (void*)zseek64_file, (void*)zclose_file, strPwd);
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

static ZPOS64_T ZCALLBACK ztell_ins(voidpf opaque, voidpf stream)
{
    (void)stream;
    return (ZPOS64_T)((Instream*)opaque)->pos();
}

static long ZCALLBACK zseek_ins(voidpf opaque, voidpf stream, ZPOS64_T offset, int origin)
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

    tell64_file_func ztell64_ins = ztell_ins;
    seek64_file_func zseek64_ins = zseek_ins;
    return _open(&ins, (void*)zread_ins, (void*)ztell64_ins, (void*)zseek64_ins, (void*)zclose_ins, strPwd);
}

static uLong ZCALLBACK zread_ifs(voidpf opaque, voidpf stream, void* buf, uLong size)
{
    (void)stream;
    return ((IFStream*)opaque)->read(buf, size);
}

static ZPOS64_T ZCALLBACK ztell_ifs(voidpf opaque, voidpf stream)
{
    (void)stream;
    return (ZPOS64_T)((IFStream*)opaque)->pos();
}

static long ZCALLBACK zseek_ifs(voidpf opaque, voidpf stream, ZPOS64_T offset, int origin)
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

    tell64_file_func ztell64_ifs = ztell_ifs;
    seek64_file_func zseek64_ifs = zseek_ifs;
    return _open(&ifs, (void*)zread_ifs, (void*)ztell64_ifs, (void*)zseek64_ifs, (void*)zclose_ifs, strPwd);
}

long CZipFile::unzip(const tagUnzFile& unzFile, cwstr strDstFile) const
{
    if (!unzOpen(unzFile))
    {
        return -1;
    }

    CByteBuffer bbfFile;
    int nCount = this->read(unzFile, bbfFile);
    if (nCount >= 0)
    {
        if (!OFStream::writefilex(strDstFile, true, bbfFile))
        {
            return -1;
        }
    }

    return nCount;
}

bool CZipFile::unzipAll(cwstr strDstDir) const
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

    for (cauto pr : m_lstSubDir)
    {
        if (!fsutil::createDir((t_strDstDir + pr.first).c_str()))
        {
            return false;
        }
    }

    for (cauto pr : m_lstSubFile)
    {
		auto& unzFile = *pr.second;
		OFStream ofs(t_strDstDir + unzFile.strPath, true);
		if (!ofs)
		{
			return false;
		}

        if (!_unzOpen())
        {
            return false;
        }

#define __unzipBuff 4094
		TD_ByteBuffer buff(__unzipBuff);
		int nCount = 0;
		do {
			nCount = _unzRead(buff, __unzipBuff);
			if (nCount <= 0)
			{
				break;
			}

			if (!ofs.writex(buff, (size_t)nCount))
			{
				_unzClose();
				return false;
			}
		} while (0);

		_unzClose();
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
        m_lstSubDir.clear();
        m_lstSubFile.clear();
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
    DIR *pDir = opendir(path.c_str());
    if (!pDir) return;

    struct dirent *pDirEnt = NULL;
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

//#include <fstream>
static int WriteInZipFile(zipFile zFile,const string& file)
{
    /*加DEFINES += _FILE_OFFSET_BITS=64后编译不过
    fstream f(file.c_str(),std::ios::binary | std::ios::in);
    f.seekg(0, std::ios::end);
    auto size = (unsigned long)f.tellg();
    f.seekg(0, std::ios::beg);*/

    auto pf = fsutil::fopen(file, "rb");
    if (NULL == pf)
    {
        return -1;
    }

    fseek(pf, 0, SEEK_END);
    auto size = ftell(pf);
    fseek(pf, 0, SEEK_SET);
    if (size <= 0)
    {
        return zipWriteInFileInZip(zFile,NULL,0);
    }

    char* buf = new char[size];
    //f.read(buf,size);
    (void)fread(buf, size, 1, pf);
    int ret = zipWriteInFileInZip(zFile,buf,size);
    delete[] buf;
    return ret;
}

static int Minizip(string src, const string& dest, E_ZMethod method = E_ZMethod::ZM_Deflated, int level = 0)
{
    if (src.find_last_of(__cPathSeparator) == (src.length() - 1)) {
        src = src.substr(0,src.length()-1);
    }

	int t_method = 0;
	if (E_ZMethod::ZM_Deflated == method)
	{
		t_method = Z_DEFLATED;
	}
	else if (E_ZMethod::ZM_BZip2ed == method)
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

int ziputil::zCompress(const void *pData, size_t len, CByteBuffer& bbfBuff, int level)
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

int ziputil::zUncompress(const void *pData, size_t len, CByteBuffer& bbfBuff)
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
