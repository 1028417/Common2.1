
#pragma once

#include <img.h>

#include <Guard.h>

class CListHeader : public CHeaderCtrl
{
	DECLARE_MESSAGE_MAP()

public:
	CListHeader(){}
	
private:
	UINT m_uHeight = 0;

	CFontGuard m_fontGuard;

private:
	LRESULT OnLayout(WPARAM wParam, LPARAM lParam);

public:
	BOOL Init(UINT uHeight, int iFontSizeOffset = 0);
};

enum class E_ListViewType
{
	LVT_Tile = LV_VIEW_TILE,
	LVT_Icon = LVS_ICON,
	LVT_SmallIcon = LVS_SMALLICON,
	LVT_List = LVS_LIST,
	LVT_Report = LVS_REPORT
};

// CObjectList

struct __CommonExt tagListColumn
{
	CString cstrText;
	UINT uWidth = 0;
	UINT uFormat = LVCFMT_LEFT;
};
typedef list<tagListColumn> TD_ListColumn;

class __CommonExt CListColumnGuard
{
public:
	CListColumnGuard()
	{
	}

	CListColumnGuard(const TD_ListColumn& lstColumn, UINT uTotalWidth=0)
		: m_lstColumn(lstColumn)
	{
		m_uTotalWidth = uTotalWidth;

		for (auto& column : m_lstColumn)
		{
			m_uFixWidth += column.uWidth;
		}
	}

	CListColumnGuard(UINT uTotalWidth)
	{
		m_uTotalWidth = uTotalWidth;
	}

private:
	UINT m_uTotalWidth = 0;

	TD_ListColumn m_lstColumn;

	list<pair<tagListColumn*, double>> m_lstPercentWidth;

	UINT m_uFixWidth = 0;

public:
	CListColumnGuard& add(const CString& cstrText, UINT uWidth, UINT uFormat=LVCFMT_LEFT)
	{
		m_lstColumn.push_back({ cstrText, uWidth, uFormat });
		
		m_uFixWidth += uWidth;

		return *this;
	}

	CListColumnGuard& add(const CString& cstrText, double fPercentWidth, UINT uFormat = LVCFMT_LEFT)
	{
		m_lstColumn.push_back({ cstrText, 0, uFormat });

		m_lstPercentWidth.push_back({ &m_lstColumn.back(), fPercentWidth });
		
		return *this;
	}

	const TD_ListColumn& getListColumn()
	{
		return getListColumn(m_uTotalWidth);
	}

	const TD_ListColumn& getListColumn(UINT uTotalWidth)
	{
		if (!m_lstPercentWidth.empty())
		{
			if (m_uFixWidth < uTotalWidth)
			{
				UINT uRamainWidth = uTotalWidth - m_uFixWidth;
				for (auto& pr : m_lstPercentWidth)
				{
					pr.first->uWidth = UINT(uRamainWidth*pr.second);
				}
			}
		}
		else
		{
			if (0 != uTotalWidth)
			{
				if (m_uFixWidth != uTotalWidth)
				{
					for (auto& column : m_lstColumn)
					{
						column.uWidth = uTotalWidth * column.uWidth / m_uFixWidth;
					}

					m_uFixWidth = uTotalWidth;
				}
			}
		}

		return m_lstColumn;
	}

	operator const TD_ListColumn&()
	{
		return getListColumn(m_uTotalWidth);
	}
};

class __CommonExt CObjectList : public CTouchWnd<CListCtrl>
{
public:
	using CB_LVCostomDraw = function<void(NMLVCUSTOMDRAW& lvcd, bool& bSkipDefault)>;
	using CB_ListViewChanged = function<void(E_ListViewType)>;

	using CB_LButtonHover = function<void(const CPoint&)>;

	struct tagListPara
	{
		COLORREF crText = 0;

		UINT uFontSize = 0;
		UINT uFontSizeHeader = 0;

		E_ListViewType eViewType = (E_ListViewType)-1;

		TD_ListColumn lstColumns;
		set<UINT> setUnderlineColumns;

		UINT uHeaderHeight = 0;
		UINT uHeaderFontSize = 0;

		UINT uItemHeight = 0;

		UINT uTileWidth = 0;
		UINT uTileHeight = 0;

		CB_LVCostomDraw cbCustomDraw;
		CB_ListViewChanged cbViewChanged;
		CB_TrackMouseEvent cbMouseEvent;
	};

	CObjectList(){}

	virtual ~CObjectList()
	{
		(void)m_Imglst.DeleteImageList();
		(void)m_ImglstSmall.DeleteImageList();		
	}

	CImglst m_Imglst;
	CImglst m_ImglstSmall;

	bool m_bDblClick = false;

private:
	tagListPara m_para;

	CFontGuard m_fontGuard;

	CListHeader m_wndHeader;

	UINT m_uColumnCount = 1;

	CCompatableFont m_fontUnderline;
	
	bool m_bCusomDrawNotify = false;

	bool m_bAutoChange = false;
	
	int m_iTrackMouseFlag = -1;
	
	CString m_cstrRenameText;

public:
	BOOL InitCtrl(const tagListPara& para);

	BOOL InitFont(COLORREF crText, int iFontSizeOffset = 0);

	BOOL InitImglst(const CSize& size, const CSize *pszSmall = NULL, const TD_IconVec& vecIcons = {});
	BOOL InitImglst(CBitmap& Bitmap, CBitmap *pBitmapSmall=NULL);
	void SetImageList(CImglst *pImglst, CImglst *pImglstSmall = NULL);

	void SetView(E_ListViewType eViewType, bool bArrange = false);
	E_ListViewType GetView();

	void InitColumn(const TD_ListColumn& lstColumns, const set<UINT>& setUnderlineColumns = {});

	BOOL InitHeader(UINT uHeaderHeight, UINT uHeaderFontSize = 0);

	BOOL SetItemHeight(UINT uItemHeight);

	void SetTileSize(ULONG cx, ULONG cy);
	
	void SetCusomDrawNotify(const CB_LVCostomDraw& cbCustomDraw = NULL)
	{
		m_bCusomDrawNotify = true;
		m_para.cbCustomDraw = cbCustomDraw;
	}

	void SetViewAutoChange(const CB_ListViewChanged& cb = NULL)
	{
		m_bAutoChange = true;
		m_para.cbViewChanged = cb;
	}

	void SetTrackMouse(const CB_TrackMouseEvent& cbMouseEvent=NULL);

private:
	template <bool _clear_other>
	void _SetItemTexts(UINT uItem, const vector<wstring>& vecText, const wstring& strPrefix = L"");

	virtual bool GetRenameText(UINT uItem, wstring& strRenameText)
	{
		return true;
	}

	virtual void OnListItemRename(UINT uItem, const CString& cstrNewText) {};

public:
	int InsertItemEx(UINT uItem, const vector<wstring>& vecText, const wstring& strPrefix = L"");
	int InsertItemEx(UINT uItem, const list<pair<UINT, wstring>>& lstText, const wstring& strPrefix = L"");

	void SetItemTexts(UINT uItem, const vector<wstring>& vecText, const wstring& strPrefix = L"");
	void SetItemTexts(UINT uItem, const list<pair<UINT, wstring>>& lstText, const wstring& strPrefix = L"");
	void SetItemTexts(UINT uItem, UINT uSubItem, const vector<wstring>& vecText, const wstring& strPrefix);

	void SetTexts(const vector<vector<wstring>>& vecTexts, int nPos = 0, const wstring& strPrefix=L"");

	void SetObjects(const TD_ListObjectList& lstObjects, int nPos=0, const wstring& strPrefix=L"");

	int InsertObject(CListObject& Object, int nItem=-1, const wstring& strPrefix=L"");

	void UpdateObject(CListObject& Object);

	void UpdateItem(UINT uItem);
	void UpdateItem(UINT uItem, CListObject& Object, const list<UINT>& lstColumn);

	void UpdateItems();
	void UpdateColumns(const list<UINT>& lstColumn);

	void SetColumnText(UINT uColumn, const wstring& strText);

	BOOL DeleteObject(const CListObject *pObject);
	void DeleteObjects(const set<CListObject*>& setDeleteObjects);
	void DeleteItems(list<UINT> lstItems);

	void SetItemObject(UINT uItem, CListObject& Object, const wstring& strPrefix=L"");

	void SetItemImage(UINT uItem, int iImage);

public:
	CListObject *GetItemObject(int iItem);
	int GetObjectItem(const CListObject *pObject);
	void GetAllObjects(TD_ListObjectList& lstListObjects);

	int GetSelItem();
	CListObject *GetSelObject();

	void GetSelItems(list<UINT>& lstItems);
	void GetSelObjects(map<UINT, CListObject*>& mapObjects);
	void GetSelObjects(TD_ListObjectList& lstObjects);

	BOOL SelectFirstItem();
	void SelectItem(UINT uItem, BOOL bSetFocus=TRUE);
	void SelectObject(const CListObject *pObject, BOOL bSetFocus=TRUE);
	void SelectItems(UINT uItem, UINT uSelectCount);
	void SelectAllItems();
	void DeselectAllItems();

	UINT GetHeaderHeight();

protected:
	virtual void GenListItem(CListObject& Object, vector<wstring>& vecText, int& iImage);

	virtual void PreSubclassWindow() override;

	virtual BOOL OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult) override;

	virtual void OnTrackMouseEvent(E_TrackMouseEvent eMouseEvent, const CPoint& point);

	virtual BOOL OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult) override;

	virtual BOOL handleNMNotify(NMHDR& NMHDR, LRESULT* pResult);

	void ChangeListCtrlView(short zDelta=0);

	virtual void OnCustomDraw(NMLVCUSTOMDRAW& lvcd, bool& bSkipDefault);
};
