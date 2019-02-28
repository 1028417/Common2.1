
#pragma once

#include <atlimage.h>

class __CommonExt CCompDC
{
public:
	CCompDC()
	{
	}

	~CCompDC()
	{
		destroy();
	}

	UINT m_cx = 0;
	UINT m_cy = 0;

private:
	CDC m_CompDC;
	CBitmap m_CompBitmap;

	HBITMAP m_hbmpPrev = NULL;

public:
	CDC* operator ->()
	{
		return &m_CompDC;
	}

	CDC& getDC()
	{
		return m_CompDC;
	}

	void getBitmap(const function<void(CBitmap&)>& cb);

	bool create(CDC *pDC, UINT cx, UINT cy);

	bool create(CDC *pDC, HBITMAP hBitmap);

	bool create(CDC *pDC, HICON hIcon);

	void destroy();
};

#define __Color_White RGB(255, 255, 255)
#define __Color_Black ((COLORREF)0)

using TD_IconVec = vector<HICON>;

enum class E_ImgFixMode
{
	IFM_Inner
	, IFM_Outer
	, IFM_Width
	, IFM_Height
};

class __CommonExt CImg : public CImage
{
public:
	CImg(COLORREF crBkgrd= __Color_White)
		: m_crBkgrd(crBkgrd)
	{
	}

	~CImg();

private:
	COLORREF m_crBkgrd = 0;

	E_ImgFixMode m_eFixMode = E_ImgFixMode::IFM_Inner;
	
	bool m_bHalfToneMode = false;

	UINT m_cx = 0;
	UINT m_cy = 0;
	CRect m_rcDst;

	CCompDC m_CompDC;

public:
	CDC *GetDC()
	{
		return CDC::FromHandle(__super::GetDC());
	}

	CDC& GetCompDC()
	{
		return m_CompDC.getDC();
	}

	BOOL InitCompDC(E_ImgFixMode eFixMode, bool bHalfToneMode, UINT cx, UINT cy, LPCRECT prcMargin=NULL);

	BOOL Load(const wstring& strFile);

	BOOL LoadEx(const wstring& strFile, const function<E_ImgFixMode(UINT uWidth, UINT uHeight)>& cb=NULL);
	
	BOOL StretchBltFix(E_ImgFixMode eFixMode, CDC& dcTarget, const CRect& rcTarget, bool bHalfToneMode, LPCRECT prcMargin = NULL);

	BOOL StretchBltEx(CDC& dcTarget, const CRect& rcTarget);

	BOOL StretchBltEx(CImg& imgTarget);
};

enum class E_ImglstType
{
	ILT_Both = -1
	, ILT_Normal = 0
	, ILT_Small = LVSIL_SMALL
};

class __CommonExt CImglst : public CImageList
{
public:
	CImglst()
	{
	}

private:
	UINT m_cx = 0;
	UINT m_cy = 0;

	CCompDC m_CompDC;

public:
	BOOL Init(UINT cx, UINT cy);

	BOOL Init(const CSize& size, const TD_IconVec& vecIcons = {});

	BOOL Init(CBitmap& bitmap);

	BOOL SetFile(const wstring& strFile, bool bHalfToneMode, LPCRECT prcMargin = NULL, int iPosReplace = -1);

	void SetImg(CImg& img, bool bHalfToneMode, LPCRECT prcMargin, int iPosReplace);
	
	void SetBitmap(CBitmap& bitmap, int iPosReplace = -1);

	void SetIcon(HICON hIcon, int iPosReplace = -1);

	void SetToListCtrl(CListCtrl &wndListCtrl, E_ImglstType eImglstType);

	void SetToTreeCtrl(CTreeCtrl &wndTreeCtrl)
	{
		(void)wndTreeCtrl.SetImageList(this, TVSIL_NORMAL);
	}

	HBITMAP GetBitmap(UINT uPos, const function<void(CDC&)>& cb=NULL);
};
