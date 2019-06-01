
#pragma once

#include <img.h>

#include <Guard.h>

class __CommonExt CBaseTree: public CTouchWnd<CTreeCtrl>
{
public:
	~CBaseTree()
	{
		(void)m_Imglst.DeleteImageList();
	}

	DECLARE_MESSAGE_MAP()

private:
	CCompatableFont m_CompatableFont;

	CImglst m_Imglst;

public:
	BOOL InitImglst(const CSize& size, const TD_IconVec& vecIcons = {});

	BOOL InitImglst(CBitmap& Bitmap);

	void SetImageList(CImglst& imglst)
	{
		(void)__super::SetImageList(&imglst, TVSIL_NORMAL);
	}

	void SetFontSize(float fFontSizeOffset);
	
	void GetAllItems(list<HTREEITEM>& lstItems);
	
protected:
	virtual HTREEITEM InsertItem(HTREEITEM hParentItem, LPCTSTR lpszItem, DWORD_PTR dwData, int nImage=0);

	void GetChildItems(HTREEITEM hItem, list<HTREEITEM>& lstItems);

public:
	virtual void PreSubclassWindow() override;

	virtual BOOL OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult) override;

	virtual BOOL OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult) override;
	
	virtual BOOL PreTranslateMessage(MSG* pMsg) override;

protected:
	virtual BOOL handleNMNotify(NMHDR& NMHDR) { return FALSE; }
};


//CObjectTree

class __CommonExt CObjectTree: public CBaseTree
{
private:
	map<const CTreeObject*, HTREEITEM> m_mapTreeObject;

public:
	inline HTREEITEM getTreeItem(const CTreeObject *pObject)
	{
		auto itr = m_mapTreeObject.find(pObject);
		if (itr == m_mapTreeObject.end())
		{
			return NULL;
		}

		return itr->second;
	}

	inline HTREEITEM getTreeItem(const CTreeObject& Object)
	{
		return getTreeItem(&Object);
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

	BOOL ExpandObject(const CTreeObject& Object, bool bExpand = true)
	{
		return __super::Expand(getTreeItem(Object), bExpand ? TVE_EXPAND : TVE_COLLAPSE);
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

	void setCheckChangedCB(const function<void(E_CheckState)>& cbCheckChanged)
	{
		m_cbCheckChanged = cbCheckChanged;
	}

protected:
	CImageList m_StateImageList;

private:
	function<void(E_CheckState)> m_cbCheckChanged;

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

	void _onItemClick(HTREEITEM hItem);

protected:
	virtual BOOL PreTranslateMessage(MSG* pMsg) override;
};
