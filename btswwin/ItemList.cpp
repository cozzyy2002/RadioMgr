#include "pch.h"

#include "ItemList.h"
#include "../Common/Assert.h"

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
    int nImage = -1;
    for(size_t i = 0; i < m_bitmapCount; i++) {
        if(m_bitmaps[i] == imageId) {
            nImage = int(i);
            break;
        }
    }
    if(SUCCEEDED(HR_EXPECT(0 <= nImage, HRESULT_FROM_WIN32(ERROR_NOT_FOUND)))) {
        SetItem(nItem, 0, LVIF_IMAGE, nullptr, nImage, 0, 0, 0);
    }
}

void CItemList::setOverlayImage(UINT imageId, int nOverlay)
{
}
