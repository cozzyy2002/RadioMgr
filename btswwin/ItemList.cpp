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
