
#include "StdAfx.h"

#include <TreeCtrl.h>

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
	(void)m_CompatableFont.setFont(*this, fFontSizeOffset);
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

BOOL CBaseTree::OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	if (WM_LBUTTONDOWN == message || WM_RBUTTONDOWN == message)
	{
		CPoint ptCursor(0,0);
		(void)::GetCursorPos(&ptCursor);
		this->ScreenToClient(&ptCursor);

		HTREEITEM hItem = this->HitTest(ptCursor);
		if (hItem)
		{
			(void)this->SelectItem(hItem);
		}
	}
	else if (WM_KEYUP == message)
	{
		if (VK_RETURN == GET_KEYSTATE_LPARAM(wParam))
		{
			CEdit *pEdit = GetEditControl();
			if (NULL == pEdit)
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
	}

	return __super::OnWndMsg(message, wParam, lParam, pResult);
}

BOOL CBaseTree::OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	if (WM_NOTIFY == message)
	{
		NMHDR *pNMHDR = (NMHDR*)lParam;
		if (NULL != pNMHDR)
		{
			if (handleNMNotify(*pNMHDR))
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
	HBITMAP hStateBitmap = g_ResModule.loadBitmap(IDB_CHECKTREE_STATE);
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

HTREEITEM CObjectCheckTree::InsertObject(CTreeObject& Object, CTreeObject *pParentObject)
{
	HTREEITEM hItem = __super::InsertObject(Object, pParentObject);

	(void)__super::SetItemState(hItem, INDEXTOSTATEIMAGEMASK(CS_Unchecked), TVIS_STATEIMAGEMASK);

	return hItem;
}

void CObjectCheckTree::SetItemCheckState(HTREEITEM hItem, E_CheckState eCheckState)
{
	(void)__super::SetItemState(hItem, INDEXTOSTATEIMAGEMASK(eCheckState), TVIS_STATEIMAGEMASK);

	this->SetChildItemsImageState(hItem);

	this->SetParentItemsImageState(hItem);
}

E_CheckState CObjectCheckTree::GetItemCheckState(HTREEITEM hItem)
{
	return (E_CheckState)(__super::GetItemState(hItem, TVIS_STATEIMAGEMASK) >>12);
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
		if (eCheckState == this->GetItemCheckState(getTreeItem(&TreeObject)))
		{
			lstObjects.add(TreeObject);
		}
	});
}

void CObjectCheckTree::GetCheckedObjects(TD_TreeObjectList& lstObjects)
{
	TD_TreeObjectList lstChekedObjects;
	this->GetAllObjects(lstChekedObjects, CS_Checked);

	HTREEITEM hParentItem = NULL;
	lstChekedObjects([&](CTreeObject& CheckedObject) {
		if (!lstChekedObjects.includes(GetParentObject(CheckedObject)))
		{
			lstObjects.add(CheckedObject);
		}
	});
}

BOOL CObjectCheckTree::OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	HTREEITEM hItem = NULL;

	if (WM_KEYDOWN == message)
	{
		 auto uKey = GET_KEYSTATE_LPARAM(wParam);
		if (VK_SPACE == uKey)
		{
			hItem = __super::GetSelectedItem();
		}
	}
	else if (WM_LBUTTONDOWN == message)
	{
		CPoint ptCursor(0,0);
		(void)::GetCursorPos(&ptCursor);

		__super::ScreenToClient(&ptCursor);

		UINT uFlag = 0;
		hItem = __super::HitTest(ptCursor, &uFlag);
		if (TVHT_ONITEMSTATEICON != uFlag)
		{
			hItem = NULL;
		}
	}

	if (hItem)
	{
		E_CheckState eCheckState = (E_CheckState)this->GetItemCheckState(hItem);
		if (eCheckState != CS_Checked)
		{
			eCheckState = CS_Checked;
		}
		else
		{
			eCheckState = CS_Unchecked;
		}

		this->SetItemCheckState(hItem, eCheckState);

		if (m_cbCheckChanged)
		{
			m_cbCheckChanged(eCheckState);
		}
	}

	return __super::OnWndMsg(message, wParam, lParam, pResult);
}

void CObjectCheckTree::SetChildItemsImageState(HTREEITEM hItem)
{
	UINT uState = __super::GetItemState(hItem, TVIS_STATEIMAGEMASK);

	HTREEITEM hChildItem = __super::GetChildItem(hItem);

	while (hChildItem)
	{
		(void)__super::SetItemState(hChildItem, uState, TVIS_STATEIMAGEMASK);

		if (__super::GetChildItem(hChildItem))
		{
			this->SetChildItemsImageState(hChildItem);
		}

		hChildItem = __super::GetNextItem(hChildItem, TVGN_NEXT);
	}
}

void CObjectCheckTree::SetParentItemsImageState(HTREEITEM hItem)
{
	HTREEITEM hParentItem = hItem;

	HTREEITEM hChildItem = NULL;
	
	UINT uChildState = 0;

	UINT uChildState2 = 0;

	
	while (hParentItem = __super::GetParentItem(hParentItem))
	{
		if (CS_Nocheck == this->GetItemCheckState(hParentItem))
		{
			continue;
		}

		hChildItem = __super::GetChildItem(hParentItem);
		ASSERT(hChildItem);

		uChildState = this->GetItemCheckState(hChildItem);

		while (hChildItem = __super::GetNextItem(hChildItem, TVGN_NEXT))
		{
			uChildState2 = this->GetItemCheckState(hChildItem);

			if (CS_Nocheck == uChildState2)
			{
				continue;
			}

			if (uChildState != uChildState2)
			{
				uChildState = CS_Grayed;
				break;
			}
		}

		(void)__super::SetItemState(hParentItem, INDEXTOSTATEIMAGEMASK(uChildState), TVIS_STATEIMAGEMASK);
	}
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
	if (pParentObject && pParentObject->m_hTreeItem)
	{
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
	Object.m_hTreeItem = hTreeItem;
	
	return hTreeItem;
}

HTREEITEM CObjectTree::InsertObjectEx(CTreeObject& Object, CTreeObject *pParentObject)
{
	Object.m_hTreeItem = InsertObject(Object, pParentObject);
	__EnsureReturn(getTreeItem(Object), NULL);

	TD_TreeObjectList lstSubObjects;
	Object.GetTreeChilds(lstSubObjects);

	lstSubObjects([&](CTreeObject& SubObject) {
		(void)InsertObjectEx(SubObject, &Object);
	});

	return getTreeItem(Object);
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

BOOL CObjectTree::handleNMNotify(NMHDR& NMHDR)
{
	if (TVN_ENDLABELEDIT == NMHDR.code)
	{
		NMTVDISPINFO *pTVDispInfo = reinterpret_cast<NMTVDISPINFO*>(&NMHDR);

		CString cstrNewName(pTVDispInfo->item.pszText);
		(void)cstrNewName.Trim();
		__EnsureReturn(!cstrNewName.IsEmpty(), TRUE);

		CTreeObject *pObject = GetItemObject(pTVDispInfo->item.hItem);
		__AssertReturn(pObject, TRUE);

		__EnsureReturn(cstrNewName != pObject->GetTreeText().c_str(), TRUE);
	}

	return __super::handleNMNotify(NMHDR);
}
