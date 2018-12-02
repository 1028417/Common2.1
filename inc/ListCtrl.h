
#pragma once

#include <img.h>

#include <Guide.h>

class CHeader : public CHeaderCtrl
{
	DECLARE_MESSAGE_MAP()

public:
	CHeader(){}
	
private:
	UINT m_uHeight = 0;

	CFontGuide m_fontGuide;

private:
	LRESULT OnLayout(WPARAM wParam, LPARAM lParam);

public:
	BOOL Init(UINT uHeight, UINT uFontSize = 0);
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

class __CommonExt CListColumnGuide
{
public:
	CListColumnGuide(UINT uMaxWidth)
	{
		m_uMaxWidth = uMaxWidth;
	}

private:
	UINT m_uMaxWidth = 0;

	TD_ListColumn m_lstColumn;

	map<tagListColumn*, double> m_mapPercentWidth;

	UINT m_uSumWidth = 0;

public:
	CListColumnGuide& add(const CString& cstrText, UINT uWidth, UINT uFormat=LVCFMT_LEFT)
	{
		m_lstColumn.push_back({ cstrText, uWidth, uFormat });
		
		m_uSumWidth += uWidth;

		return *this;
	}

	CListColumnGuide& add(const CString& cstrText, double fPercentWidth, UINT uFormat = LVCFMT_LEFT)
	{
		m_lstColumn.push_back({ cstrText, 0, uFormat });

		m_mapPercentWidth[&m_lstColumn.back()] = fPercentWidth;
		
		return *this;
	}

	operator const TD_ListColumn&()
	{
		if (m_uSumWidth < m_uMaxWidth)
		{
			UINT uRamainWidth = m_uMaxWidth - m_uSumWidth;
			for (auto& pr : m_mapPercentWidth)
			{
				pr.first->uWidth = UINT(uRamainWidth*pr.second);
			}
		}

		return m_lstColumn;
	}
};

class __CommonExt CObjectList : public CListCtrl
{
public:
	using CB_LVCostomDraw = function<void(class CObjectList& wndList, NMLVCUSTOMDRAW& lvcd, bool& bSkipDefault)>;
	using CB_ListViewChanged = function<void(E_ListViewType)>;

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

	CFontGuide m_fontGuide;

	CHeader m_wndHeader;

	UINT m_uColumnCount = 1;

	CCompatableFont m_fontUnderline;
	
	bool m_bCusomDrawNotify = false;

	bool m_bAutoChange = false;
	
	int m_iTrackMouseFlag = -1;
	
	CString m_cstrRenameText;

public:
	BOOL InitCtrl(const tagListPara& para);

	BOOL InitFont(COLORREF crText, UINT uFontSize = 0);

	BOOL InitImglst(const CSize& size, const CSize *pszSmall = NULL, const TD_IconVec& vecIcons = {});
	BOOL InitImglst(CBitmap& Bitmap, CBitmap *pBitmapSmall=NULL);
	void SetImageList(CImglst *pImglst, CImglst *pImglstSmall = NULL);

	void SetView(E_ListViewType eViewType, bool bArrange = false);
	E_ListViewType GetView();

	BOOL InitColumn(const TD_ListColumn &lstColumns, const set<UINT>& setUnderlineColumns = {});
	BOOL SetUnderlineColumn(const set<UINT>& setUnderlineColumns);

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

public:
	int InsertItemEx(UINT uItem, vector<wstring> vecText, const wstring& strPrefix = L"");
	int InsertItemEx(UINT uItem, const list<pair<UINT, wstring>>& lstText);

	void SetItemTexts(UINT uItem, const vector<wstring>& vecText, const wstring& strPrefix = L"");
	void SetItemTexts(UINT uItem, const list<pair<UINT, wstring>>& lstText);

	void SetObjects(const TD_ListObjectList& lstObjects, int nPos=0);

	int InsertObject(CListObject& Object, int nItem=-1);

	void UpdateObject(CListObject& Object);

	void UpdateItem(UINT uItem);
	void UpdateItem(UINT uItem, CListObject& Object, const list<UINT>& lstColumn);

	void UpdateItems();
	void UpdateColumns(const list<UINT>& lstColumn);

	void SetColumnText(UINT uColumn, const wstring& strText);

	void DeleteObjects(const set<CListObject*>& setDeleteObjects);

	BOOL DeleteObject(const CListObject *pObject);

	void SetItemObject(UINT uItem, CListObject& Object);

	void SetItemImage(UINT uItem, int iImage);

public:
	CListObject *GetItemObject(int iItem);
	int GetObjectItem(const CListObject *pObject);
	void GetAllObjects(TD_ListObjectList& lstListObjects);

	int GetSingleSelectedItem();
	CListObject *GetSingleSelectedObject();

	void GetMultiSelectedItems(list<UINT>& lstItems);
	void GetMultiSelectedObjects(map<UINT, CListObject*>& mapObjects);
	void GetMultiSelectedObjects(TD_ListObjectList& lstObjects);

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
