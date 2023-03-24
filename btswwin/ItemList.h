#pragma once

#include <afxcmn.h>

// Base class for the class that shows one or more items in CListCtrl.
class CItemList : public CListCtrl
{
public:
    struct ColumnTitle
    {
        int index;
        LPCTSTR title;
        int pixelWidth;
    };

    template<size_t size>
    HRESULT setupColumns(const ColumnTitle (&columns)[size]);
    template<size_t size>
    HRESULT setupImageList(const UINT (&bitmaps)[size]);

protected:
    CItemList() : m_columnCount(0), m_bitmaps(nullptr), m_bitmapCount(0) {}

    size_t m_columnCount;
    CImageList m_imageList;
    const UINT* m_bitmaps;
    size_t m_bitmapCount;

    int addItem(LPCTSTR);
    void removeItem(LPCTSTR);
    int findItem(LPCTSTR);

    void setItemImage(int nItem, UINT imageId, int nOverlay = 0);
    void setOverlayImage(UINT imageId, int nOverlay);
    int findImage(UINT imageId);

    virtual UINT getContextMenuId() const { return 0; }
    DECLARE_MESSAGE_MAP()

public:
    void OnContextMenu(CWnd* pWnd, CPoint point);
    HRESULT Copy();
};

template<size_t size>
HRESULT CItemList::setupColumns(const ColumnTitle(&columns)[size])
{
    m_columnCount = size;

    for(auto& c : columns) {
        InsertColumn(c.index, c.title, LVCFMT_LEFT, c.pixelWidth);
    }
    return S_OK;
}

template<size_t size>
HRESULT CItemList::setupImageList(const UINT(&bitmaps)[size])
{
    m_bitmaps = bitmaps;
    m_bitmapCount = size;

    m_imageList.Create(16, 16, ILC_COLOR, size, size);
    for(auto b : bitmaps) {
        CBitmap bm;
        bm.LoadBitmap(b);
        m_imageList.Add(&bm, RGB(0, 0, 0));
    }
    //m_imageList.SetOverlayImage(1, 1);
    //SetImageList(&m_imageList, LVSIL_STATE);
    SetImageList(&m_imageList, LVSIL_SMALL);

    return S_OK;
}

inline LPCTSTR boolToString(BOOL b) { return ((b) ? _T("Yes") : _T("No")); }

CString join(const CStringArray& array, LPCTSTR separator = _T(","));
