
#include "StdAfx.h"

#include <ListCtrl.h>

BEGIN_MESSAGE_MAP(CHeader, CHeaderCtrl)
	ON_MESSAGE(HDM_LAYOUT, OnLayout)
END_MESSAGE_MAP()

LRESULT CHeader::OnLayout(WPARAM wParam, LPARAM lParam)
{
	LRESULT lResult = CHeaderCtrl::DefWindowProc(HDM_LAYOUT, 0, lParam);

	if (0 != m_uHeight)
	{
		HD_LAYOUT& hdl = *(HD_LAYOUT*)lParam;
		hdl.prc->top = hdl.pwpos->cy = m_uHeight;
	}

	return lResult;
}

BOOL CHeader::Init(UINT uHeight, UINT uFontSize)
{
	if (0 != uFontSize)
	{
		__EnsureReturn(m_fontGuard.setFontSize(*this, uFontSize), FALSE);
	}

	m_uHeight = uHeight;
	this->Invalidate();

	return TRUE;
}

// CObjectList

void CObjectList::PreSubclassWindow()
{
	__super::PreSubclassWindow();

	(void)ModifyStyle(0, LVS_SHOWSELALWAYS | LVS_SHAREIMAGELISTS);

	(void)SetExtendedStyle(__super::GetExtendedStyle()
		| LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP | LVS_EX_LABELTIP);
}

BOOL CObjectList::InitCtrl(const tagListPara& para)
{
	m_para = para;
	
	__EnsureReturn(InitFont(m_para.crText, m_para.uFontSize), FALSE);
	
	if (-1 != (int)m_para.eViewType)
	{
		SetView(m_para.eViewType);
	}

	if (!m_para.lstColumns.empty())
	{
		__EnsureReturn(InitColumn(m_para.lstColumns, m_para.setUnderlineColumns), FALSE);
	}

	if (0 != m_para.uHeaderHeight || 0 != m_para.uHeaderFontSize)
	{
		__EnsureReturn(InitHeader(m_para.uHeaderHeight, m_para.uHeaderFontSize), FALSE);
	}

	if (0 != m_para.uItemHeight)
	{
		__EnsureReturn(SetItemHeight(m_para.uItemHeight), FALSE);
	}

	if (0 != m_para.uTileWidth && 0 != m_para.uTileHeight)
	{
		SetTileSize(m_para.uTileWidth, m_para.uTileHeight);
	}

	if (m_para.cbCustomDraw)
	{
		m_bCusomDrawNotify = true;
	}
	
	if (m_para.cbViewChanged)
	{
		m_bAutoChange = true;
	}

	if (m_para.cbMouseEvent)
	{
		m_iTrackMouseFlag = 0;
	}

	return TRUE;
}

BOOL CObjectList::InitFont(COLORREF crText, UINT uFontSize)
{
	__AssertReturn(SetTextColor(crText), FALSE);

	if (0 != uFontSize)
	{
		__AssertReturn(m_fontGuard.setFontSize(*this, uFontSize), FALSE);
	}

	return TRUE;
}

BOOL CObjectList::InitImglst(const CSize& size, const CSize *pszSmall, const TD_IconVec& vecIcons)
{
	__AssertReturn(m_Imglst.Init(size, vecIcons), FALSE);
	(void)__super::SetImageList(&m_Imglst, LVSIL_NORMAL);

	if (NULL != pszSmall)
	{
		__AssertReturn(m_ImglstSmall.Init(*pszSmall, vecIcons), FALSE);
		(void)__super::SetImageList(&m_ImglstSmall, LVSIL_SMALL);
	}
	else
	{
		(void)__super::SetImageList(&m_Imglst, LVSIL_SMALL);
	}
	
	return TRUE;
}

BOOL CObjectList::InitImglst(CBitmap& Bitmap, CBitmap *pBitmapSmall)
{
	__AssertReturn(m_Imglst.Init(Bitmap), FALSE);
	(void)__super::SetImageList(&m_Imglst, LVSIL_NORMAL);

	if (NULL != pBitmapSmall)
	{
		__AssertReturn(m_ImglstSmall.Init(*pBitmapSmall), FALSE);
		(void)__super::SetImageList(&m_ImglstSmall, LVSIL_SMALL);
	}
	else
	{
		(void)__super::SetImageList(&m_Imglst, LVSIL_SMALL);
	}

	return TRUE;
}

void CObjectList::SetImageList(CImglst *pImglst, CImglst *pImglstSmall)
{
	if (NULL != pImglst)
	{
		(void)__super::SetImageList(pImglst, LVSIL_NORMAL);
	}

	if (NULL == pImglstSmall)
	{
		pImglstSmall = pImglst;
	}
	if (NULL != pImglstSmall)
	{
		(void)__super::SetImageList(pImglstSmall, LVSIL_SMALL);
	}
}

void CObjectList::SetView(E_ListViewType eViewType, bool bArrange)
{
	CRedrawLockGuard RedrawLockGuard(*this);

	m_para.eViewType = eViewType;
	(void)__super::SetView((int)eViewType);

	if (bArrange)
	{
		(void)Arrange(0);
	}
}

E_ListViewType CObjectList::GetView()
{
	if ((E_ListViewType)-1 != m_para.eViewType)
	{
		return m_para.eViewType;
	}

	m_para.eViewType = (E_ListViewType)__super::GetView();

	return m_para.eViewType;
}

BOOL CObjectList::InitColumn(const TD_ListColumn& lstColumns, const set<UINT>& setUnderlineColumns)
{
	m_uColumnCount = 0;
	for (auto& column : lstColumns)
	{
		(void)__super::InsertColumn(m_uColumnCount, (0 == column.uFormat) ? L' ' + column.cstrText
			: column.cstrText, column.uFormat, column.uWidth);

		m_uColumnCount++;
	}

	if (!setUnderlineColumns.empty())
	{
		__EnsureReturn(SetUnderlineColumn(setUnderlineColumns), FALSE);
	}

	return TRUE;
}

BOOL CObjectList::SetUnderlineColumn(const set<UINT>& setUnderlineColumns)
{
	m_para.setUnderlineColumns = setUnderlineColumns;

	__AssertReturn(m_fontUnderline.create(*this, [](LOGFONT& logFont) {
		logFont.lfUnderline = 1;
	}), FALSE);

	m_bCusomDrawNotify = true;

	return TRUE;
}

BOOL CObjectList::InitHeader(UINT uHeaderHeight, UINT uHeaderFontSize)
{
	if (!m_wndHeader)
	{
		__AssertReturn(m_wndHeader.SubclassWindow(CListCtrl::GetHeaderCtrl()->GetSafeHwnd()), FALSE);
		m_wndHeader.SetFont(this->GetFont());
	}

	__EnsureReturn(m_wndHeader.Init(uHeaderHeight, uHeaderFontSize), FALSE);
	
	return TRUE;
}

BOOL CObjectList::SetItemHeight(UINT uItemHeight)
{
	__AssertReturn(m_ImglstSmall.Create(1, uItemHeight, ILC_COLOR, 0, 0), FALSE);

	SetImageList(NULL, &m_ImglstSmall);
	
	return TRUE;
}

void CObjectList::SetTileSize(ULONG cx, ULONG cy)
{
	LVTILEVIEWINFO LvTileViewInfo = { sizeof(LVTILEVIEWINFO) };
	LvTileViewInfo.dwFlags = LVTVIF_FIXEDSIZE;
	LvTileViewInfo.sizeTile = { (LONG)cx, (LONG)cy };
	//LvTileViewInfo.cLines = 2;
	LvTileViewInfo.dwMask = LVTVIM_TILESIZE;// | LVTVIM_COLUMNS;
	(void)__super::SetTileViewInfo(&LvTileViewInfo);
}

void CObjectList::SetTrackMouse(const CB_TrackMouseEvent& cbMouseEvent)
{
	m_para.cbMouseEvent = cbMouseEvent;

	m_iTrackMouseFlag = 0;
}

template <bool _clear_other>
void CObjectList::_SetItemTexts(UINT uItem, const vector<wstring>& vecText, const wstring& strPrefix)
{
	UINT uIndex = 0;
	for (; uIndex < vecText.size() && uIndex < m_uColumnCount; ++uIndex)
	{
		wstring strText(strPrefix);
		strText.append(vecText[uIndex]);

		(void)__super::SetItemText(uItem, uIndex, strText.c_str());
	}

	if (_clear_other)
	{
		for (; uIndex < m_uColumnCount; uIndex++)
		{
			(void)__super::SetItemText(uItem, uIndex, NULL);
		}
	}
}

int CObjectList::InsertItemEx(UINT uItem, const vector<wstring>& vecText, const wstring& strPrefix)
{
	__EnsureReturn(!vecText.empty(), -1);

	wstring strText(strPrefix);
	strText.append(vecText.front());
	int iItem = InsertItem(uItem, strText.c_str());
	if (iItem >= 0)
	{
		for (UINT uIndex = 1; uIndex < m_uColumnCount && uIndex < vecText.size(); ++uIndex)
		{
			wstring strText(strPrefix);
			strText.append(vecText[uIndex]);

			(void)__super::SetItemText(uItem, uIndex, strText.c_str());
		}
	}

	return iItem;
}

int CObjectList::InsertItemEx(UINT uItem, const list<pair<UINT, wstring>>& lstText, const wstring& strPrefix)
{
	int iItem = InsertItem(uItem, L"");
	if (iItem >= 0)
	{
		SetItemTexts(iItem, lstText, strPrefix);
	}

	return iItem;
}

void CObjectList::SetItemTexts(UINT uItem, const vector<wstring>& vecText, const wstring& strPrefix)
{
	_SetItemTexts<false>(uItem, vecText, strPrefix);
}

void CObjectList::SetItemTexts(UINT uItem, const list<pair<UINT, wstring>>& lstText, const wstring& strPrefix)
{
	for (auto& pr : lstText)
	{
		if (pr.first < m_uColumnCount)
		{
			wstring strText(strPrefix);
			strText.append(pr.second);

			(void)__super::SetItemText(uItem, pr.first, strText.c_str());
		}
	}
}

void CObjectList::SetItemTexts(UINT uItem, UINT uSubItem, const vector<wstring>& vecText, const wstring& strPrefix)
{
	for (auto& strText : vecText)
	{
		if (uSubItem >= m_uColumnCount)
		{
			break;
		}

		(void)__super::SetItemText(uItem, uSubItem++, (strPrefix+strText).c_str());
	}
}

void CObjectList::SetTexts(const vector<vector<wstring>>& vecTexts, int nPos, const wstring& strPrefix)
{
	if (vecTexts.empty())
	{
		if (0 == nPos)
		{
			(void)DeleteAllItems();
		}

		return;
	}

	__Assert(nPos <= GetItemCount());

	int nMaxItem = GetItemCount() - 1;

	DeselectAllItems();

	CRedrawLockGuard RedrawLockGuard(*this);

	int nItem = nPos;
	for (auto& vecText : vecTexts)
	{
		if (nItem <= nMaxItem)
		{
			SetItemTexts(nItem, vecText, strPrefix);
		}
		else
		{
			(void)InsertItemEx(nItem, vecText, strPrefix);
		}

		nItem++;
	}

	for (; nMaxItem >= nItem; --nMaxItem)
	{
		(void)DeleteItem(nMaxItem);
	}
}

void CObjectList::SetObjects(const TD_ListObjectList& lstObjects, int nPos, const wstring& strPrefix)
{
	if (!lstObjects)
	{
		if (0 == nPos)
		{
			(void)DeleteAllItems();
		}
		
		return;
	}
	
	__Assert(nPos <= GetItemCount());

	int nMaxItem = GetItemCount()-1;

	DeselectAllItems();

	CRedrawLockGuard RedrawLockGuard(*this);

	int nItem = nPos;
	lstObjects([&](CListObject& object) {
		if (nItem <= nMaxItem)
		{
			SetItemObject(nItem, object, strPrefix);
		}
		else
		{
			(void)InsertObject(object, nItem, strPrefix);
		}

		nItem++;
	});
	
	for (; nMaxItem >= nItem; --nMaxItem)
	{
		(void)DeleteItem(nMaxItem);
	}
}

void CObjectList::SetColumnText(UINT uColumn, const wstring& strText)
{
	HDITEM hdItem;
	hdItem.mask = HDI_TEXT;
	hdItem.pszText = (LPWSTR)strText.c_str();
	CListCtrl::GetHeaderCtrl()->SetItem(uColumn, &hdItem);
}

int CObjectList::InsertObject(CListObject& Object, int nItem, const wstring& strPrefix)
{
	if (-1 == nItem)
	{
		nItem = __super::GetItemCount();
	}

	nItem = __super::InsertItem(nItem, NULL);
	
	this->SetItemObject(nItem, Object, strPrefix);
	
	return nItem;
}

void CObjectList::UpdateObject(CListObject& Object)
{
	__Ensure(m_hWnd);

	int nItem = this->GetObjectItem(&Object);
	if (0 <= nItem)
	{
		this->SetItemObject(nItem, Object);
	}
}

void CObjectList::SetItemObject(UINT uItem, CListObject& Object, const wstring& strPrefix)
{
	int iImage = 0;
	vector<wstring> vecText;
	GenListItem(Object, vecText, iImage);

	__Assert(SetItem(uItem, 0, LVIF_IMAGE | LVIF_PARAM, NULL
		, iImage, 0, 0, (LPARAM)&Object));

	_SetItemTexts<true>(uItem, vecText, strPrefix);
}

void CObjectList::UpdateItem(UINT uItem)
{
	__Ensure(m_hWnd);

	auto pObject = this->GetItemObject(uItem);
	if (NULL != pObject)
	{
		this->SetItemObject(uItem, *pObject);
	}
	else
	{
		__super::Update(uItem);
	}
}

void CObjectList::UpdateItem(UINT uItem, CListObject& Object, const list<UINT>& lstColumn)
{
	__Ensure(m_hWnd);

	int iImage = 0;
	vector<wstring> vecText;
	GenListItem(Object, vecText, iImage);

	__Assert(SetItem(uItem, 0, LVIF_IMAGE | LVIF_PARAM, NULL
		, iImage, 0, 0, (LPARAM)&Object));

	for (auto nColumn : lstColumn)
	{
		if (nColumn < vecText.size())
		{
			(void)__super::SetItemText(uItem, nColumn, vecText[nColumn].c_str());
		}
	}
}

void CObjectList::UpdateItems()
{
	__Ensure(m_hWnd);

	CRedrawLockGuard RedrawLockGuard(*this);

	for (UINT uItem = 0; uItem < (UINT)this->GetItemCount(); ++uItem)
	{
		UpdateItem(uItem);
	}
}

void CObjectList::UpdateColumns(const list<UINT>& lstColumn)
{
	__Ensure(m_hWnd);

	CRedrawLockGuard RedrawLockGuard(*this);

	for (UINT uItem = 0; uItem < (UINT)this->GetItemCount(); uItem++)
	{
		auto pObject = (CListObject*)__super::GetItemData(uItem);
		if (pObject)
		{
			UpdateItem(uItem, *pObject, lstColumn);
		}
	}
}

BOOL CObjectList::DeleteObject(const CListObject *pObject)
{
	int nItem = this->GetObjectItem(pObject);
	__EnsureReturn(0 <= nItem, FALSE);

	return this->DeleteItem(nItem);
}

void CObjectList::DeleteObjects(const set<CListObject*>& setDeleteObjects)
{
	__Ensure(m_hWnd);

	CListObject *pObject = NULL;

	int nItemCount = this->GetItemCount();
	for (int nItem = 0; nItem < nItemCount; )
	{
		pObject = this->GetItemObject(nItem);
		
		if (setDeleteObjects.find(pObject) != setDeleteObjects.end())
		{
			this->DeleteItem(nItem);

			nItemCount--;

			continue;
		}

		nItem++;
	}
}

void CObjectList::DeleteItems(list<UINT> lstItems)
{
	for (int iItem = GetItemCount()-1; iItem >= 0; iItem--)
	{
		auto itr = std::find(lstItems.begin(), lstItems.end(), iItem);
		if (itr != lstItems.end())
		{
			DeleteItem(iItem);

			(void)lstItems.erase(itr);
			if (lstItems.empty())
			{
				break;
			}
		}
	}
}

void CObjectList::GenListItem(CListObject& Object, vector<wstring>& vecText, int& iImage)
{
	Object.GenListItem(E_ListViewType::LVT_Report == this->GetView(), vecText, iImage);
}

void CObjectList::SetItemImage(UINT uItem, int iImage)
{
	(void)SetItem(uItem, 0, LVIF_IMAGE, NULL, iImage, 0, 0, 0);
	__super::Update(uItem);
}

CListObject *CObjectList::GetItemObject(int iItem)
{
	__EnsureReturn(iItem >= 0 && iItem < GetItemCount(), NULL);
	return (CListObject*)__super::GetItemData(iItem);
}

int CObjectList::GetObjectItem(const CListObject *pObject)
{
	for (int nItem = 0; nItem < __super::GetItemCount(); ++nItem)
	{
		if ((CListObject*)__super::GetItemData(nItem) == pObject)
		{
			return nItem;
		}
	}

	return -1;
}

void CObjectList::GetAllObjects(TD_ListObjectList& lstListObjects)
{
	for (int nItem = 0; nItem < __super::GetItemCount(); ++nItem)
	{
		auto pObject = (CListObject*)__super::GetItemData(nItem);
		if (pObject)
		{
			lstListObjects.add(pObject);
		}
	}
}

int CObjectList::GetSelItem()
{
	POSITION lpPos = __super::GetFirstSelectedItemPosition();
	__EnsureReturn(lpPos, -1);

	return __super::GetNextSelectedItem(lpPos);
}

CListObject *CObjectList::GetSelObject()
{
	int nItem = this->GetSelItem();
	__EnsureReturn(0 <= nItem, NULL);

	return this->GetItemObject(nItem);
}

void CObjectList::GetSelItems(list<UINT>& lstItems)
{
	POSITION lpPos = __super::GetFirstSelectedItemPosition();
	while (lpPos)
	{
		lstItems.push_back(__super::GetNextSelectedItem(lpPos));
	}
}

void CObjectList::GetSelObjects(map<UINT, CListObject*>& mapObjects)
{
	list<UINT> lstItems;
	this->GetSelItems(lstItems);

	for (auto uItem : lstItems)
	{
		auto pObject = this->GetItemObject(uItem);
		if (pObject)
		{
			mapObjects[uItem] = pObject;
		}
	}
}

void CObjectList::GetSelObjects(TD_ListObjectList& lstObjects)
{
	list<UINT> lstItems;
	this->GetSelItems(lstItems);

	for (auto uItem : lstItems)
	{
		auto pObject = this->GetItemObject(uItem);
		if (pObject)
		{
			lstObjects.add(pObject);
		}
	}
}

BOOL CObjectList::SelectFirstItem()
{
	__EnsureReturn(0 < this->GetItemCount(), FALSE);
	
	this->SelectItem(0);

	return TRUE;
}

void CObjectList::SelectItem(UINT uItem, BOOL bSetFocus)
{
	UINT uState = LVIS_SELECTED;
	if (bSetFocus)
	{
		uState |= LVIS_FOCUSED;
	}

	(void)this->SetItemState(uItem, uState, uState);

	(void)this->SetSelectionMark(uItem);
	
	(void)this->EnsureVisible(uItem, FALSE);
}

void CObjectList::SelectObject(const CListObject *pObject, BOOL bSetFocus)
{
	int nItem =	this->GetObjectItem(pObject);
	if (0 <= nItem)
	{
		this->SelectItem(nItem, bSetFocus);
	}
}

void CObjectList::SelectItems(UINT uItem, UINT uSelectCount)
{
	DeselectAllItems();

	for (UINT uIndex = 0; uIndex < uSelectCount; ++uIndex)
	{
		SelectItem(uItem + uIndex, 0==uIndex);
	}
}

void CObjectList::SelectAllItems()
{
	CRedrawLockGuard RedrawLockGuard(*this);

	int nCount = this->GetItemCount();
	for (int nItem = 0; nItem < nCount; ++nItem)
	{
		(void)__super::SetItemState(nItem, LVIS_SELECTED, LVIS_SELECTED);
	}
}

void CObjectList::DeselectAllItems()
{
	int nItem = 0;

	POSITION lpPos = __super::GetFirstSelectedItemPosition();
	while (lpPos)
	{
		nItem = __super::GetNextSelectedItem(lpPos);

		(void)__super::SetItemState(nItem, 0, LVIS_SELECTED);
	}
}

BOOL CObjectList::OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	if (WM_NOTIFY == message)
	{
		NMHDR *pNMHDR = (NMHDR*)lParam;
		if (NULL != pNMHDR)
		{
			if (handleNMNotify(*pNMHDR, pResult))
			{
				return TRUE;
			}
		}
	}

	return __super::OnChildNotify(message, wParam, lParam, pResult);
}

BOOL CObjectList::handleNMNotify(NMHDR& NMHDR, LRESULT* pResult)
{
	switch (NMHDR.code)
	{
	case NM_CUSTOMDRAW:
	{
		__EnsureBreak(m_bCusomDrawNotify);

		LPNMLVCUSTOMDRAW pLVCD = reinterpret_cast<LPNMLVCUSTOMDRAW>(&NMHDR);

		if (CDDS_PREPAINT == pLVCD->nmcd.dwDrawStage)
		{
			*pResult = CDRF_NOTIFYITEMDRAW;
		}
		else if (CDDS_ITEMPREPAINT == pLVCD->nmcd.dwDrawStage)
		{
			*pResult = CDRF_NOTIFYITEMDRAW;
		}

		if (pLVCD->nmcd.dwDrawStage & CDDS_ITEM)
		{
			if (pLVCD->nmcd.dwDrawStage & CDDS_SUBITEM)
			{
				bool bSkipDefault = false;
				OnCustomDraw(*pLVCD, bSkipDefault);
				if (bSkipDefault)
				{
					*pResult = CDRF_SKIPDEFAULT;
				}
			}
		}

		return TRUE;
	}

	break;
	case LVN_BEGINLABELEDIT:
	{
		CEdit *pwndEdit = this->GetEditControl();
		__AssertBreak(pwndEdit);

		NMLVDISPINFO *pLVDispInfo = reinterpret_cast<NMLVDISPINFO*>(&NMHDR);

		CListObject *pObject = this->GetItemObject(pLVDispInfo->item.iItem);
		if (pObject)
		{
			m_cstrRenameText = pObject->GetRenameText().c_str();
		}
		else
		{
			m_cstrRenameText = pLVDispInfo->item.pszText;
			getRenameText(pLVDispInfo->item.iItem, m_cstrRenameText);
		}

		pwndEdit->SetWindowText(m_cstrRenameText);
		pwndEdit->SetSel(0, m_cstrRenameText.GetLength(), TRUE);
	}

	break;
	case LVN_ENDLABELEDIT:
	{
		NMLVDISPINFO *pLVDispInfo = reinterpret_cast<NMLVDISPINFO*>(&NMHDR);

		CString cstrNewText(pLVDispInfo->item.pszText);
		(void)cstrNewText.Trim();
		__EnsureReturn(!cstrNewText.IsEmpty(), TRUE);

		__EnsureReturn(cstrNewText != m_cstrRenameText, TRUE);

		int nItem = pLVDispInfo->item.iItem;
		CListObject *pObject = GetItemObject(nItem);
		if (pObject)
		{
			pObject->OnListItemRename((LPCTSTR)cstrNewText);
		}
		else
		{
			OnListItemRename(pLVDispInfo->item.iItem, cstrNewText);
		}

		this->UpdateItem(pLVDispInfo->item.iItem);
	}

	break;
	case LVN_KEYDOWN:
	{
		LPNMLVKEYDOWN pLVKeyDow = reinterpret_cast<LPNMLVKEYDOWN>(&NMHDR);

		if ('A' == pLVKeyDow->wVKey && CMainApp::getKeyState(VK_CONTROL))
		{
			if (0 == (this->GetStyle() & LVS_SINGLESEL))
			{
				this->DeselectAllItems();
				this->SelectAllItems();
			}
		}

		return TRUE;
	}

	break;
	case NM_CLICK:
		m_bDblClick = false;

		break;
	case NM_DBLCLK:
		m_bDblClick = true;

		break;
	default:
		break;
	}
	
	return FALSE;
}

void CObjectList::OnCustomDraw(NMLVCUSTOMDRAW& lvcd, bool& bSkipDefault)
{
	if (!m_para.setUnderlineColumns.empty())
	{
		if (m_para.setUnderlineColumns.find(lvcd.iSubItem) != m_para.setUnderlineColumns.end())
		{
			::SelectObject(lvcd.nmcd.hdc, m_fontUnderline);
		}
		else
		{
			::SelectObject(lvcd.nmcd.hdc, *this->GetFont());
		}
	}

	if (m_para.cbCustomDraw)
	{
		m_para.cbCustomDraw(*this, lvcd, bSkipDefault);
	}
}

void CObjectList::OnTrackMouseEvent(E_TrackMouseEvent eMouseEvent, const CPoint& point)
{
	if (m_para.cbMouseEvent)
	{
		m_para.cbMouseEvent(eMouseEvent, point);
	}
}

BOOL CObjectList::OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	if (WM_MOUSEWHEEL == message)
	{
		if (m_bAutoChange)
		{
			UINT uFlag = GET_FLAGS_LPARAM(wParam);
			if (uFlag & MK_CONTROL)
			{
				static DWORD dwLastTime = 0;

				DWORD dwTime = ::GetTickCount();

				if (500 < dwTime - dwLastTime)
				{
					ChangeListCtrlView();

					dwLastTime = dwTime;
				}

				return TRUE;
			}
		}
	}
	else if (-1 != m_iTrackMouseFlag)
	{
		if (WM_MOUSEMOVE == message)
		{
			if (0 == m_iTrackMouseFlag)
			{
				TRACKMOUSEEVENT tme;
				tme.cbSize = sizeof(tme);
				tme.hwndTrack = m_hWnd;
				tme.dwFlags = TME_LEAVE | TME_HOVER;
				tme.dwHoverTime = HOVER_DEFAULT;
				m_iTrackMouseFlag = ::TrackMouseEvent(&tme);
			}

			OnTrackMouseEvent(E_TrackMouseEvent::LME_MouseMove, CPoint(lParam));
			return TRUE;
		}
		else if (WM_MOUSELEAVE == message)
		{
			m_iTrackMouseFlag = 0;

			OnTrackMouseEvent(E_TrackMouseEvent::LME_MouseLeave, CPoint(lParam));
			return TRUE;
		}
		else if (WM_MOUSEHOVER == message)
		{
			//m_iTrackMouseFlag = 0;

			OnTrackMouseEvent(E_TrackMouseEvent::LME_MouseHover, CPoint(lParam));
			return TRUE;
		}
	}

	return __super::OnWndMsg(message, wParam, lParam, pResult);
}

void CObjectList::ChangeListCtrlView(short zDelta)
{
	E_ListViewType lpViewType[] = {
		E_ListViewType::LVT_Tile
		, E_ListViewType::LVT_Report
		, E_ListViewType::LVT_List
		//, LVT_SmallIcon
		, E_ListViewType::LVT_Icon
	};

	E_ListViewType nPreViewType = this->GetView();

	for (UINT uIndex = 0; uIndex < sizeof(lpViewType) / sizeof(E_ListViewType); ++uIndex)
	{
		if (lpViewType[uIndex] == nPreViewType)
		{
			uIndex++;

			if (sizeof(lpViewType) / sizeof(E_ListViewType) <= uIndex)
			{
				uIndex = 0;
			}
			
			m_para.eViewType = lpViewType[uIndex];
			this->SetView(m_para.eViewType);

			if (m_para.cbViewChanged)
			{
				m_para.cbViewChanged(m_para.eViewType);
			}

			break;
		}
	}
}

UINT CObjectList::GetHeaderHeight()
{
	if (E_ListViewType::LVT_Report != this->GetView())
	{
		return 0;
	}

	CRect rcHeader;
	CListCtrl::GetHeaderCtrl()->GetWindowRect(&rcHeader);

	return (UINT)rcHeader.Height();
}
