
#include "StdAfx.h"

#include "Common/img.h"

void CCompDC::getBitmap(cfn_void_t<CBitmap&> cb)
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

bool CCompDC::_create(HDC hDC)
{
	if (NULL == hDC)
	{
		hDC = ::GetDC(NULL);
	}

	HDC hCompDC = ::CreateCompatibleDC(hDC);
	__AssertReturn(hCompDC, false);

	if (!m_CompDC.Attach(hCompDC))
	{
		(void)::DeleteDC(hCompDC);
		return false;
	}

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
		(void)m_CompDC.DeleteDC();
		return false;
	}
	if (!m_CompBitmap.Attach(hCompBitmap))
	{
		(void)::DeleteObject(hCompBitmap);
		return false;
	}
	m_hbmpPrev = (HBITMAP)m_CompDC.SelectObject(m_CompBitmap);

	//__AssertReturn(m_CompBitmap.CreateCompatibleBitmap(pDC, cx, cy), false);
	
	//m_prevBrush = m_CompDC.SelectObject(pDC->GetCurrentBrush());
	//m_prevPen = m_CompDC.SelectObject(pDC->GetCurrentPen());
	//m_prevFont = m_CompDC.SelectObject(pDC->GetCurrentFont());

	m_cx = cx;
	m_cy = cy;

	return true;
}

bool CCompDC::create(HBITMAP hBitmap, HDC hDC)
{
	__EnsureReturn(_create(hDC), false);

	m_hbmpPrev = (HBITMAP)m_CompDC.SelectObject(hBitmap);

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

	::DeleteObject(iconInfo.hbmColor);
	::DeleteObject(iconInfo.hbmMask);

	return true;
}

void CCompDC::destroy()
{
	if (m_CompDC)
	{
		/*if (NULL != m_hbmpPrev)
		{
			(void)m_CompDC.SelectObject(m_hbmpPrev);
			m_hbmpPrev = NULL;
		}*/

		/*if (NULL != m_prevBrush)
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
		}*/

		(void)m_CompDC.DeleteDC();
	}

	if (m_CompBitmap.m_hObject)
	{
		m_CompBitmap.DeleteObject();
	}
}

BOOL CImg::Load(const wstring& strFile)
{
	Destroy();
	
	HRESULT hr = __super::Load(strFile.c_str());
	__EnsureReturn(S_OK == hr, FALSE);

	return TRUE;
}

/*for (int x = 0; x < cx; x++)
{
	for (int y = 0; y < cy; y++)
	{
		byte *pByte = (byte *)GetPixelAddress(x, y);
		pByte[0] = pByte[0] * pByte[3] / 255;
		pByte[1] = pByte[1] * pByte[3] / 255;
		pByte[2] = pByte[2] * pByte[3] / 255;
	}
}*/

BOOL CImg::DrawEx(HDC hDC, const RECT& rc)
{
	return CImage::Draw(hDC, rc, Gdiplus::InterpolationMode::InterpolationModeHighQualityBicubic);
}

BOOL CImg::DrawEx(HDC hDC, const RECT& rc, bool bHalfToneMode)
{
	(void)::SetStretchBltMode(hDC, bHalfToneMode ? STRETCH_HALFTONE : STRETCH_DELETESCANS);
	return CImage::StretchBlt(hDC, rc, SRCCOPY);
}

BOOL CImg::DrawEx(HDC hDC, const RECT& rc, E_ImgFixMode eFixMode, bool bHalfToneMode)
{
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
	
	(void)::SetStretchBltMode(hDC, bHalfToneMode ? STRETCH_HALFTONE : STRETCH_DELETESCANS);
	BOOL bRet = ::StretchBlt(hDC, rcDst.left, rcDst.top, nDstWidth, nDstHeight
		, GetDC(), rcSrc.left, rcSrc.top, rcSrc.Width(), rcSrc.Height(), SRCCOPY);
	this->ReleaseDC();

	return bRet;
}

BOOL CImglst::Init(UINT cx, UINT cy)
{
	__AssertReturn(Create(cx, cy, ILC_COLOR32, 0, 0), FALSE);

	SetBkColor(CLR_NONE);

	__AssertReturn(m_CompDC.create(cx, cy), FALSE);
	
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

BOOL CImglst::SetFile(const wstring& strFile, bool bHalfToneMode, LPCRECT prcMargin, int nPosReplace)
{
	CImg img;
	__EnsureReturn(img.Load(strFile.c_str()), FALSE);

	SetImg(img, bHalfToneMode, prcMargin, nPosReplace);
	
	return TRUE;
}

void CImglst::SetImg(CImg& img, bool bHalfToneMode, LPCRECT prcMargin, int nPosReplace)
{
	CRect rc(0, 0, m_cx, m_cy);
	m_CompDC.getDC().FillSolidRect(rc, __crWhite);

	if (prcMargin)
	{
		rc.left += prcMargin->left;
		rc.top += prcMargin->top;
		rc.right -= prcMargin->right;
		rc.bottom -= prcMargin->bottom;
	}
	img.DrawEx(m_CompDC.getDC(), rc, E_ImgFixMode::IFM_Inner, bHalfToneMode);

	m_CompDC.getBitmap([&](CBitmap& bitmap) {
		SetBitmap(bitmap, nPosReplace);
	});
}
// TODO CPngImage
void CImglst::SetBitmap(CBitmap& bitmap, int nPosReplace)
{
	if (nPosReplace >= 0)
	{
		(void)__super::Replace(nPosReplace, &bitmap, NULL);
	}
	else
	{
		(void)Add(&bitmap, CLR_NONE);
	}
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

HBITMAP CImglst::GetBitmap(UINT uPos, cfn_void_t<CDC&> cb)
{
	IMAGEINFO ImageInfo;
	memzero(ImageInfo);
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
