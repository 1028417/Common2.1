
#pragma once

#include <img.h>

#include <Guide.h>

class __CommonPrjExt CBaseTree: public CTreeCtrl
{
public:
	CBaseTree();

	virtual ~CBaseTree();

	CImglst m_Imglst;

	DECLARE_MESSAGE_MAP()

private:
	CFontGuide m_fontGuide;

public:
	BOOL InitImglst(const CSize& size, const TD_IconVec& vecIcons = {});

	BOOL InitImglst(CBitmap& Bitmap);

	void SetImageList(CImglst& imglst)
	{
		(void)__super::SetImageList(&imglst, TVSIL_NORMAL);
	}

	void SetFont(ULONG uFontSize);

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

class CTreeObject;

typedef ptrlist<CTreeObject*> TD_TreeObjectList;

class __CommonPrjExt CTreeObject
{
public:
	CTreeObject()
	{
		m_hTreeItem = NULL;
	}

public:
	HTREEITEM m_hTreeItem;

	virtual CString GetTreeText()
	{
		return _T("");
	};

	virtual int GetTreeImage()
	{
		return 0;
	}

	virtual void GetTreeChilds(TD_TreeObjectList& lstChilds)
	{
		//do nothing
	}
};


class __CommonPrjExt CObjectTree: public CBaseTree
{
public:
	CObjectTree();

	virtual ~CObjectTree();

public:
	void SetRootObject(CTreeObject& Object);

	virtual HTREEITEM InsertObject(CTreeObject& Object, CTreeObject *pParentObject=NULL);
	HTREEITEM InsertObjectEx(CTreeObject& Object, CTreeObject *pParentObject=NULL);

	void UpdateImage(CTreeObject& Object);

	CTreeObject *GetSelectedObject();

	CTreeObject *GetItemObject(HTREEITEM hItem);

	CTreeObject *GetParentObject(CTreeObject& Object);
	
	void GetAllObjects(TD_TreeObjectList& lstObjects);

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

// CCheckObjectTree
class __CommonPrjExt CCheckObjectTree: public CObjectTree
{
public:
	CCheckObjectTree();
	virtual ~CCheckObjectTree();

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
