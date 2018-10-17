
#include "stdafx.h"

#include <DragDrop.h>

#define __InnerClipboardFormat ((UINT)-1)

static LPVOID m_pCurrDragData = NULL;
static map<CWnd*, IDropTargetEx*> m_mapDropTargets;

BOOL CDragDropMgr::DoDrag(LPVOID pDragData)
{
	/*
	HGLOBAL hGlobal = ::GlobalAlloc(GHND|GMEM_SHARE, sizeof(LPVOID));
	LPVOID *ppData = (LPVOID*)::GlobalLock(hGlobal);
	*ppData = pData;
	__EnsureReturn(::GlobalUnlock(hGlobal), FALSE);

	COleDataSource DataSource;
	DataSource.CacheGlobalData(__InnerClipboardFormat, hGlobal);
	*/

	m_pCurrDragData = pDragData;
	
	DROPEFFECT dwResult = COleDataSource().DoDragDrop();

	m_pCurrDragData = NULL;

	//(void)::GlobalFree(hGlobal);

	return TRUE;
}

static list<CDragDropMgr> g_lstDragDropMgr;

BOOL CDragDropMgr::RegDropTarget(IDropTargetEx& DropTarget, CWnd& Wnd)
{
	__AssertReturn(Wnd.GetSafeHwnd(), FALSE);

	if (m_mapDropTargets.find(&Wnd) != m_mapDropTargets.end())
	{
		return TRUE;
	}

	g_lstDragDropMgr.push_back(CDragDropMgr());
	__AssertReturn(g_lstDragDropMgr.back().Register(&Wnd), FALSE);

	m_mapDropTargets[&Wnd] = &DropTarget;

	return TRUE;
}

DROPEFFECT CDragDropMgr::OnDragEnter(CWnd *pWnd, COleDataObject *pDataObject, DWORD dwKeyState, CPoint point)
{
	__AssertReturn(m_mapDropTargets[pWnd], DROPEFFECT_NONE);
		
	return m_mapDropTargets[pWnd]->OnDragOver(pWnd, m_pCurrDragData, dwKeyState, point, TRUE);
}

DROPEFFECT CDragDropMgr::OnDragOver(CWnd *pWnd, COleDataObject *pDataObject, DWORD dwKeyState, CPoint point)
{
	__AssertReturn(m_mapDropTargets[pWnd], DROPEFFECT_NONE);
	
	return m_mapDropTargets[pWnd]->OnDragOver(pWnd, m_pCurrDragData, dwKeyState, point);
}

BOOL CDragDropMgr::OnDrop(CWnd *pWnd, COleDataObject *pDataObject, DROPEFFECT dropEffect, CPoint point)
{
	__AssertReturn(m_mapDropTargets[pWnd], FALSE);

	BOOL bResult = m_mapDropTargets[pWnd]->OnDrop(pWnd, m_pCurrDragData, dropEffect, point);

	return bResult;
}

void CDragDropMgr::OnDragLeave(CWnd *pWnd)
{
	__Assert(m_mapDropTargets[pWnd]);

	m_mapDropTargets[pWnd]->OnDragLeave(pWnd, m_pCurrDragData);
}
