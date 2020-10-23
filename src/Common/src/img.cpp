
#include "StdAfx.h"

#include "Common/img.h"

void CCompDC::getBitmap(cfn_void_t<CBitmap&> cb)
{
	if (m_Bitmap.m_hObject)
	{
		if (m_hbmpPrev)
		{
			(void)m_dc.SelectObject(m_hbmpPrev);
			m_hbmpPrev = NULL;
		}

		cb(m_Bitmap);

		m_hbmpPrev = (HBITMAP)m_dc.SelectObject(m_Bitmap);
	}
}

bool CCompDC::_create(HDC hDC)
{
	if (NULL == hDC)
	{
		hDC = ::GetDC(NULL);
	}

	HDC hCompDC = ::CreateCompatibleDC(hDC);
	__AssertReturn(hCompDC, false);

	if (!m_dc.Attach(hCompDC))
	{
		(void)::DeleteDC(hCompDC);
		return false;
	}

	m_dc.SetBkMode(TRANSPARENT);

	return true;
}

bool CCompDC::create(UINT cx, UINT cy, HDC hDC)
{
	if (NULL == hDC)
	{
		hDC = ::GetDC(NULL);
	}
	__EnsureReturn(_create(hDC), false);

	HBITMAP hCompBitmap = ::CreateCompatibleBitmap(hDC, cx, cy);
	if (NULL == hCompBitmap)
	{
		(void)m_dc.DeleteDC();
		return false;
	}
	if (!m_Bitmap.Attach(hCompBitmap))
	{
		(void)::DeleteObject(hCompBitmap);
		return false;
	}
	m_hbmpPrev = (HBITMAP)m_dc.SelectObject(m_Bitmap);

	//__AssertReturn(m_Bitmap.CreateCompatibleBitmap(pDC, cx, cy), false);
	
	//m_prevBrush = m_dc.SelectObject(pDC->GetCurrentBrush());
	//m_prevPen = m_dc.SelectObject(pDC->GetCurrentPen());
	//m_prevFont = m_dc.SelectObject(pDC->GetCurrentFont());

	m_cx = cx;
	m_cy = cy;

	return true;
}

bool CCompDC::create(HBITMAP hBitmap, HDC hDC)
{
	__EnsureReturn(_create(hDC), false);

	m_hbmpPrev = (HBITMAP)m_dc.SelectObject(hBitmap);

	BITMAP bitmap;
	GetObject(hBitmap, sizeof(BITMAP), &bitmap);

	m_cx = bitmap.bmWidth;
	m_cy = bitmap.bmHeight;

	return true;
}

bool CCompDC::create(HICON hIcon, HDC hDC)
{
	ICONINFO iconInfo;
	GetIconInfo(hIcon, &iconInfo);

	if (!create(iconInfo.hbmColor, hDC))
	{
		return false;
	}
	
	return true;
}

void CCompDC::destroy()
{
	if (m_dc)
	{
		/*if (NULL != m_hbmpPrev)
		{
			(void)m_dc.SelectObject(m_hbmpPrev);
			m_hbmpPrev = NULL;
		}*/

		/*if (NULL != m_prevBrush)
		{
			(void)m_dc.SelectObject(m_prevBrush);
			m_prevBrush = NULL;
		}
		if (NULL != m_prevPen)
		{
			(void)m_dc.SelectObject(m_prevPen);
			m_prevPen = NULL;
		}
		if (NULL == m_prevFont)
		{
			(void)m_dc.SelectObject(m_prevFont);
			m_prevFont = NULL;
		}*/

		(void)m_dc.DeleteDC();
	}

	if (m_Bitmap.m_hObject)
	{
		m_Bitmap.DeleteObject();
	}
}

BOOL CImg::Load(cwstr strFile)
{
	Destroy();
	
	HRESULT hr = __super::Load(strFile.c_str());
	__EnsureReturn(S_OK == hr, FALSE);

	return TRUE;
}

BOOL CImg::StretchBltEx(HDC hDC, const RECT& rc, E_ImgFixMode eFixMode)
{
	(void)::SetStretchBltMode(hDC, STRETCH_HALFTONE);

	if (E_ImgFixMode::IFM_None == eFixMode)
	{
		return CImage::StretchBlt(hDC, rc, SRCCOPY);
	}

	CRect rcDst(rc);
	
	int cx = this->GetWidth();
	int cy = this->GetHeight();
	float fHWRate = (float)cy / cx;

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
		rcSrc.right = cx;

		rcSrc.top = LONG(cy - cx*fNeedHWRate) / 2;
		rcSrc.top = MAX(rcSrc.top, 0);

		rcSrc.bottom = cy - rcSrc.top;
		rcSrc.bottom = MAX(rcSrc.bottom, 0);
	}
	else
	{
		rcSrc.top = 0;
		rcSrc.bottom = cy;

		rcSrc.left = LONG(cx - cy / fNeedHWRate) / 2;
		rcSrc.left = MAX(rcSrc.left, 0);
		
		rcSrc.right = cx - rcSrc.left;
		rcSrc.right = MAX(rcSrc.right, 0);
	}
	
	return CImage::StretchBlt(hDC, rcDst.left, rcDst.top, nDstWidth, nDstHeight
		, rcSrc.left, rcSrc.top, rcSrc.Width(), rcSrc.Height(), SRCCOPY);
	
	/*BOOL bRet = ::StretchBlt(hDC, rcDst.left, rcDst.top, nDstWidth, nDstHeight
		, GetDC(), rcSrc.left, rcSrc.top, rcSrc.Width(), rcSrc.Height(), SRCCOPY);
	this->ReleaseDC();

	return bRet;*/
}

BOOL CImglst::Init(UINT cx, UINT cy)
{
	__AssertReturn(Create(cx, cy, ILC_COLOR32, 0, 0), FALSE);

	SetBkColor(m_crBkg);

	//__AssertReturn(m_adpDC.create(cx, cy), FALSE);
	
	m_cx = cx;
	m_cy = cy;

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

	(void)Add(&bitmap, CLR_NONE);

	return TRUE;
}

void CImglst::SetIcon(HICON hIcon, int nPosReplace)
{
	if (nPosReplace >= 0)
	{
		(void)Replace(nPosReplace, hIcon);
	}
	else
	{
		(void)Add(hIcon);
	}
}

void CImglst::SetBitmap(CBitmap& bitmap, int nPosReplace)
{
	/*auto bmp = Gdiplus::Bitmap::FromHBITMAP(bitmap, NULL);
	HICON hIcon = NULL;
	bmp->GetHICON(&hIcon);
	delete bmp;
	SetIcon(hIcon, nPosReplace);*/
	
	if (nPosReplace >= 0)
	{
		(void)__super::Replace(nPosReplace, &bitmap, NULL);
	}
	else
	{
		(void)Add(&bitmap, CLR_NONE);
	}
}

#define __initAdpDC() \
CRect rc(0, 0, m_cx, m_cy);\
m_adpDC.destroy();\
m_adpDC.create(m_cx, m_cy);\
if (m_crBkg != CLR_NONE) m_adpDC.getDC().FillSolidRect(rc, m_crBkg);

void CImglst::SetImg(Gdiplus::Image& img, int nPosReplace)
{
	__initAdpDC();

	Gdiplus::Graphics graphics(m_adpDC.getDC());
	graphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQuality);
	graphics.DrawImage(&img, rc.left, rc.top, rc.Width(), rc.Height());	

	m_adpDC.getBitmap([&](CBitmap& bitmap) {
		SetBitmap(bitmap, nPosReplace);
	});
}

void CImglst::SetImg(CImg& img, E_ImgFixMode eFixMode, int nPosReplace)
{
	__initAdpDC();

	img.StretchBltEx(m_adpDC.getDC(), rc, eFixMode);

	m_adpDC.getBitmap([&](CBitmap& bitmap) {
		SetBitmap(bitmap, nPosReplace);
	});
}
