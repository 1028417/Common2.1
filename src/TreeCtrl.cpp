
#include "StdAfx.h"

#include <TreeCtrl.h>

#include "Resource.h"

// CBaseTree

CBaseTree::CBaseTree()
{
}

CBaseTree::~CBaseTree()
{
	(void)m_Imglst.DeleteImageList();
}

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

void CBaseTree::SetFont(ULONG uFontSize)
{
	if (0 < uFontSize)
	{
		(void)m_fontGuide.setFontSize(*this, uFontSize);
	}
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


// CCheckObjectTree

CCheckObjectTree::CCheckObjectTree()
{
}

CCheckObjectTree::~CCheckObjectTree()
{
	(void)m_StateImageList.DeleteImageList();
}

BEGIN_MESSAGE_MAP(CCheckObjectTree, CObjectTree)
END_MESSAGE_MAP()

// CCheckObjectTree 消息处理程序

BOOL CCheckObjectTree::InitCtrl()
{
	HBITMAP hStateBitmap = ::LoadBitmap(g_hInstance, MAKEINTRESOURCE(IDB_CHECKTREE_STATE));
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

HTREEITEM CCheckObjectTree::InsertObject(CTreeObject& Object, CTreeObject *pParentObject)
{
	HTREEITEM hItem = __super::InsertObject(Object, pParentObject);

	(void)__super::SetItemState(hItem, INDEXTOSTATEIMAGEMASK(CS_Unchecked), TVIS_STATEIMAGEMASK);

	return hItem;
}

void CCheckObjectTree::SetItemCheckState(HTREEITEM hItem, E_CheckState eCheckState)
{
	(void)__super::SetItemState(hItem, INDEXTOSTATEIMAGEMASK(eCheckState), TVIS_STATEIMAGEMASK);

	this->SetChildItemsImageState(hItem);

	this->SetParentItemsImageState(hItem);
}

E_CheckState CCheckObjectTree::GetItemCheckState(HTREEITEM hItem)
{
	return (E_CheckState)(__super::GetItemState(hItem, TVIS_STATEIMAGEMASK) >>12);
}

void CCheckObjectTree::GetAllObjects(TD_TreeObjectList& lstObjects)
{
	list<HTREEITEM> lstItems;
	__super::GetAllItems(lstItems);

	for (list<HTREEITEM>::iterator itItem = lstItems.begin()
		; itItem != lstItems.end(); ++itItem)
	{
		lstObjects.push_back(__super::GetItemObject(*itItem));
	}
}

void CCheckObjectTree::GetAllObjects(TD_TreeObjectList& lstObjects, E_CheckState eCheckState)
{
	TD_TreeObjectList lstItemObjects;
	__super::GetAllObjects(lstItemObjects);

	for (TD_TreeObjectList::iterator itObject = lstItemObjects.begin()
		; itObject != lstItemObjects.end(); ++itObject)
	{
		if (eCheckState == this->GetItemCheckState((*itObject)->m_hTreeItem))
		{
			lstObjects.push_back(*itObject);
		}
	}
}

void CCheckObjectTree::GetCheckedObjects(TD_TreeObjectList& lstObjects)
{
	TD_TreeObjectList lstChekedObjects;
	this->GetAllObjects(lstChekedObjects, CS_Checked);

	HTREEITEM hParentItem = NULL;
	for (TD_TreeObjectList::iterator itObject = lstChekedObjects.begin()
		; itObject != lstChekedObjects.end(); ++itObject)
	{
		hParentItem = this->GetParentItem((*itObject)->m_hTreeItem);

		if (!util::ContainerFind(lstChekedObjects, this->GetItemObject(hParentItem)))
		{
			lstObjects.push_back(*itObject);
		}
	}
}

BOOL CCheckObjectTree::OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	HTREEITEM hItem = NULL;

	if (WM_KEYDOWN == message)
	{
		int nChar = LOWORD(wParam);
		if (VK_SPACE == nChar)
		{
			hItem = __super::GetSelectedItem();
		}
	}
	else if (WM_LBUTTONDOWN == message)
	{
		CPoint ptCursor(0,0);
		(void)::GetCursorPos(&ptCursor);

		__super::ScreenToClient(&ptCursor);

		UINT nFlag = 0;
		hItem = __super::HitTest(ptCursor, &nFlag);
		if (TVHT_ONITEMSTATEICON != nFlag)
		{
			hItem = NULL;
		}
	}

	if (hItem)
	{
		UINT nState = this->GetItemCheckState(hItem);
		this->SetItemCheckState(hItem, nState!=CS_Checked? CS_Checked:CS_Unchecked);
	}

	return __super::OnWndMsg(message, wParam, lParam, pResult);
}

void CCheckObjectTree::SetChildItemsImageState(HTREEITEM hItem)
{
	UINT nState = __super::GetItemState(hItem, TVIS_STATEIMAGEMASK);

	HTREEITEM hChildItem = __super::GetChildItem(hItem);

	while (hChildItem)
	{
		(void)__super::SetItemState(hChildItem, nState, TVIS_STATEIMAGEMASK);

		if (__super::GetChildItem(hChildItem))
		{
			this->SetChildItemsImageState(hChildItem);
		}

		hChildItem = __super::GetNextItem(hChildItem, TVGN_NEXT);
	}
}

void CCheckObjectTree::SetParentItemsImageState(HTREEITEM hItem)
{
	HTREEITEM hParentItem = hItem;

	HTREEITEM hChildItem = NULL;
	
	UINT nChildState = 0;

	UINT nChildState2 = 0;

	
	while (hParentItem = __super::GetParentItem(hParentItem))
	{
		if (CS_Nocheck == this->GetItemCheckState(hParentItem))
		{
			continue;
		}

		hChildItem = __super::GetChildItem(hParentItem);
		ASSERT(hChildItem);

		nChildState = this->GetItemCheckState(hChildItem);

		while (hChildItem = __super::GetNextItem(hChildItem, TVGN_NEXT))
		{
			nChildState2 = this->GetItemCheckState(hChildItem);

			if (CS_Nocheck == nChildState2)
			{
				continue;
			}

			if (nChildState != nChildState2)
			{
				nChildState = CS_Grayed;
				break;
			}
		}

		(void)__super::SetItemState(hParentItem, INDEXTOSTATEIMAGEMASK(nChildState), TVIS_STATEIMAGEMASK);
	}
}


// CObjectTree
CObjectTree::CObjectTree()
{
}

CObjectTree::~CObjectTree()
{
}

void CObjectTree::SetRootObject(CTreeObject& Object)
{
	(void)DeleteAllItems();

	(void)InsertObjectEx(Object);

	(void)__super::SelectItem(Object.m_hTreeItem);
	(void)__super::EnsureVisible(Object.m_hTreeItem);
	(void)__super::Expand(Object.m_hTreeItem, TVE_EXPAND);
}

HTREEITEM CObjectTree::InsertObject(CTreeObject& Object, CTreeObject *pParentObject)
{
	HTREEITEM hParentItem = TVI_ROOT;
	if (pParentObject && pParentObject->m_hTreeItem)
	{
		TVITEM item;
		ZeroMemory(&item, sizeof(item));
		item.hItem = pParentObject->m_hTreeItem;
		item.mask = TVIF_PARAM;
		(void)::SendMessage(m_hWnd, TVM_GETITEM, 0, (LPARAM)&item);
		
		if ((CTreeObject*)item.lParam == pParentObject)
		{
			hParentItem = pParentObject->m_hTreeItem;
		}
	}
	
	return Object.m_hTreeItem = __super::InsertItem(hParentItem, Object.GetTreeText()
		, (DWORD_PTR)&Object, Object.GetTreeImage());
}

HTREEITEM CObjectTree::InsertObjectEx(CTreeObject& Object, CTreeObject *pParentObject)
{
	Object.m_hTreeItem = InsertObject(Object, pParentObject);
	__EnsureReturn(Object.m_hTreeItem, NULL);

	TD_TreeObjectList lstSubObjects;
	Object.GetTreeChilds(lstSubObjects);

	for (TD_TreeObjectList::iterator itSubObject=lstSubObjects.begin()
		; itSubObject!=lstSubObjects.end(); ++itSubObject)
	{
		(void)InsertObjectEx(**itSubObject, &Object);
	}

	return Object.m_hTreeItem;
}

void CObjectTree::UpdateImage(CTreeObject& Object)
{
	int iImage = Object.GetTreeImage();
	SetItemImage(Object.m_hTreeItem, iImage, iImage);
	this->Invalidate(FALSE);
}

CTreeObject *CObjectTree::GetSelectedObject()
{
	HTREEITEM hItem = this->GetSelectedItem();
	if (!hItem)
	{
		return NULL;
	}

	return GetItemObject(hItem);
}

CTreeObject *CObjectTree::GetItemObject(HTREEITEM hItem)
{
	return (CTreeObject*)__super::GetItemData(hItem);
}

CTreeObject *CObjectTree::GetParentObject(CTreeObject& Object)
{
	__AssertReturn(Object.m_hTreeItem, NULL);
	
	HTREEITEM hItemParent = __super::GetParentItem(Object.m_hTreeItem);
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
		lstObjects.push_back(this->GetItemObject(*itItem));
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

		__EnsureReturn(cstrNewName != pObject->GetTreeText(), TRUE);
	}

	return __super::handleNMNotify(NMHDR);
}
