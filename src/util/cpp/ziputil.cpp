
#include "util.h"

extern "C"
{
#include "zlib.h"
#include "contrib/minizip/unzip.h"
#include "contrib/minizip/zip.h"
}

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

inline bool CUnZip::_unzOpen() const
{
	if (NULL == m_pUnz)
	{
		return false;
	}

    int nRet = 0;
    if (m_strPwd.empty())
    {
        nRet = unzOpenCurrentFile(m_pUnz);
    }
    else
    {
        nRet = unzOpenCurrentFilePassword(m_pUnz, m_strPwd.c_str());
    }
    if (nRet != UNZ_OK)
    {
        return false;
    }

    return true;
}

bool CUnZip::unzOpen(const tagUnzFile& unzFile) const
{
	if (NULL == m_pUnz)
	{
		return false;
	}

    unz64_file_pos file_pos { unzFile.pos_in_zip_directory, unzFile.num_of_file };
    int nRet = unzGoToFilePos64(m_pUnz, &file_pos);
    if (nRet != UNZ_OK)
    {
        return false;
    }

    if (!_unzOpen())
    {
        return false;
    }

   ((tagUnzFile&)unzFile).data_pos = unzGetCurrentFileZStreamPos64(m_pUnz);

    return true;
}

#define _unzRead(buf, len) unzReadCurrentFile(m_pUnz, buf, len)

long CUnZip::unzRead(void *buf, size_t len) const
{
	if (NULL == m_pUnz)
	{
		return -1;
	}

    return _unzRead(buf, len);
}

#define _unzClose() (void)unzCloseCurrentFile(m_pUnz)

void CUnZip::unzClose()
{
	if (NULL == m_pUnz)
	{
		return;
	}

    _unzClose();
}

long CUnZip::_read(const tagUnzFile& unzFile, void *buf, size_t len) const
{
	if (NULL == m_pUnz)
	{
		return -1;
	}

    if (!unzOpen(unzFile))
	{
		return -1;
	}

    auto nCount = _unzRead(buf, len);
    _unzClose();

    return nCount;
}

bool CUnZip::_open(const char *szFile, void *zfunc, const string& strPwd)
{
    m_strPwd = strPwd;

    unzFile pUnz = NULL;
    if (zfunc)
	{
        pUnz = ::unzOpen2_64(szFile, (zlib_filefunc64_def*)zfunc);
	}
	else
	{
        pUnz = ::unzOpen64(szFile);
	}
    if (NULL == pUnz)
    {
		return false;
    }

    unz_file_info64 file_info;
	memzero(file_info);

	char pszFileName[MAX_PATH + 1];
	memzero(pszFileName);

    unz64_file_pos file_pos{ 0,0 };

	do {
        int nRet = unzGetCurrentFileInfo64(pUnz, &file_info, pszFileName, MAX_PATH, NULL, 0, NULL, 0);
		if (nRet != UNZ_OK)
		{
            (void)::unzClose(pUnz);
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
            nRet = unzGetFilePos64(pUnz, &file_pos);
			if (nRet != UNZ_OK)
			{
                (void)::unzClose(pUnz);
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
    } while (unzGoToNextFile(pUnz) == UNZ_OK);

    (void)unzGoToFirstFile(pUnz);
	
	m_pUnz = pUnz;

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

bool CUnZip::_open(void *opaque, void *zread, void *ztell, void *zseek, void *zclose, const string& strPwd)
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

bool CUnZip::open(FILE *pf, const string& strPwd)
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

bool CUnZip::open(Instream& ins, const string& strPwd)
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

bool CUnZip::open(IFStream& ifs, const string& strPwd)
{
    if (!ifs.is_open())
    {
        return false;
    }

    tell64_file_func ztell64_ifs = ztell_ifs;
    seek64_file_func zseek64_ifs = zseek_ifs;
    return _open(&ifs, (void*)zread_ifs, (void*)ztell64_ifs, (void*)zseek64_ifs, (void*)zclose_ifs, strPwd);
}

long CUnZip::unzip(const tagUnzFile& unzFile, cwstr strDstFile) const
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

bool CUnZip::unzipAll(cwstr strDstDir) const
{
    if (NULL == m_pUnz)
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

    (void)unzGoToFirstFile(m_pUnz);

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
        (void)unzGoToNextFile(m_pUnz);
	}

	return true;
}

void CUnZip::close()
{
	if (NULL == m_pUnz)
	{
		return;
	}

    (void)::unzClose(m_pUnz);
	m_pUnz = NULL;

    m_strPwd.clear();
    m_root.clear();
    m_lstSubDir.clear();
    m_lstSubFile.clear();
}

#if __winvc
#include "dirent.h" //非sdk
#else
#include <dirent.h>
#endif

static void _EnumDirFiles(const string& dirPrefix,const string& dirName,list<string>& lstFiles)
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

    string path = dirPre + dirNameTmp;

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
            _EnumDirFiles(dirPrefix,innerDir,lstFiles);
            continue;
        }

        lstFiles.push_back(innerDir);
    }

    if (pDir)
        closedir(pDir);
}

//#include <fstream>
static int _WriteInZipFile(zipFile zFile,const string& file)
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
		fclose(pf);
        return zipWriteInFileInZip(zFile,NULL,0);
    }

    char* buf = new char[size];
	if (NULL == buf) // TODO 文件太大
	{
		fclose(pf);
		return -1;
	}

    //f.read(buf,size);
    (void)fread(buf, size, 1, pf);
	fclose(pf);
    int ret = zipWriteInFileInZip(zFile,buf,size);
    delete[] buf;
    return ret;
}

//minizip的zipOpenNewFileInZip函数改造，加上密码参数
#ifndef VERSIONMADEBY
# define VERSIONMADEBY   (0x0) /* platform depedent */
#endif
static int _zipOpenNewFileInZip(zipFile file, const char* filename, const zip_fileinfo* zipfi,
	const void* extrafield_local, uInt size_extrafield_local,
	const void*extrafield_global, uInt size_extrafield_global,
	const char* comment, int method, int level, const char *password)
{
	return zipOpenNewFileInZip4_64(file, filename, zipfi,
		extrafield_local, size_extrafield_local,
		extrafield_global, size_extrafield_global,
		comment, method, level, 0,
		-MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
		password, 0, VERSIONMADEBY, 0, 0);
}

static int _MinizipFile(zipFile zFile, const tagZipSrc& src, const string& strPwd)
{
	int method = 0;
	if (E_ZMethod::ZM_Deflated == src.method)
	{
		method = Z_DEFLATED;
	}
	else if (E_ZMethod::ZM_BZip2ed == src.method)
	{
		method = Z_BZIP2ED;
	}

    zip_fileinfo zfi;
    memset(&zfi, 0, sizeof (zfi));
	auto password = !strPwd.empty() ? strPwd.c_str() : NULL;
    int ret = _zipOpenNewFileInZip(zFile,src.strInnerPath.c_str(),&zfi,NULL,0,NULL,0,NULL,method,src.level, password);
    if (ret != ZIP_OK) {
        return ret;
    }

    ret = _WriteInZipFile(zFile,src.strFile);
    if (ret != ZIP_OK) {
        return ret;
    }
    return ZIP_OK;
}

//APPEND_STATUS_CREATE 新建zip包
//APPEND_STATUS_CREATEAFTER 新建zip包到现有的非zip文件末尾
// APPEND_STATUS_ADDINZIP 打开现有的zip包
bool CZip::open(const string& strFile, const string& strPwd)
{
	m_pZip = zipOpen64(strFile.c_str(), APPEND_STATUS_CREATE);
	if (m_pZip == NULL) {
		return false;
	}
	m_strPwd = strPwd;
	return true;
}

void CZip::close()
{
	if (m_pZip)
	{
		zipClose((zipFile*)m_pZip, NULL);
	}
}

int CZip::zDir(bool bKeetRoot, const string& src, E_ZMethod method, int level)
{
	if (NULL == m_pZip)
	{
		return -1;
	}

	string dirName;
	size_t pos = src.find_last_of("\\/");
	if (pos == (src.length() - 1)) {
		pos = src.find_last_of("\\/", pos - 1);
		dirName = src.substr(pos + 1);
		dirName.pop_back();
	}
	else
	{
		dirName = src.substr(pos + 1);
	}
	string dirPrefix = src.substr(0, pos);

	list<string> lstFiles;
	_EnumDirFiles(dirPrefix, dirName, lstFiles);

	int ret = 0;
	for (cauto strFile : lstFiles)
	{
		auto strInnerPath = strFile;
		if (!bKeetRoot)
		{
			strInnerPath.erase(0, strInnerPath.find(__cPathSeparator) + 1);
		}
		strutil::replaceChar(strInnerPath, '\\', '/');

		tagZipSrc zipSrc(dirPrefix + __cPathSeparator + strFile, strInnerPath, method, level));
		ret = _MinizipFile(m_pZip, zipSrc, m_strPwd);
		if (ret != ZIP_OK) {
			break;
		}
	}

	return ret;
}

int CZip::zFiles(const list<tagZipSrc>& lstSrc)
{
	if (NULL == m_pZip)
	{
		return -1;
	}
	
	int ret = 0;
	for (cauto src : lstSrc)
	{
		ret = _MinizipFile(m_pZip, src, m_strPwd);
		if (ret != ZIP_OK) {
			break;
		}
	}
	return ret;
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
