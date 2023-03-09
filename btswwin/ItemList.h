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
    CImageList m_imageList;

    int addItem(LPCTSTR);
    void removeItem(LPCTSTR);
    int findItem(LPCTSTR);

    LPCTSTR boolToString(BOOL b) { return ((b) ? _T("Yes") : _T("No")); }
};

template<size_t size>
HRESULT CItemList::setupColumns(const ColumnTitle(&columns)[size])
{
    for(auto& c : columns) {
        InsertColumn(c.index, c.title, LVCFMT_LEFT, c.pixelWidth);
    }
    return S_OK;
}

template<size_t size>
HRESULT CItemList::setupImageList(const UINT(&bitmaps)[size])
{
    m_imageList.Create(16, 16, ILC_COLOR, size, size);
    for(auto b : bitmaps) {
        CBitmap bm;
        bm.LoadBitmap(b);
        m_imageList.Add(&bm, RGB(0, 0, 0));
    }
    SetImageList(&m_imageList, LVSIL_SMALL);

    return S_OK;
}
