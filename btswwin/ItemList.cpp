#include "pch.h"

#include "ItemList.h"
#include "../Common/Assert.h"

#include <memory>

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
    HRESULT hr;
    auto nImage = findImage(imageId);
    if(SUCCEEDED(HR_EXPECT(0 <= nImage, HRESULT_FROM_WIN32(ERROR_NOT_FOUND)))) {
        hr = WIN32_EXPECT(m_imageList.SetOverlayImage(nImage, nOverlay));
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

CString join(const CStringArray& array, LPCTSTR separator /*= _T(",")*/)
{
    switch(array.GetCount()) {
    case 0:
        return _T("");
    case 1:
        return array[0];
    }

    auto arrayCount = array.GetCount();
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
