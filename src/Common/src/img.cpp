
#include "StdAfx.h"

#include <img.h>

void CCompDC::getBitmap(const function<void(CBitmap&)>& cb)
{
	if (m_CompBitmap.m_hObject)
	{
		if (NULL != m_hbmpPrev)
		{
			(void)m_CompDC.SelectObject(m_hbmpPrev);
			m_hbmpPrev = NULL;
		}

		cb(m_CompBitmap);

		m_hbmpPrev = (HBITMAP)m_CompDC.SelectObject(m_CompBitmap);
	}
}

bool CCompDC::create(CDC *pDC, UINT cx, UINT cy)
{
	__AssertReturn(m_CompDC.CreateCompatibleDC(pDC), false);

	__AssertReturn(m_CompBitmap.CreateCompatibleBitmap(pDC, cx, cy), FALSE);

	m_hbmpPrev = (HBITMAP)m_CompDC.SelectObject(m_CompBitmap);

	m_prevBrush = m_CompDC.SelectObject(pDC->GetCurrentBrush());
	m_prevPen = m_CompDC.SelectObject(pDC->GetCurrentPen());
	m_prevFont = m_CompDC.SelectObject(pDC->GetCurrentFont());

	m_cx = cx;
	m_cy = cy;

	return true;
}

bool CCompDC::create(CDC *pDC, HBITMAP hBitmap)
{
	__AssertReturn(m_CompDC.CreateCompatibleDC(pDC), false);

	m_hbmpPrev = (HBITMAP)m_CompDC.SelectObject(hBitmap);

	BITMAP bitmap;
	GetObject(hBitmap, sizeof(BITMAP), &bitmap);

	m_cx = bitmap.bmWidth;
	m_cy = bitmap.bmHeight;

	return true;
}

bool CCompDC::create(CDC *pDC, HICON hIcon)
{
	ICONINFO iconInfo;
	GetIconInfo(hIcon, &iconInfo);

	if (!create(pDC, iconInfo.hbmColor))
	{
		return false;
	}

	::DeleteObject(iconInfo.hbmColor);
	::DeleteObject(iconInfo.hbmMask);

	return true;
}

void CCompDC::destroy()
{
	if (m_CompDC)
	{
		if (NULL != m_hbmpPrev)
		{
			(void)m_CompDC.SelectObject(m_hbmpPrev);
			m_hbmpPrev = NULL;
		}

		if (NULL != m_prevBrush)
		{
			(void)m_CompDC.SelectObject(m_prevBrush);
			m_prevBrush = NULL;
		}
		if (NULL != m_prevPen)
		{
			(void)m_CompDC.SelectObject(m_prevPen);
			m_prevPen = NULL;
		}
		if (NULL == m_prevFont)
		{
			(void)m_CompDC.SelectObject(m_prevFont);
			m_prevFont = NULL;
		}

		(void)m_CompDC.DeleteDC();
	}

	if (m_CompBitmap.m_hObject)
	{
		m_CompBitmap.DeleteObject();
	}
}

CImg::~CImg()
{
	Destroy();
}

BOOL CImg::StretchBltFix(E_ImgFixMode eFixMode, CDC& dcTarget, const CRect& rcTarget, bool bHalfToneMode, LPCRECT prcMargin)
{
	CRect rcDst(rcTarget);
	dcTarget.FillSolidRect(rcDst, m_crBkgrd);
	
	if (NULL != prcMargin)
	{
		rcDst.left = prcMargin->left;
		rcDst.top = prcMargin->top;
		rcDst.right -= prcMargin->right;
		rcDst.bottom -= prcMargin->bottom;
	}

	int nImgWidth = this->GetWidth();
	int nImgHeight = this->GetHeight();
	float fHWRate = (float)nImgHeight / nImgWidth;

	int nDstWidth = rcDst.Width();
	int nDstHeight = rcDst.Height();

	float fNeedHWRate = (float)rcDst.Height() / rcDst.Width();

	if (E_ImgFixMode::IFM_Inner == eFixMode)
	{
		if (fHWRate > fNeedHWRate)
		{
			eFixMode = E_ImgFixMode::IFM_Width;
		}
		else
		{
			eFixMode = E_ImgFixMode::IFM_Height;
		}
	}
	else if (E_ImgFixMode::IFM_Outer == eFixMode)
	{
		if (fHWRate > fNeedHWRate)
		{
			eFixMode = E_ImgFixMode::IFM_Height;
			
			nDstWidth = int(nDstHeight / fHWRate);
			auto offset = (rcDst.Width() - nDstWidth) / 2;
			rcDst.left += offset;
			rcDst.right -= offset;
		}
		else
		{
			eFixMode = E_ImgFixMode::IFM_Width;

			nDstHeight = int(nDstWidth * fHWRate);
			auto offset = (rcDst.Height() - nDstHeight) / 2;
			rcDst.top += offset;
			rcDst.bottom -= offset;
		}
	}

	CRect rcSrc;
	if (E_ImgFixMode::IFM_Width == eFixMode)
	{
		rcSrc.left = 0;
		rcSrc.right = nImgWidth;

		rcSrc.top = LONG(nImgHeight - nImgWidth*fNeedHWRate) / 2;
		rcSrc.top = MAX(rcSrc.top, 0);

		rcSrc.bottom = nImgHeight - rcSrc.top;
		rcSrc.bottom = MAX(rcSrc.bottom, 0);
	}
	else
	{
		rcSrc.top = 0;
		rcSrc.bottom = nImgHeight;

		rcSrc.left = LONG(nImgWidth - nImgHeight / fNeedHWRate) / 2;
		rcSrc.left = MAX(rcSrc.left, 0);
		
		rcSrc.right = nImgWidth - rcSrc.left;
		rcSrc.right = MAX(rcSrc.right, 0);
	}

	(void)dcTarget.SetStretchBltMode(bHalfToneMode ? STRETCH_HALFTONE : STRETCH_DELETESCANS);

	CDC *pdcThis = GetDC();

	BOOL bRet = dcTarget.StretchBlt(rcDst.left, rcDst.top, nDstWidth, nDstHeight
		, pdcThis, rcSrc.left, rcSrc.top, rcSrc.Width(), rcSrc.Height(), SRCCOPY);

	this->ReleaseDC();

	return bRet;
}

BOOL CImg::InitCompDC(E_ImgFixMode eFixMode, bool bHalfToneMode, UINT cx, UINT cy, LPCRECT prcMargin)
{
	m_eFixMode = eFixMode;

	m_cx = cx;
	m_cy = cy;
	m_rcDst = { 0, 0, (int)cx, (int)cy };

	m_bHalfToneMode = bHalfToneMode;

	CDC *pDC = CDC::FromHandle(::GetDC(NULL));
	__AssertReturn(pDC, FALSE);

	__AssertReturn(m_CompDC.create(pDC, cx, cy), FALSE);

	if (NULL != prcMargin)
	{
		m_CompDC->FillSolidRect(m_rcDst, __Color_White);

		m_rcDst.left = prcMargin->left;
		m_rcDst.top = prcMargin->top;
		m_rcDst.right -= prcMargin->right;
		m_rcDst.bottom -= prcMargin->bottom;
	}

	return TRUE;
}

BOOL CImg::Load(const wstring& strFile)
{
	Destroy();

	HRESULT hr = __super::Load(strFile.c_str());
	__EnsureReturn(S_OK == hr, FALSE);

	return TRUE;
}

BOOL CImg::LoadEx(const wstring& strFile, const function<E_ImgFixMode(UINT uWidth, UINT uHeight)>& cb)
{
	__EnsureReturn(Load(strFile), FALSE);

	E_ImgFixMode eFixMode = m_eFixMode;
	if (cb)
	{
		eFixMode = cb((UINT)CImage::GetWidth(), (UINT)CImage::GetHeight());
	}

	__EnsureReturn(StretchBltFix(eFixMode, m_CompDC.getDC(), m_rcDst, m_bHalfToneMode), FALSE);

	return TRUE;
}

BOOL CImg::StretchBltEx(CDC& dcTarget, const CRect& rcTarget)
{
	return dcTarget.StretchBlt(rcTarget.left, rcTarget.top, rcTarget.Width(), rcTarget.Height()
		, &m_CompDC.getDC(), 0, 0, m_cx, m_cy, SRCCOPY);
}

BOOL CImg::StretchBltEx(CImg& imgTarget)
{
	return StretchBltEx(imgTarget.m_CompDC.getDC(), CRect(0, 0, m_cx, m_cy));
}

BOOL CImglst::Init(UINT cx, UINT cy)
{
	__AssertReturn(Create(cx, cy, ILC_COLOR32, 0, 0), FALSE);

	m_cx = cx;
	m_cy = cy;

	CDC *pDC = CDC::FromHandle(::GetDC(NULL));
	__AssertReturn(pDC, FALSE);

	__AssertReturn(m_CompDC.create(pDC, cx, cy), FALSE);
	
	return TRUE;
}

BOOL CImglst::Init(const CSize& size, const TD_IconVec& vecIcons)
{
	__AssertReturn(Init(size.cx, size.cy), FALSE);

	for (auto hIcon : vecIcons)
	{
		(void)Add(hIcon);
	}

	return TRUE;
}

BOOL CImglst::Init(CBitmap& bitmap)
{
	BITMAP bmp;
	(void)bitmap.GetBitmap(&bmp);
	__AssertReturn(Init(bmp.bmHeight, bmp.bmHeight), FALSE);

	(void)Add(&bitmap, __Color_Black);

	return TRUE;
}

BOOL CImglst::SetFile(const wstring& strFile, bool bHalfToneMode, LPCRECT prcMargin, int iPosReplace)
{
	CImg img;
	__EnsureReturn(img.Load(strFile.c_str()), FALSE);

	SetImg(img, bHalfToneMode, prcMargin, iPosReplace);
	
	return TRUE;
}

void CImglst::SetImg(CImg& img, bool bHalfToneMode, LPCRECT prcMargin, int iPosReplace)
{
	img.StretchBltFix(E_ImgFixMode::IFM_Inner, m_CompDC.getDC(), CRect(0, 0, m_cx, m_cy), bHalfToneMode, prcMargin);

	m_CompDC.getBitmap([&](CBitmap& bitmap) {
		SetBitmap(bitmap, iPosReplace);
	});
}

void CImglst::SetBitmap(CBitmap& bitmap, int iPosReplace)
{
	if (iPosReplace >= 0)
	{
		(void)__super::Replace(iPosReplace, &bitmap, NULL);
	}
	else
	{
		(void)Add(&bitmap, __Color_Black);
	}
}

void CImglst::SetIcon(HICON hIcon, int iPosReplace)
{
	if (iPosReplace >= 0)
	{
		(void)Replace(iPosReplace, hIcon);
	}
	else
	{
		(void)Add(hIcon);
	}
}

void CImglst::SetToListCtrl(CListCtrl &wndListCtrl, E_ImglstType eImglstType)
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

HBITMAP CImglst::GetBitmap(UINT uPos, const function<void(CDC&)>& cb)
{
	IMAGEINFO ImageInfo;
	memset(&ImageInfo, 0, sizeof(ImageInfo));

	if (!GetImageInfo(uPos, &ImageInfo))
	{
		return NULL;
	}

	if (cb)
	{
		auto prev = m_CompDC->SelectObject(ImageInfo.hbmImage);

		cb(m_CompDC.getDC());

		(void)m_CompDC->SelectObject(prev);
	}

	return ImageInfo.hbmImage;
}
