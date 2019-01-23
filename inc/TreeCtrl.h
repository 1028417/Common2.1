
#pragma once

#include <img.h>

#include <Guard.h>

class __CommonExt CBaseTree: public CTreeCtrl
{
public:
	~CBaseTree()
	{
		(void)m_Imglst.DeleteImageList();
	}

	DECLARE_MESSAGE_MAP()

private:
	CFontGuard m_fontGuard;

	CImglst m_Imglst;

public:
	BOOL InitImglst(const CSize& size, const TD_IconVec& vecIcons = {});

	BOOL InitImglst(CBitmap& Bitmap);

	void SetImageList(CImglst& imglst)
	{
		(void)__super::SetImageList(&imglst, TVSIL_NORMAL);
	}

	void SetFontSize(int iFontSizeOffset);
	
	void GetAllItems(list<HTREEITEM>& lstItems);
	
protected:
	virtual HTREEITEM InsertItem(HTREEITEM hParentItem, LPCTSTR lpszItem, DWORD_PTR dwData, int nImage=0);

	void GetChildItems(HTREEITEM hItem, list<HTREEITEM>& lstItems);

public:
	virtual void PreSubclassWindow() override;

	virtual BOOL OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult) override;

	virtual BOOL OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult) override;

protected:
	virtual BOOL handleNMNotify(NMHDR& NMHDR) { return FALSE; }
};


//CObjectTree

class __CommonExt CObjectTree: public CBaseTree
{
public:
	static HTREEITEM getTreeItem(const CTreeObject& Object)
	{
		return (HTREEITEM)Object.m_hTreeItem;
	}

	static HTREEITEM getTreeItem(const CTreeObject *pObject)
	{
		return pObject ? (HTREEITEM)pObject->m_hTreeItem : NULL;
	}

	void SetRootObject(CTreeObject& Object);

	virtual HTREEITEM InsertObject(CTreeObject& Object, CTreeObject *pParentObject=NULL);
	HTREEITEM InsertObjectEx(CTreeObject& Object, CTreeObject *pParentObject=NULL);

	void UpdateImage(CTreeObject& Object);

	CTreeObject *GetSelectedObject();

	CTreeObject *GetItemObject(HTREEITEM hItem);

	CTreeObject *GetParentObject(CTreeObject& Object);
	
	void GetAllObjects(TD_TreeObjectList& lstObjects);

	BOOL EnsureVisible(const CTreeObject& Object)
	{
		return __super::EnsureVisible(getTreeItem(Object));
	}

	BOOL SelectObject(const CTreeObject& Object)
	{
		return __super::SelectItem(getTreeItem(Object));
	}

	BOOL ExpandObject(const CTreeObject& Object)
	{
		return __super::Expand(getTreeItem(Object), TVE_EXPAND);
	}

	CEdit* EditObject(const CTreeObject& Object)
	{
		return __super::EditLabel(getTreeItem(Object));
	}
	
private:
	virtual BOOL handleNMNotify(NMHDR& NMHDR) override;
};


enum E_CheckState
{
	CS_Nocheck = 0
	, CS_Unchecked
	, CS_Checked
	, CS_Grayed
};

// CObjectCheckTree
class __CommonExt CObjectCheckTree : public CObjectTree
{
public:
	~CObjectCheckTree()
	{
		(void)m_StateImageList.DeleteImageList();
	}

	DECLARE_MESSAGE_MAP()

public:
	BOOL InitCtrl();

protected:
	CImageList m_StateImageList;

public:
	HTREEITEM InsertObject(CTreeObject& Object, CTreeObject *pParentObject=NULL);

	E_CheckState GetItemCheckState(HTREEITEM hItem);

	void SetItemCheckState(HTREEITEM hItem, E_CheckState eCheckState);
	
	void GetAllObjects(TD_TreeObjectList& lstObjects);

	void GetAllObjects(TD_TreeObjectList& lstObjects, E_CheckState eCheckState);
	
	void GetCheckedObjects(TD_TreeObjectList& lstObjects);

private:
	void SetChildItemsImageState(HTREEITEM hItem);
	void SetParentItemsImageState(HTREEITEM hItem);

protected:
	virtual BOOL OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult);
};
