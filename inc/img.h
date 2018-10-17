
#pragma once

#include <atlimage.h>

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

class __CommonPrjExt CImg : public CImage
{
public:
	CImg(COLORREF crBkgrd= __Color_White)
		: m_crBkgrd(crBkgrd)
	{
	}

	~CImg()
	{
		RestoreMemDC();

		if (NULL )
		CDC m_MemDC;
		CBitmap m_MemBitmap;

	}

private:
	COLORREF m_crBkgrd = 0;

	E_ImgFixMode m_eFixMode = E_ImgFixMode::IFM_Inner;
	
	bool m_bHalfToneMode = false;

	UINT m_cx = 0;
	UINT m_cy = 0;
	CRect m_rcDst;

	CDC m_MemDC;
	CBitmap m_MemBitmap;
	
	CBitmap *m_pbmpPrev = NULL;

private:
	void RestoreMemDC();

public:
	BOOL Load(const wstring& strFile);

	CDC *GetDC();

	BOOL InitMemDC(E_ImgFixMode eFixMode, bool bHalfToneMode, UINT cx, UINT cy, LPCRECT prcMargin=NULL);

	CDC& GetMemDC()
	{
		return m_MemDC;
	}

	BOOL LoadEx(const wstring& strFile, const function<E_ImgFixMode(UINT uWidth, UINT uHeight)>& cb=NULL);

	BOOL StretchBltFix(E_ImgFixMode eFixMode, CDC& dcTarget, const CRect& rcTarget, bool bHalfToneMode, LPCRECT prcMargin = NULL);

	BOOL StretchBltEx(CDC& dcTarget, const CRect& rcTarget);

	BOOL StretchBltEx(CImg& imgTarget);

	CBitmap& FetchMemBitmap();
};

enum class E_ImglstType
{
	ILT_Both = -1
	, ILT_Normal = 0
	, ILT_Small = LVSIL_SMALL
};

class __CommonPrjExt CImglst : public CImageList
{
public:
	CImglst()
	{
	}

private:
	UINT m_cx = 0;
	UINT m_cy = 0;

	CDC m_MemDC;
	CBitmap m_MemBitmap;

public:
	BOOL Init(UINT cx, UINT cy);

	BOOL Init(const CSize& size, const TD_IconVec& vecIcons = {});

	BOOL Init(CBitmap& bitmap);

	BOOL SetFile(const wstring& strFile, bool bHalfToneMode, LPCRECT prcMargin = NULL, int iPosReplace = -1);

	void SetImg(CImg& img, bool bHalfToneMode, LPCRECT prcMargin, int iPosReplace);
	
	void SetBitmap(CBitmap& bitmap, int iPosReplace = -1);

	void SetIcon(HICON hIcon, int iPosReplace = -1);

	void SetToListCtrl(CListCtrl &wndListCtrl, E_ImglstType eImglstType)
	{
		if (E_ImglstType::ILT_Both == eImglstType)
		{
			(void)wndListCtrl.SetImageList(this, (int)E_ImglstType::ILT_Normal);
			(void)wndListCtrl.SetImageList(this, (int)E_ImglstType::ILT_Small);
		}
		else
		{
			(void)wndListCtrl.SetImageList(this, (int)eImglstType);
		}
	}

	void SetToTreeCtrl(CTreeCtrl &wndTreeCtrl)
	{
		(void)wndTreeCtrl.SetImageList(this, TVSIL_NORMAL);
	}
};
