
#include "StdAfx.h"

#include <img.h>

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

	int iImgWidth = this->GetWidth();
	int iImgHeight = this->GetHeight();
	float fHWRate = (float)iImgHeight / iImgWidth;

	int iDstWidth = rcDst.Width();
	int iDstHeight = rcDst.Height();

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
			
			iDstWidth = int(iDstHeight / fHWRate);
			auto offset = (rcDst.Width() - iDstWidth) / 2;
			rcDst.left += offset;
			rcDst.right -= offset;
		}
		else
		{
			eFixMode = E_ImgFixMode::IFM_Width;

			iDstHeight = int(iDstWidth * fHWRate);
			auto offset = (rcDst.Height() - iDstHeight) / 2;
			rcDst.top += offset;
			rcDst.bottom -= offset;
		}
	}

	CRect rcSrc;
	if (E_ImgFixMode::IFM_Width == eFixMode)
	{
		rcSrc.left = 0;
		rcSrc.right = iImgWidth;

		rcSrc.top = LONG(iImgHeight - iImgWidth*fNeedHWRate) / 2;
		rcSrc.top = max(rcSrc.top, 0);

		rcSrc.bottom = iImgHeight - rcSrc.top;
		rcSrc.bottom = max(rcSrc.bottom, 0);
	}
	else
	{
		rcSrc.top = 0;
		rcSrc.bottom = iImgHeight;

		rcSrc.left = LONG(iImgWidth - iImgHeight / fNeedHWRate) / 2;
		rcSrc.left = max(rcSrc.left, 0);
		
		rcSrc.right = iImgWidth - rcSrc.left;
		rcSrc.right = max(rcSrc.right, 0);
	}

	(void)dcTarget.SetStretchBltMode(bHalfToneMode ? STRETCH_HALFTONE : STRETCH_DELETESCANS);

	CDC *pdcThis = GetDC();

	BOOL bRet = dcTarget.StretchBlt(rcDst.left, rcDst.top, iDstWidth, iDstHeight
		, pdcThis, rcSrc.left, rcSrc.top, rcSrc.Width(), rcSrc.Height(), SRCCOPY);

	this->ReleaseDC();

	return bRet;
}

CDC *CImg::GetDC()
{
	return CDC::FromHandle(__super::GetDC());
}

BOOL CImg::InitMemDC(E_ImgFixMode eFixMode, bool bHalfToneMode, UINT cx, UINT cy, LPCRECT prcMargin)
{
	m_eFixMode = eFixMode;

	m_cx = cx;
	m_cy = cy;
	m_rcDst = { 0, 0, (int)cx, (int)cy };

	m_bHalfToneMode = bHalfToneMode;

	CDC *pDC = CDC::FromHandle(::GetDC(NULL));
	__AssertReturn(pDC, FALSE);

	__AssertReturn(m_MemDC.CreateCompatibleDC(pDC), FALSE);

	__AssertReturn(m_MemBitmap.CreateCompatibleBitmap(pDC, m_rcDst.Width(), m_rcDst.Height()), FALSE);

	if (NULL != prcMargin)
	{
		m_MemDC.FillSolidRect(m_rcDst, __Color_White);

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

	if (NULL == m_pbmpPrev && m_MemBitmap.m_hObject)
	{
		m_pbmpPrev = (CBitmap*)m_MemDC.SelectObject(&m_MemBitmap);
	}

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

	__EnsureReturn(StretchBltFix(eFixMode, m_MemDC, m_rcDst, m_bHalfToneMode), FALSE);

	return TRUE;
}

BOOL CImg::StretchBltEx(CDC& dcTarget, const CRect& rcTarget)
{
	return dcTarget.StretchBlt(rcTarget.left, rcTarget.top, rcTarget.Width(), rcTarget.Height()
		, &m_MemDC, 0, 0, m_cx, m_cy, SRCCOPY);
}

BOOL CImg::StretchBltEx(CImg& imgTarget)
{
	return StretchBltEx(imgTarget.m_MemDC, CRect(0, 0, m_cx, m_cy));
}

void CImg::RestoreMemDC()
{
	if (NULL != m_pbmpPrev)
	{
		(void)m_MemDC.SelectObject(m_pbmpPrev);
		m_pbmpPrev = NULL;
	}
}

CBitmap& CImg::FetchMemBitmap()
{
	RestoreMemDC();
	return m_MemBitmap;
}

BOOL CImglst::Init(UINT cx, UINT cy)
{
	__AssertReturn(Create(cx, cy, ILC_COLOR32, 0, 0), FALSE);

	m_cx = cx;
	m_cy = cy;

	CDC *pDC = CDC::FromHandle(::GetDC(NULL));
	__AssertReturn(pDC, FALSE);

	__AssertReturn(m_MemDC.CreateCompatibleDC(pDC), FALSE);

	__AssertReturn(m_MemBitmap.CreateCompatibleBitmap(pDC, cx, cy), FALSE);
	
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
	CBitmap *pbmpPrev = (CBitmap*)m_MemDC.SelectObject(&m_MemBitmap);

	img.StretchBltFix(E_ImgFixMode::IFM_Inner, m_MemDC, CRect(0, 0, m_cx, m_cy), bHalfToneMode, prcMargin);

	(void)m_MemDC.SelectObject(pbmpPrev);

	SetBitmap(m_MemBitmap, iPosReplace);
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
