
#include "StdAfx.h"

#include "Common/ListCtrl.h"

BEGIN_MESSAGE_MAP(CListHeader, CHeaderCtrl)
	ON_MESSAGE(HDM_LAYOUT, OnLayout)
END_MESSAGE_MAP()

LRESULT CListHeader::OnLayout(WPARAM wParam, LPARAM lParam)
{
	LRESULT lResult = CHeaderCtrl::DefWindowProc(HDM_LAYOUT, 0, lParam);

	auto& hdl = *(HD_LAYOUT*)lParam;
	hdl.prc->top = hdl.pwpos->cy = m_uHeight;
	
	return lResult;
}

BOOL CListHeader::Init(HWND hWnd, int nHeight)
{
	if (nHeight > 0)
	{
		m_uHeight = (UINT)nHeight;
		if (NULL == m_hWnd)
		{
			__AssertReturn(SubclassWindow(hWnd), FALSE);
		}
	}
	else
	{
		if (m_hWnd)
		{
			UnsubclassWindow();
		}
	}
	
	::InvalidateRect(hWnd, NULL, TRUE);

	return TRUE;
}

// CObjectList

void CObjectList::PreSubclassWindow()
{
	__super::PreSubclassWindow();
	
	(void)ModifyStyle(0, LVS_SHOWSELALWAYS | LVS_SHAREIMAGELISTS);

	(void)SetExtendedStyle(GetExtendedStyle()
		| LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP | LVS_EX_LABELTIP);
}

BOOL CObjectList::InitCtrl(const tagListPara& para)
{
	m_para = para;

	if (-1 != (int)m_para.eViewType)
	{
		SetView(m_para.eViewType);
	}

	if (!m_para.lstColumns.empty())
	{
		InitColumn(m_para.lstColumns);
	}

	__EnsureReturn(SetHeaderHeight(m_para.nHeaderHeight), FALSE);

	auto pHeader = CListCtrl::GetHeaderCtrl();
	__EnsureReturn(m_fontHeader.create(pHeader->m_hWnd, m_para.fHeaderFontSize, 0), FALSE);

	__EnsureReturn(InitFont(m_para.crText, m_para.fFontSize), FALSE);

	pHeader->SetFont(&m_fontHeader);
	
	if (0 != m_para.uItemHeight)
	{
		__EnsureReturn(SetItemHeight(m_para.uItemHeight), FALSE);
	}

	if (0 != m_para.uTileWidth && 0 != m_para.uTileHeight)
	{
		SetTileSize(m_para.uTileWidth, m_para.uTileHeight);
	}
	
	return TRUE;
}

BOOL CObjectList::InitFont(COLORREF crText, float fFontSizeOffset)
{
	__AssertReturn(SetTextColor(crText), FALSE);
	
	__AssertReturn(m_font.setFont(*this, fFontSizeOffset), FALSE);
	
	__AssertReturn(m_fontUnderline.create(*this, 0, 0, false, true), FALSE);

	return TRUE;
}

BOOL CObjectList::InitImglst(const CSize& size, const CSize *pszSmall, const TD_IconVec& vecIcons)
{
	__AssertReturn(m_Imglst.Init(size, vecIcons), FALSE);
	(void)__super::SetImageList(&m_Imglst, LVSIL_NORMAL);

	if (NULL != pszSmall)
	{
		__AssertReturn(m_smallImglst.Init(*pszSmall, vecIcons), FALSE);
		(void)__super::SetImageList(&m_smallImglst, LVSIL_SMALL);
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
		__AssertReturn(m_smallImglst.Init(*pBitmapSmall), FALSE);
		(void)__super::SetImageList(&m_smallImglst, LVSIL_SMALL);
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
	CRedrawLock RedrawLock(*this);

	m_para.eViewType = eViewType;
	(void)__super::SetView((int)eViewType);

	if (bArrange)
	{
		(void)Arrange(LVA_DEFAULT);
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

void CObjectList::InitColumn(const TD_ListColumn& lstColumns)
{
	m_uColumnCount = 0;
	for (cauto column : lstColumns)
	{
		(void)__super::InsertColumn(m_uColumnCount, column.bCenter ? column.cstrText : (L' ' + column.cstrText)
			, column.bCenter ? LVCFMT_CENTER : LVCFMT_LEFT, column.uWidth);

		m_uColumnCount++;
	}
}

BOOL CObjectList::SetHeaderHeight(int nHeaderHeight)
{
	if (0 == nHeaderHeight)
	{
		(void)ModifyStyle(0, LVS_NOCOLUMNHEADER);
	}
	else
	{
		(void)ModifyStyle(LVS_NOCOLUMNHEADER, 0);

		__AssertReturn(m_wndHeader.Init(CListCtrl::GetHeaderCtrl()->m_hWnd, nHeaderHeight), FALSE);
	}

	return true;
}

BOOL CObjectList::SetItemHeight(UINT uItemHeight)
{
	__AssertReturn(m_smallImglst.Create(1, uItemHeight, ILC_COLOR, 0, 0), FALSE);

	SetImageList(NULL, &m_smallImglst);
	
	return TRUE;
}

void CObjectList::SetTileSize(ULONG cx, ULONG cy)
{
	LVTILEVIEWINFO LvTileViewInfo { sizeof(LVTILEVIEWINFO) };
	LvTileViewInfo.dwFlags = LVTVIF_FIXEDSIZE;
	LvTileViewInfo.sizeTile = { (LONG)cx, (LONG)cy };
	LvTileViewInfo.dwMask = LVTVIM_TILESIZE;
	(void)SetTileViewInfo(&LvTileViewInfo);
}

template <bool _clear_other>
void CObjectList::_SetItemTexts(UINT uItem, const vector<wstring>& vecText, cwstr strPrefix)
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

int CObjectList::InsertItemEx(UINT uItem, const vector<wstring>& vecText, cwstr strPrefix)
{
	__EnsureReturn(!vecText.empty(), -1);

	wstring strText(strPrefix);
	strText.append(vecText.front());
	int nItem = InsertItem(uItem, strText.c_str());
	if (nItem >= 0)
	{
		for (UINT uIndex = 1; uIndex < m_uColumnCount && uIndex < vecText.size(); ++uIndex)
		{
			wstring strText(strPrefix);
			strText.append(vecText[uIndex]);

			(void)__super::SetItemText(uItem, uIndex, strText.c_str());
		}
	}

	return nItem;
}

int CObjectList::InsertItemEx(UINT uItem, const prlist<UINT, wstring>& lstText, cwstr strPrefix)
{
	int nItem = InsertItem(uItem, L"");
	if (nItem >= 0)
	{
		SetItemTexts(nItem, lstText, strPrefix);
	}

	return nItem;
}

void CObjectList::SetItemTexts(UINT uItem, const vector<wstring>& vecText, cwstr strPrefix)
{
	_SetItemTexts<false>(uItem, vecText, strPrefix);
}

void CObjectList::SetItemTexts(UINT uItem, const prlist<UINT, wstring>& lstText, cwstr strPrefix)
{
	for (cauto pr : lstText)
	{
		if (pr.first < m_uColumnCount)
		{
			wstring strText(strPrefix);
			strText.append(pr.second);

			(void)__super::SetItemText(uItem, pr.first, strText.c_str());
		}
	}
}

void CObjectList::SetItemTexts(UINT uItem, UINT uSubItem, const vector<wstring>& vecText, cwstr strPrefix)
{
	for (cauto strText : vecText)
	{
		if (uSubItem >= m_uColumnCount)
		{
			break;
		}

		(void)__super::SetItemText(uItem, uSubItem++, (strPrefix+strText).c_str());
	}
}

void CObjectList::SetTexts(const vector<vector<wstring>>& vecTexts)// , int nPos, cwstr strPrefix)
{
	if (vecTexts.empty())
	{
		//if (0 == nPos)
		//{
			(void)DeleteAllItems();
		//}

		return;
	}

	int nPrevCount = GetItemCount();
	//__Assert(nPos <= nPrevCount);

	DeselectAll();

	CRedrawLock RedrawLock(*this);

	int nItem = 0;// nPos;
	for (cauto vecText : vecTexts)
	{
		if (nItem < nPrevCount)
		{
			SetItemTexts(nItem, vecText);// , strPrefix);
		}
		else
		{
			(void)InsertItemEx(nItem, vecText);// , strPrefix);
		}

		nItem++;
	}

	for (int nMaxItem = nPrevCount-1; nMaxItem >= nItem; --nMaxItem)
	{
		(void)DeleteItem(nMaxItem);
	}
}

void CObjectList::SetObjects(const TD_ListObjectList& lstObjects)// , int nPos, cwstr strPrefix)
{
	if (!lstObjects)
	{
		//if (0 == nPos)
		//{
			(void)DeleteAllItems();
		//}
		
		return;
	}
	
	int nPrevCount = GetItemCount();
	//__Assert(nPos <= nPrevCount);

	CRedrawLock RedrawLock(*this);

	DeselectAll();

	int nItem = 0;//nPos;
	lstObjects([&](CListObject& object) {
		if (nItem < nPrevCount)
		{
			_SetItemObject(nItem, object);//, strPrefix);
		}
		else
		{
			(void)InsertObject(object, nItem);// , strPrefix);
		}

		nItem++;
	});
	
	for (int nMaxItem = nPrevCount-1; nMaxItem >= nItem; --nMaxItem)
	{
		(void)DeleteItem(nMaxItem);
	}

	if (E_ListViewType::LVT_List == GetView())
	{
		SetView(E_ListViewType::LVT_Tile);
		SetView(E_ListViewType::LVT_List);
	}
}

void CObjectList::SetColumnText(UINT uColumn, cwstr strText)
{
	HDITEM hdItem;
	hdItem.mask = HDI_TEXT;
	hdItem.pszText = (LPWSTR)strText.c_str();
	CListCtrl::GetHeaderCtrl()->SetItem(uColumn, &hdItem);
}

int CObjectList::InsertObject(CListObject& Object, int nItem, cwstr strPrefix)
{
	if (-1 == nItem)
	{
		nItem = GetItemCount();
	}

	nItem = __super::InsertItem(nItem, NULL);
	
	_SetItemObject(nItem, Object, strPrefix);
	
	return nItem;
}

void CObjectList::_SetItemObject(UINT uItem, CListObject& Object, cwstr strPrefix)
{
	vector<wstring> vecText;
	int iImage = -1;
	GenListItem(Object, GetView(), vecText, iImage);

	__Assert(SetItem(uItem, 0, LVIF_PARAM | LVIF_IMAGE, NULL, iImage, 0, 0, (LPARAM)&Object));

	_SetItemTexts<true>(uItem, vecText, strPrefix);
}

void CObjectList::UpdateItem(UINT uItem, CListObject *pObject)
{
	__Ensure(m_hWnd);

	if (NULL == pObject)
	{
		pObject = GetItemObject(uItem);
	}

	if (NULL != pObject)
	{
		_SetItemObject(uItem, *pObject);
	}
	else
	{
		__super::Update(uItem);
	}
}

void CObjectList::UpdateItems()
{
	__Ensure(m_hWnd);

	CRedrawLock RedrawLock(*this);

	UINT uItemCount = (UINT)GetItemCount();
	for (UINT uItem = 0; uItem < uItemCount; ++uItem)
	{
		UpdateItem(uItem);
	}
}

void CObjectList::UpdateColumns(const list<UINT>& lstColumn)
{
	__Ensure(m_hWnd);

	CRedrawLock RedrawLock(*this);
	
	auto eViewType = GetView();

	UINT uItemCount = (UINT)GetItemCount();
	for (UINT uItem = 0; uItem < uItemCount; uItem++)
	{
		auto pObject = (CListObject*)GetItemData(uItem);
		if (pObject)
		{
			vector<wstring> vecText;
			int iImage = -1;
			GenListItem(*pObject, eViewType, vecText, iImage);

			for (auto nColumn : lstColumn)
			{
				if (nColumn < vecText.size())
				{
					(void)__super::SetItemText(uItem, nColumn, vecText[nColumn].c_str());
				}
			}
		}
	}
}

BOOL CObjectList::DeleteObject(const CListObject *pObject)
{
	int nItem = GetObjectItem(pObject);
	__EnsureReturn(nItem >= 0, FALSE);

	return DeleteItem(nItem);
}

void CObjectList::DeleteObjects(const set<CListObject*>& setDeleteObjects)
{
	__Ensure(m_hWnd);

	CRedrawLock RedrawLock(*this);

	CListObject *pObject = NULL;

	int nItemCount = GetItemCount();
	for (int nItem = 0; nItem < nItemCount; )
	{
		pObject = GetItemObject(nItem);		
		if (setDeleteObjects.find(pObject) != setDeleteObjects.end())
		{
			DeleteItem(nItem);

			nItemCount--;

			continue;
		}

		nItem++;
	}
}

void CObjectList::DeleteItems(const set<UINT>& setItems)
{
	__Ensure(m_hWnd);

	CRedrawLock RedrawLock(*this);

	for (auto itr = setItems.rbegin(); itr != setItems.rend(); ++itr)
	{
		DeleteItem(*itr);
	}
}

void CObjectList::GenListItem(CListObject& Object, E_ListViewType eViewType, vector<wstring>& vecText, TD_ListImgIdx& iImage)
{
	Object.GenListItem(eViewType, vecText, iImage);
}

void CObjectList::SetItemImage(UINT uItem, UINT uImage)
{
	(void)SetItem(uItem, 0, LVIF_IMAGE, NULL, uImage, 0, 0, 0);
	__super::Update(uItem);
}

CListObject *CObjectList::GetItemObject(int nItem)
{
	__EnsureReturn(nItem >= 0 && nItem < GetItemCount(), NULL);
	return (CListObject*)GetItemData(nItem);
}

int CObjectList::GetObjectItem(const CListObject *pObject)
{
	int uItemCount = GetItemCount();
	for (int nItem = 0; nItem < uItemCount; ++nItem)
	{
		if ((CListObject*)GetItemData(nItem) == pObject)
		{
			return nItem;
		}
	}

	return -1;
}

void CObjectList::GetAllObjects(TD_ListObjectList& lstListObjects)
{
	UINT uItemCount = GetItemCount();
	for (UINT nItem = 0; nItem < uItemCount; ++nItem)
	{
		auto pObject = (CListObject*)GetItemData(nItem);
		if (pObject)
		{
			lstListObjects.add(pObject);
		}
	}
}

int CObjectList::GetSelItem()
{
	POSITION lpPos = GetFirstSelectedItemPosition();
	__EnsureReturn(lpPos, -1);

	return GetNextSelectedItem(lpPos);
}

CListObject *CObjectList::GetSelObject()
{
	int nItem = GetSelItem();
	__EnsureReturn(nItem >= 0, NULL);

	return GetItemObject(nItem);
}

void CObjectList::GetSelItems(list<UINT>& lstItems)
{
	POSITION lpPos = GetFirstSelectedItemPosition();
	while (lpPos)
	{
		lstItems.push_back(GetNextSelectedItem(lpPos));
	}
}

void CObjectList::GetSelObjects(map<UINT, CListObject*>& mapObjects)
{
	list<UINT> lstItems;
	GetSelItems(lstItems);

	for (auto uItem : lstItems)
	{
		auto pObject = GetItemObject(uItem);
		if (pObject)
		{
			mapObjects[uItem] = pObject;
		}
	}
}

void CObjectList::GetSelObjects(TD_ListObjectList& lstObjects)
{
	list<UINT> lstItems;
	GetSelItems(lstItems);

	for (auto uItem : lstItems)
	{
		auto pObject = GetItemObject(uItem);
		if (pObject)
		{
			lstObjects.add(pObject);
		}
	}
}

BOOL CObjectList::SelectFirstItem()
{
	__EnsureReturn(GetItemCount() > 0, FALSE);
	
	SelectItem(0);

	return TRUE;
}

void CObjectList::SelectItem(UINT uItem, BOOL bSetFocus)
{
	UINT uState = LVIS_SELECTED;
	if (bSetFocus)
	{
		uState |= LVIS_FOCUSED;
	}

	(void)SetItemState(uItem, uState, uState);

	(void)SetSelectionMark(uItem);
	
	(void)EnsureVisible(uItem, FALSE);
}

void CObjectList::SelectObject(const CListObject *pObject, BOOL bSetFocus)
{
	int nItem =	GetObjectItem(pObject);
	if (nItem >= 0)
	{
		SelectItem(nItem, bSetFocus);
	}
}

void CObjectList::SelectItems(UINT uItem, UINT uSelectCount)
{
	DeselectAll();

	for (UINT uIndex = 0; uIndex < uSelectCount; ++uIndex)
	{
		SelectItem(uItem + uIndex, 0==uIndex);
	}
}

void CObjectList::SelectAll()
{
	CRedrawLock RedrawLock(*this);

	int nItemCount = GetItemCount();
	for (int nItem = 0; nItem < nItemCount; ++nItem)
	{
		(void)SetItemState(nItem, LVIS_SELECTED, LVIS_SELECTED);
	}
}

void CObjectList::DeselectAll()
{
	int nItem = 0;

	POSITION lpPos = GetFirstSelectedItemPosition();
	while (lpPos)
	{
		nItem = GetNextSelectedItem(lpPos);

		(void)SetItemState(nItem, 0, LVIS_SELECTED);
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

bool CObjectList::GetRenameText(UINT uItem, wstring& strRenameText)
{
	CListObject *pObject = GetItemObject(uItem);
	if (NULL != pObject)
	{
		if (!pObject->GetRenameText(strRenameText))
		{
			return false;
		}
	}

	return true;
}

static CCompatableFont m_fontCustom;

void CObjectList::SetCustomFont(CDC& dc, float fFontSizeOffset, bool bUnderline)
{
	(void)m_fontCustom.DeleteObject();
	if (m_fontCustom.create(m_font, fFontSizeOffset, -1, false, bUnderline))
	{
		(void)dc.SelectObject(m_fontCustom);
	}
}

void CObjectList::handleCustomDraw(NMLVCUSTOMDRAW& lvnmcd, LRESULT* pResult)
{
	auto& nmcd = lvnmcd.nmcd;
	if (CDDS_PREPAINT == nmcd.dwDrawStage)
	{
		*pResult = CDRF_NOTIFYITEMDRAW;
	}
	else if (CDDS_ITEMPREPAINT == nmcd.dwDrawStage)
	{
		if (GetItemState(nmcd.dwItemSpec, LVIS_SELECTED))
		{
			lvnmcd.clrTextBk = __crHit;
		}
		else
		{
			lvnmcd.clrTextBk = __crWhite;
		}

		CDC dc;
		if (dc.Attach(lvnmcd.nmcd.hdc))
		{
			if (GetView() != E_ListViewType::LVT_Icon)
			{
				dc.FillSolidRect(&lvnmcd.nmcd.rc, lvnmcd.clrTextBk);
			}
			else
			{
				dc.FillSolidRect(&lvnmcd.nmcd.rc, __crWhite);
			}

			dc.Detach();
		}

		nmcd.uItemState &= ~CDIS_SELECTED;
		nmcd.uItemState &= ~CDIS_FOCUS;

		auto eViewType = GetView();
		if (E_ListViewType::LVT_Report == eViewType)
		{
			if (m_cbDrawSubItem)
			{
				*pResult = CDRF_NOTIFYSUBITEMDRAW;
			}
		}

		if (m_cbPostDraw)
		{
			*pResult |= CDRF_NOTIFYPOSTPAINT;
		}		
	}
	else
	{
		if (CDDS_ITEMPOSTPAINT == nmcd.dwDrawStage)
		{
			if (m_cbPostDraw)
			{
				if (0 == lvnmcd.nmcd.rc.right)
				{
					return; // TODO ���ĳbug
				}
				tagLVDrawSubItem lvcd(lvnmcd);
				lvcd.crText = m_para.crText;
				m_cbPostDraw(lvcd);
			}
		}
		else if ((CDDS_ITEMPREPAINT | CDDS_SUBITEM) == nmcd.dwDrawStage)
		{
			if (!m_cbDrawSubItem)
			{
				return;
			}

			tagLVDrawSubItem lvcd(lvnmcd);
			lvcd.crText = m_para.crText;
			auto crBkg = lvcd.crBkg;

			m_cbDrawSubItem(lvcd);
			if (lvcd.bSkipDefault)
			{
				*pResult = CDRF_SKIPDEFAULT;
				return;
			}
			
			if (lvcd.crBkg != crBkg)
			{
				if (GetItemState(lvcd.uItem, LVIS_SELECTED))
				{
					lvcd.crBkg = crBkg;
				}
			}

			if (0 != lvcd.fFontSizeOffset)
			{
				SetCustomFont(lvcd.dc, lvcd.fFontSizeOffset, lvcd.bSetUnderline);
				*pResult = CDRF_NEWFONT;
			}
			else if (lvcd.bSetUnderline)
			{
				(void)lvcd.dc.SelectObject(m_fontUnderline);
				*pResult = CDRF_NEWFONT;
			}
			else
			{
				(void)lvcd.dc.SelectObject(m_font);
			}
		}
	}
}

BOOL CObjectList::handleNMNotify(NMHDR& NMHDR, LRESULT* pResult)
{
	switch (NMHDR.code)
	{
	case NM_CUSTOMDRAW:
	{
		LPNMLVCUSTOMDRAW pLVCD = reinterpret_cast<LPNMLVCUSTOMDRAW>(&NMHDR);
		handleCustomDraw(*pLVCD, pResult);
		return TRUE;
	}
	
	break;
	case LVN_BEGINLABELEDIT:
	{
		CEdit *pwndEdit = GetEditControl();
		__AssertBreak(pwndEdit);
		
		NMLVDISPINFO *pLVDispInfo = reinterpret_cast<NMLVDISPINFO*>(&NMHDR);
		auto& item = pLVDispInfo->item;

		wstring strRenameText;
		if (NULL != item.pszText)
		{
			strRenameText = item.pszText;
		}
		
		if (!GetRenameText(item.iItem, strRenameText))
		{
			pwndEdit->ShowWindow(SW_HIDE);
			*pResult = 1;
			return TRUE;
		}

		pwndEdit->SetMargins(2, 3);

		m_cstrRenameText = strRenameText.c_str();
		pwndEdit->SetWindowText(m_cstrRenameText);

		/*__async([=]{
			CEdit *pwndEdit = GetEditControl();
			if (NULL != pwndEdit)
			{
				CRect rcPos;
				pwndEdit->GetWindowRect(rcPos);
				ScreenToClient(rcPos);
				rcPos.bottom += 3;
				pwndEdit->MoveWindow(rcPos);
			}
		});*/
	}

	break;
	case LVN_ENDLABELEDIT:
	{
		NMLVDISPINFO *pLVDispInfo = reinterpret_cast<NMLVDISPINFO*>(&NMHDR);
		cauto item = pLVDispInfo->item;
		CString cstrNewText(item.pszText);
		(void)cstrNewText.Trim();
		__EnsureReturn(!cstrNewText.IsEmpty(), TRUE);

		__EnsureReturn(cstrNewText != m_cstrRenameText, TRUE);
		
		::ShowWindow(GetEditControl()->GetSafeHwnd(), SW_HIDE);

		auto pObject = (CListObject*)item.lParam;
		if (OnItemRename(item.iItem, pObject, cstrNewText))
		{
			if (pObject)
			{
				UpdateItem(item.iItem, pObject);
			}
			else
			{
				(void)__super::SetItemText(item.iItem, 0, cstrNewText);
			}
		}
	}

	break;
	case LVN_KEYDOWN:
	{
		LPNMLVKEYDOWN pLVKeyDow = reinterpret_cast<LPNMLVKEYDOWN>(&NMHDR);

		if ('A' == pLVKeyDow->wVKey && CMainApp::getKeyState(VK_CONTROL))
		{
			if (0 == (GetStyle() & LVS_SINGLESEL))
			{
				DeselectAll();
				SelectAll();
			}
		}

		return TRUE;
	}

	break;
	}
	
	return FALSE;
}

BOOL CObjectList::OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	if (WM_MBUTTONDOWN == message)
	{
		if (m_cbViewChanged)
		{
			ChangeListCtrlView(1);
		}
	}
	else if (WM_MOUSEWHEEL == message)
	{
		if (m_cbViewChanged)
		{
			UINT uFlag = GET_FLAGS_LPARAM(wParam);
			if (uFlag & MK_CONTROL)
			{
				static DWORD dwLastTime = 0;

				DWORD dwTime = ::GetTickCount();

				if (500 < dwTime - dwLastTime)
				{
					ChangeListCtrlView((short)HIWORD(wParam));

					dwLastTime = dwTime;
				}

				return TRUE;
			}
		}
	}
	else if (WM_LBUTTONDBLCLK == message)
	{
		m_cbLButtondown = NULL;
	}
	else
	{
		if (m_cbTrackMouseEvent)
		{
			if (WM_MOUSEMOVE == message)
			{
				if (!m_bTrackingMouse)
				{
					TRACKMOUSEEVENT tme;
					memzero(tme);
					tme.cbSize = sizeof(tme);
					tme.hwndTrack = m_hWnd;
					tme.dwFlags = TME_LEAVE | TME_HOVER;
					tme.dwHoverTime = HOVER_DEFAULT;
					m_bTrackingMouse = TRUE==::TrackMouseEvent(&tme);
				}

				return TRUE;
			}
			else if (WM_MOUSELEAVE == message)
			{
				m_bTrackingMouse = false;

				m_cbTrackMouseEvent(E_TrackMouseEvent::LME_MouseLeave, CPoint(lParam));
				return TRUE;
			}
			else if (WM_MOUSEHOVER == message)
			{
				//m_bTrackingMouse = false;

				m_cbTrackMouseEvent(E_TrackMouseEvent::LME_MouseHover, CPoint(lParam));
				return TRUE;
			}
		}

		if (WM_DESTROY == message)
		{
			m_AsyncTaskTimer.kill();
		}
	}

	return __super::OnWndMsg(message, wParam, lParam, pResult);
}

void CObjectList::ChangeListCtrlView(short zDelta)
{
	E_ListViewType lpViewType[] {
		E_ListViewType::LVT_Report,
		E_ListViewType::LVT_List,
		//E_ListViewType::LVT_SmallIcon,
		E_ListViewType::LVT_Tile,
		E_ListViewType::LVT_Icon,
	};

	E_ListViewType eViewType = GetView();

	int nMax = sizeof(lpViewType) / sizeof(E_ListViewType);
	for (int nIndex = 0; nIndex < nMax; ++nIndex)
	{
		if (lpViewType[nIndex] == eViewType)
		{
			if (zDelta < 0)
			{
				nIndex++;

				if (nIndex >= nMax)
				{
					nIndex = 0;
				}
			}
			else
			{
				nIndex--;
				if (nIndex < 0)
				{
					nIndex = nMax-1;
				}
			}

			m_para.eViewType = lpViewType[nIndex];

			if (m_cbViewChanged)
			{
				m_cbViewChanged(m_para.eViewType);
			}

			SetView(m_para.eViewType);

			break;
		}
	}
}

UINT CObjectList::GetHeaderHeight()
{
	if (GetView() != E_ListViewType::LVT_Report)
	{
		return 0;
	}

	CRect rcHeader;
	CListCtrl::GetHeaderCtrl()->GetWindowRect(&rcHeader);

	return (UINT)rcHeader.Height();
}

const LVHITTESTINFO& CObjectList::hittest(const POINT& ptPos) const
{
	static LVHITTESTINFO htinfo;
	memzero(htinfo);

	htinfo.pt = ptPos;
	(void)__super::HitTest(&htinfo);

	return htinfo;
}

void CObjectList::_AsyncTask(UINT uElapse, cfn_void_t<UINT> cb)
{
	if (GetView() != E_ListViewType::LVT_Report)
	{
		return;
	}

	int nItemCount = GetItemCount();
	if (0 == nItemCount)
	{
		m_AsyncTaskTimer.kill();
		return;
	}
	m_vecAsyncTaskStatus.assign((UINT)nItemCount, FALSE);

	m_AsyncTaskTimer.set(uElapse, [&, cb]{
		if (GetView() != E_ListViewType::LVT_Report)
		{
			return false;
		}

		int nTopItem = GetTopIndex();
		if (nTopItem < 0)
		{
			return false;
		}

		UINT uCount = MIN((UINT)GetItemCount(), m_vecAsyncTaskStatus.size());
		for (UINT uItem = (UINT)nTopItem; uItem < uCount; uItem++)
		{
			auto& bAsyncTaskStatus = m_vecAsyncTaskStatus[uItem];
			if (!bAsyncTaskStatus)
			{
				bAsyncTaskStatus = TRUE;
				cb(uItem);
				return true;
			}
		}

		for (UINT uItem = 0; uItem < (UINT)nTopItem; uItem++)
		{
			auto& bAsyncTaskStatus = m_vecAsyncTaskStatus[uItem];
			if (!bAsyncTaskStatus)
			{
				bAsyncTaskStatus = TRUE;
				cb(uItem);
				return true;
			}
		}

		return false;
	});
}

void CObjectList::AsyncTask(UINT uElapse, cfn_void_t<UINT> cb)
{
	_AsyncTask(uElapse, [&, cb](UINT uItem) {
		cb(uItem);
		Update(uItem);
	});
}

void CObjectList::AsyncTask(UINT uElapse, cfn_bool_t<CListObject&> cb)
{
	_AsyncTask(uElapse, [&, cb](UINT uItem) {
		auto pObject = GetItemObject(uItem);
		if (pObject)
		{
			if (cb(*pObject))
			{
				UpdateItem(uItem, pObject);
			}
			else
			{
				Update(uItem);
			}
		}
	});
}

void CObjectList::AsyncLButtondown(cfn_void cb)
{
	m_cbLButtondown = cb;

	auto hWnd = m_hWnd;
	__async(300, [&, hWnd]{
		if (::IsWindowVisible(hWnd))
		{
			if (m_cbLButtondown)
			{
				m_cbLButtondown();
			}
		}
	});
}
