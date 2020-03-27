
#include "stdafx.h"

#include "Common/DragDrop.h"

#define __InnerClipboardFormat ((UINT)-1)

static LPVOID g_pCurrDragData = NULL;
static map<CWnd*, IDropTargetEx*> g_mapDropTargets;

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

	g_pCurrDragData = pDragData;
	
	DROPEFFECT dwResult = COleDataSource().DoDragDrop();

	g_pCurrDragData = NULL;

	//(void)::GlobalFree(hGlobal);

	return TRUE;
}

static list<CDragDropMgr> g_lstDragDropMgr;

BOOL CDragDropMgr::RegDropTarget(IDropTargetEx& DropTarget, CWnd& Wnd)
{
	__AssertReturn(Wnd.GetSafeHwnd(), FALSE);

	if (g_mapDropTargets.find(&Wnd) != g_mapDropTargets.end())
	{
		return TRUE;
	}

	g_lstDragDropMgr.emplace_back();
	__AssertReturn(g_lstDragDropMgr.back().Register(&Wnd), FALSE);

	g_mapDropTargets[&Wnd] = &DropTarget;

	return TRUE;
}

DROPEFFECT CDragDropMgr::OnDragEnter(CWnd *pWnd, COleDataObject *pDataObject, DWORD dwKeyState, CPoint point)
{
	__AssertReturn(g_mapDropTargets[pWnd], DROPEFFECT_NONE);
		
	return g_mapDropTargets[pWnd]->OnDragOver(pWnd, g_pCurrDragData, dwKeyState, point, TRUE);
}

DROPEFFECT CDragDropMgr::OnDragOver(CWnd *pWnd, COleDataObject *pDataObject, DWORD dwKeyState, CPoint point)
{
	__AssertReturn(g_mapDropTargets[pWnd], DROPEFFECT_NONE);
	
	return g_mapDropTargets[pWnd]->OnDragOver(pWnd, g_pCurrDragData, dwKeyState, point);
}

BOOL CDragDropMgr::OnDrop(CWnd *pWnd, COleDataObject *pDataObject, DROPEFFECT dropEffect, CPoint point)
{
	__AssertReturn(g_mapDropTargets[pWnd], FALSE);

	BOOL bResult = g_mapDropTargets[pWnd]->OnDrop(pWnd, g_pCurrDragData, dropEffect, point);

	return bResult;
}

void CDragDropMgr::OnDragLeave(CWnd *pWnd)
{
	__Assert(g_mapDropTargets[pWnd]);

	g_mapDropTargets[pWnd]->OnDragLeave(pWnd, g_pCurrDragData);
}
