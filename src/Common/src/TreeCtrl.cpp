
#include "StdAfx.h"

#include "TreeCtrl.h"

BEGIN_MESSAGE_MAP(CBaseTree, CTreeCtrl)
END_MESSAGE_MAP()

// CBaseTree 消息处理程序

void CBaseTree::PreSubclassWindow()
{
	__super::PreSubclassWindow();

	(void)ModifyStyle(0, TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT | TVS_SHOWSELALWAYS | LVS_SHAREIMAGELISTS);
}

BOOL CBaseTree::InitImglst(const CSize& size, const TD_IconVec& vecIcons)
{
	__AssertReturn(m_Imglst.Init(size, vecIcons), FALSE);
	(void)__super::SetImageList(&m_Imglst, TVSIL_NORMAL);
	
	return TRUE;
}

BOOL CBaseTree::InitImglst(CBitmap& Bitmap)
{
	__AssertReturn(m_Imglst.Init(Bitmap), FALSE);
	(void)__super::SetImageList(&m_Imglst, TVSIL_NORMAL);

	return TRUE;
}

void CBaseTree::SetFontSize(float fFontSizeOffset)
{
	(void)m_font.setFont(*this, fFontSizeOffset);
}

HTREEITEM CBaseTree::InsertItem(HTREEITEM hParentItem, LPCTSTR lpszItem, DWORD_PTR dwData, int nImage)
{
	if (!hParentItem)
	{
		hParentItem = TVI_ROOT;
	}

	HTREEITEM hItem = __super::InsertItem(lpszItem, nImage, nImage, hParentItem);
	
	(void)__super::SetItemData(hItem, dwData);

	return hItem;
}

void CBaseTree::RedrawItem(HTREEITEM hItem)
{
	CRect rcItem;
	if (GetItemRect(hItem, rcItem, FALSE))
	{
		this->InvalidateRect(rcItem);
	}
}

void CBaseTree::GetAllItems(list<HTREEITEM>& lstItems)
{
	HTREEITEM hRootItem = this->GetRootItem();
	while (hRootItem)
	{
		lstItems.push_back(hRootItem);

		GetChildItems(hRootItem, lstItems);

		hRootItem = this->GetNextItem(hRootItem, TVGN_NEXT);
	}
}

void CBaseTree::GetChildItems(HTREEITEM hItem, list<HTREEITEM>& lstItems)
{
	HTREEITEM hChildItem = this->GetChildItem(hItem);
	while (hChildItem)
	{
		lstItems.push_back(hChildItem);

		GetChildItems(hChildItem, lstItems);

		hChildItem = this->GetNextItem(hChildItem, TVGN_NEXT);
	}
}

BOOL CBaseTree::PreTranslateMessage(MSG* pMsg)
{
	if (WM_KEYUP == pMsg->message)
	{
		WORD uVkKey = GET_KEYSTATE_LPARAM(pMsg->wParam);
		if (pMsg->hwnd == m_hWnd)
		{
			if (VK_RETURN == uVkKey)
			{
				HTREEITEM hItem = GetSelectedItem();
				if (NULL != hItem)
				{
					if (GetItemState(hItem, TVIS_EXPANDED) & TVIS_EXPANDED)
					{
						Expand(hItem, TVE_COLLAPSE);
					}
					else
					{
						Expand(hItem, TVE_EXPAND);
					}
				}
			}
		}
		else
		{
			if (VK_RETURN == uVkKey || VK_ESCAPE == uVkKey)
			{
				CEdit *pEdit = this->GetEditControl();
				if (pEdit->GetSafeHwnd() == pMsg->hwnd)
				{	
					if (VK_ESCAPE == uVkKey)
					{
						(void)pEdit->SetWindowText(NULL);
					}

					(void)this->SetFocus();

					return TRUE;
				}
			}
		}
	}

	return __super::PreTranslateMessage(pMsg);
}

BOOL CBaseTree::OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
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


// CObjectCheckTree

BEGIN_MESSAGE_MAP(CObjectCheckTree, CObjectTree)
END_MESSAGE_MAP()

// CObjectCheckTree 消息处理程序

BOOL CObjectCheckTree::InitCtrl()
{
	CResModule ResModule("Common");
	HBITMAP hStateBitmap = ResModule.loadBitmap(IDB_CHECKTREE_STATE);
	__AssertReturn(hStateBitmap, FALSE);

	CBitmap StateBitmap;
	StateBitmap.Attach(hStateBitmap);

	BITMAP bmp;
	(void)StateBitmap.GetBitmap(&bmp);

	__AssertReturn(m_StateImageList.Create(bmp.bmHeight, bmp.bmHeight, ILC_COLOR32, 0, 0), FALSE);
	(void)m_StateImageList.Add(&StateBitmap, __Color_Black);
	
	(void)StateBitmap.DeleteObject();

	(void)CTreeCtrl::SetImageList(&m_StateImageList, TVSIL_STATE);

	(void)ModifyStyle(TVS_CHECKBOXES, 0);

	return TRUE;
}

void CObjectCheckTree::SetRootObject(CTreeObject& Object, bool bShowNocheck)
{
	m_bShowNocheck = bShowNocheck;

	__super::SetRootObject(Object);
}

HTREEITEM CObjectCheckTree::InsertObject(CTreeObject& Object, CTreeObject *pParentObject)
{
	auto eCheckState = CS_Unchecked;
	if (!Object.hasCheckState())
	{
		if (!m_bShowNocheck)
		{
			return NULL;
		}
		eCheckState = CS_Nocheck;
	}

	HTREEITEM hItem = __super::InsertObject(Object, pParentObject);

	_setImgMask(hItem, eCheckState);

	return hItem;
}

void CObjectCheckTree::_setImgMask(HTREEITEM hItem, E_CheckState eCheckState)
{
	(void)__super::SetItemState(hItem, INDEXTOSTATEIMAGEMASK(eCheckState), TVIS_STATEIMAGEMASK);
}

void CObjectCheckTree::_setCheckState(HTREEITEM hItem, E_CheckState eCheckState)
{
	_setImgMask(hItem, eCheckState);

	_setChildsState(hItem, eCheckState);

	_updateParentState(hItem);
}

void CObjectCheckTree::SetCheckState(CTreeObject& Object, bool bCheck)
{
	if (Object.hasCheckState())
	{
		_setCheckState(getTreeItem(Object), bCheck? CS_Checked:CS_Unchecked);
	}
}

E_CheckState CObjectCheckTree::_getCheckState(HTREEITEM hItem)
{
	return (E_CheckState)(__super::GetItemState(hItem, TVIS_STATEIMAGEMASK) >>12);
}

E_CheckState CObjectCheckTree::GetCheckState(CTreeObject& Object)
{
	if (!Object.hasCheckState())
	{
		return CS_Nocheck;
	}

	return _getCheckState(getTreeItem(Object));
}

BOOL CObjectCheckTree::PreTranslateMessage(MSG* pMsg)
{
	switch (pMsg->message)
	{
	case WM_KEYUP:
		if (pMsg->hwnd == m_hWnd)
		{
			if (VK_SPACE == GET_KEYSTATE_LPARAM(pMsg->wParam))
			{
				_onItemClick(__super::GetSelectedItem());
			}
		}

		break;
	case WM_LBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
	{
		CPoint ptPos(pMsg->lParam);
		UINT uFlag = 0;
		HTREEITEM hItem = __super::HitTest(ptPos, &uFlag);
		if (TVHT_ONITEMSTATEICON == uFlag)
		{
			if (WM_LBUTTONDBLCLK == pMsg->message)
			{
				return TRUE;
			}

			_onItemClick(hItem);
		}
	}
	break;
	};

	return __super::PreTranslateMessage(pMsg);
}

void CObjectCheckTree::_onItemClick(HTREEITEM hItem)
{
	__Ensure(hItem);
	
	E_CheckState eCheckState = _getCheckState(hItem);
	if (CS_Nocheck == eCheckState)
	{
		return;
	}

	if (eCheckState != CS_Checked)
	{
		eCheckState = CS_Checked;
	}
	else
	{
		eCheckState = CS_Unchecked;
	}
	_setCheckState(hItem, eCheckState);

	if (m_cbCheckChanged)
	{
		m_cbCheckChanged(eCheckState);
	}
}

void CObjectCheckTree::_setChildsState(HTREEITEM hItem, E_CheckState eCheckState)
{
	hItem = __super::GetChildItem(hItem);
	while (hItem)
	{
		if (_getCheckState(hItem) != CS_Nocheck)
		{
			_setImgMask(hItem, eCheckState);
			
			if (GetChildItem(hItem))
			{
				_setChildsState(hItem, eCheckState);
			}
		}

		hItem = __super::GetNextItem(hItem, TVGN_NEXT);
	}
}

void CObjectCheckTree::_updateParentState(HTREEITEM hItem)
{
	while (hItem = __super::GetParentItem(hItem))
	{
		if (CS_Nocheck == _getCheckState(hItem))
		{
			continue;
		}

		HTREEITEM hChildItem = __super::GetChildItem(hItem);
		E_CheckState eChildState = _getCheckState(hChildItem);
		while (hChildItem = __super::GetNextItem(hChildItem, TVGN_NEXT))
		{
			E_CheckState eChildState2 = _getCheckState(hChildItem);
			if (CS_Nocheck == eChildState2)
			{
				continue;
			}

			if (eChildState != eChildState2)
			{
				eChildState = CS_Grayed;
				break;
			}
		}

		_setImgMask(hItem, eChildState);
	}
}

void CObjectCheckTree::GetAllObjects(TD_TreeObjectList& lstObjects)
{
	list<HTREEITEM> lstItems;
	__super::GetAllItems(lstItems);

	for (list<HTREEITEM>::iterator itItem = lstItems.begin()
		; itItem != lstItems.end(); ++itItem)
	{
		lstObjects.add(__super::GetItemObject(*itItem));
	}
}

void CObjectCheckTree::GetAllObjects(TD_TreeObjectList& lstObjects, E_CheckState eCheckState)
{
	TD_TreeObjectList lstTreeObjects;
	__super::GetAllObjects(lstTreeObjects);

	lstTreeObjects([&](CTreeObject& TreeObject) {
		if (eCheckState == GetCheckState(TreeObject))
		{
			lstObjects.add(TreeObject);
		}
	});
}

void CObjectCheckTree::GetCheckedObjects(TD_TreeObjectList& lstObjects)
{
	TD_TreeObjectList lstChekedObjects;
	GetAllObjects(lstChekedObjects, CS_Checked);

	lstObjects = lstChekedObjects.filter([&](CTreeObject& TreeObject) {
		return !lstChekedObjects.includes(GetParentObject(TreeObject));
	});
}

void CObjectTree::SetRootObject(CTreeObject& Object)
{
	(void)DeleteAllItems();

	(void)InsertObjectEx(Object);

	(void)__super::SelectItem(getTreeItem(Object));
	(void)__super::EnsureVisible(getTreeItem(Object));
	(void)ExpandObject(Object);
}

HTREEITEM CObjectTree::InsertObject(CTreeObject& Object, CTreeObject *pParentObject)
{
	HTREEITEM hParentItem = TVI_ROOT;
	if (pParentObject)
	{
		hParentItem = getTreeItem(pParentObject);

		TVITEM item;
		ZeroMemory(&item, sizeof(item));
		item.hItem = getTreeItem(pParentObject);
		item.mask = TVIF_PARAM;
		(void)::SendMessage(m_hWnd, TVM_GETITEM, 0, (LPARAM)&item);
		
		if ((CTreeObject*)item.lParam == pParentObject)
		{
			hParentItem = getTreeItem(pParentObject);
		}
	}
	
	HTREEITEM hTreeItem = __super::InsertItem(hParentItem
		, Object.GetTreeText().c_str(), (DWORD_PTR)&Object, Object.GetTreeImage());
	m_mapTreeObject[&Object] = hTreeItem;
	
	return hTreeItem;
}

HTREEITEM CObjectTree::InsertObjectEx(CTreeObject& Object, CTreeObject *pParentObject)
{
	auto hTreeItem = InsertObject(Object, pParentObject);
	__EnsureReturn(hTreeItem, NULL);

	m_mapTreeObject[&Object] = hTreeItem;

	TD_TreeObjectList lstSubObjects;
	Object.GetTreeChilds(lstSubObjects);

	lstSubObjects([&](CTreeObject& SubObject) {
		(void)InsertObjectEx(SubObject, &Object);
	});

	return hTreeItem;
}

void CObjectTree::UpdateImage(CTreeObject& Object)
{
	int iImage = Object.GetTreeImage();
	SetItemImage(getTreeItem(Object), iImage, iImage);
	this->Invalidate(FALSE);
}

CTreeObject *CObjectTree::GetSelectedObject()
{
	HTREEITEM hItem = this->GetSelectedItem();
	if (NULL == hItem)
	{
		return NULL;
	}

	return GetItemObject(hItem);
}

CTreeObject *CObjectTree::GetItemObject(HTREEITEM hItem)
{
	if (NULL == hItem)
	{
		return NULL;
	}

	return (CTreeObject*)__super::GetItemData(hItem);
}

CTreeObject *CObjectTree::GetParentObject(CTreeObject& Object)
{
	__AssertReturn(getTreeItem(Object), NULL);
	
	HTREEITEM hItemParent = __super::GetParentItem(getTreeItem(Object));
	__EnsureReturn(hItemParent, NULL);

	return this->GetItemObject(hItemParent);
}

void CObjectTree::GetAllObjects(TD_TreeObjectList& lstObjects)
{
	list<HTREEITEM> lstItems;
	__super::GetAllItems(lstItems);

	for (list<HTREEITEM>::iterator itItem = lstItems.begin()
		; itItem != lstItems.end(); ++itItem)
	{
		lstObjects.add(this->GetItemObject(*itItem));
	}
}

void CObjectTree::handleCustomDraw(NMTVCUSTOMDRAW& tvnmcd, LRESULT* pResult)
{
	auto& nmcd = tvnmcd.nmcd;
	if (CDDS_PREPAINT == nmcd.dwDrawStage)
	{
		*pResult = CDRF_NOTIFYITEMDRAW;
	}
	else if (CDDS_ITEMPREPAINT == nmcd.dwDrawStage)
	{
		tagTVCustomDraw tvcd(tvnmcd);
		tvcd.crText = GetTextColor();

		if (nmcd.uItemState & CDIS_SELECTED)
		{
			tvcd.crBkg = BkgColor_Select;
		}
		else
		{
			tvcd.crBkg = GetBkColor();
		}
		nmcd.uItemState &= ~CDIS_FOCUS;
		
		if (m_cbCustomDraw)
		{
			m_cbCustomDraw(tvcd);
			if (tvcd.bSkipDefault)
			{
				*pResult = CDRF_SKIPDEFAULT;
				return;
			}
		}
		
		cauto& uTextAlpha = tvcd.uTextAlpha;
		if (0 != uTextAlpha && uTextAlpha <= 255)
		{
			auto pb = (BYTE*)&tvcd.crText;
			int r = *pb;
			int g = pb[1];
			int b = pb[2];

			pb = (BYTE*)&tvcd.crBkg;
			r += (-r + pb[0])*uTextAlpha / 255;
			g += (-g + pb[1])*uTextAlpha / 255;
			b += (-b + pb[2])*uTextAlpha / 255;

			tvcd.crText = RGB(r, g, b);
		}
	}
}

BOOL CObjectTree::handleNMNotify(NMHDR& NMHDR, LRESULT* pResult)
{
	switch (NMHDR.code)
	{
	case NM_CUSTOMDRAW:
	{
		LPNMTVCUSTOMDRAW pTVCD = reinterpret_cast<LPNMTVCUSTOMDRAW>(&NMHDR);
		handleCustomDraw(*pTVCD, pResult);
		return TRUE;
	}
	
	break;
	case TVN_ENDLABELEDIT:
	{
		NMTVDISPINFO *pTVDispInfo = reinterpret_cast<NMTVDISPINFO*>(&NMHDR);

		CString cstrNewName(pTVDispInfo->item.pszText);
		(void)cstrNewName.Trim();
		__EnsureReturn(!cstrNewName.IsEmpty(), TRUE);

		CTreeObject *pObject = GetItemObject(pTVDispInfo->item.hItem);
		__AssertReturn(pObject, TRUE);

		__EnsureReturn(cstrNewName != pObject->GetTreeText().c_str(), TRUE);
	}

	break;
	}

	return __super::handleNMNotify(NMHDR, pResult);
}
