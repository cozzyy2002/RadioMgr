#include "pch.h"

#include "ItemList.h"
#include "../Common/Assert.h"

#include <memory>

BEGIN_MESSAGE_MAP(CItemList, CListCtrl)
    ON_WM_CONTEXTMENU()
END_MESSAGE_MAP()

int CItemList::addItem(LPCTSTR key)
{
    auto nItem = GetItemCount();
    return InsertItem(nItem, key);
}

void CItemList::removeItem(LPCTSTR key)
{
    auto nItem = findItem(key);
    if(SUCCEEDED(HR_EXPECT(-1 < nItem, HRESULT_FROM_WIN32(ERROR_NOT_FOUND)))) {
        DeleteItem(nItem);
    }
}

void CItemList::selectItem(int nItem, BOOL select /*= TRUE*/)
{
    SetItemState(nItem, select ? LVIS_SELECTED : 0, LVIS_SELECTED);
}

int CItemList::findItem(LPCTSTR key)
{
    LVFINDINFO fi = {LVFI_STRING, key};
    return FindItem(&fi);
}

void CItemList::setItemImage(int nItem, UINT imageId, int nOverlay /*= 0*/)
{
    auto nImage = findImage(imageId);
    if(SUCCEEDED(HR_EXPECT(0 <= nImage, HRESULT_FROM_WIN32(ERROR_NOT_FOUND)))) {
        LVITEM lvItem{0};
        lvItem.mask = LVIF_IMAGE | LVIF_STATE;
        lvItem.iItem = nItem;
        lvItem.iImage = nImage;
        lvItem.state = INDEXTOOVERLAYMASK(nOverlay);
        lvItem.stateMask = LVIS_OVERLAYMASK;
        SetItem(&lvItem);
    }
}

void CItemList::setOverlayImage(UINT imageId, int nOverlay)
{
    auto nImage = findImage(imageId);
    if(SUCCEEDED(HR_EXPECT(0 <= nImage, HRESULT_FROM_WIN32(ERROR_NOT_FOUND)))) {
        WIN32_EXPECT(m_imageList.SetOverlayImage(nImage, nOverlay));
    }
}

int CItemList::findImage(UINT imageId)
{
    int nImage = -1;
    for(size_t i = 0; i < m_bitmapCount; i++) {
        if(m_bitmaps[i] == imageId) {
            nImage = int(i);
            break;
        }
    }
    return nImage;
}

void CItemList::OnContextMenu(CWnd* pWnd, CPoint point)
{
    UINT menuId = getContextMenuId();
    if(!menuId) { return; }

    CMenu menu;
    if(menu.LoadMenu(menuId)) {
        auto pSubMenu = menu.GetSubMenu(0);
        if(SUCCEEDED(HR_EXPECT(pSubMenu, E_UNEXPECTED))) {
            WIN32_EXPECT(pSubMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, GetParent()));
        }
    }
}

// Copy all column text and item text to clipboard.
HRESULT CItemList::Copy()
{
    // setupColumns() method should have been called
    // to know how many columns in this list.
    HR_ASSERT(0 < m_columnCount, E_ILLEGAL_METHOD_CALL);

    static const LPCTSTR ItemSeparator = _T("\t");
    static const LPCTSTR LineSeparator = _T("\n");

    CStringArray array;
    TCHAR text[100];

    // Get column text.
    LVCOLUMN column{0};
    column.mask = LVCF_TEXT;
    column.pszText = text;
    column.cchTextMax = ARRAYSIZE(text);
    CStringArray columnArray;
    for(int nCol = 0; nCol < m_columnCount; nCol++) {
        GetColumn(nCol, &column);
        columnArray.Add(text);
    }
    array.Add(join(columnArray, ItemSeparator));

    // Get item text.
    for(int nItem = 0; nItem < GetItemCount(); nItem++) {
        CStringArray itemArray;
        for(int nSubItem = 0; nSubItem < m_columnCount; nSubItem++) {
            GetItemText(nItem, nSubItem, text, ARRAYSIZE(text));
            itemArray.Add(text);
        }
        array.Add(join(itemArray, ItemSeparator));
    }
    array.Add(_T("\n"));

    // Copy column and item text to the clipboard.
    HR_ASSERT(OpenClipboard(), E_UNEXPECTED);
    WIN32_ASSERT(EmptyClipboard());

    auto str = join(array, LineSeparator);
    auto size = str.GetLength() * sizeof(TCHAR);
    auto hMem = GlobalAlloc(GMEM_MOVEABLE, size);
    if(SUCCEEDED(WIN32_EXPECT(hMem != NULL))) {
        memcpy_s(GlobalLock(hMem), size, str.LockBuffer(), size);
        WIN32_EXPECT(GlobalUnlock(hMem));
        str.UnlockBuffer();

        UINT format = (sizeof(TCHAR) == sizeof(WCHAR) ? CF_UNICODETEXT : CF_TEXT);
        WIN32_EXPECT(::SetClipboardData(format, hMem) == hMem);
    }

    WIN32_ASSERT(CloseClipboard());

    return S_OK;
}

CString join(const CStringArray& array, LPCTSTR separator /*= _T(",")*/)
{
    auto arrayCount = array.GetCount();
    switch(arrayCount) {
    case 0:
        return _T("");
    case 1:
        return array[0];
    }

    auto separatorLength = _tcslen(separator);
    size_t allCharLength = (separatorLength * arrayCount);
    for(int i = 0; i < arrayCount; i++) {
        allCharLength += array[i].GetLength();
    }

    auto joined = std::make_unique<TCHAR[]>(allCharLength + 1);
    auto joinedPtr = joined.get();
    for(int i = 0; i < arrayCount; i++) {
        if(0 < i) {
            CopyMemory(joinedPtr, separator, separatorLength * sizeof(TCHAR));
            joinedPtr += separatorLength;
        }

        auto& str = array[i];
        CopyMemory(joinedPtr, str.GetString(), str.GetLength() * sizeof(TCHAR));
        joinedPtr += str.GetLength();
    }
    *joinedPtr = _T('\0');

    return joined.get();
}
