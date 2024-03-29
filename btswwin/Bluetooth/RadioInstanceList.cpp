#include "pch.h"
#include "RadioInstanceList.h"
#include "../Common/Assert.h"
#include "resource.h"

#define COLUMN_TITLE_ITEM(c, s, l) {int(CRadioInstanceList::Column::##c), _T(s), l * 8}
static const CRadioInstanceList::ColumnTitle columns[] = {
    COLUMN_TITLE_ITEM(Id, "Signature/ID", 20),
    COLUMN_TITLE_ITEM(Name, "FriendlyName", 12),
    COLUMN_TITLE_ITEM(State, "State", 8),
    COLUMN_TITLE_ITEM(IsMultiComm, "IsMultiComm", 10),
    COLUMN_TITLE_ITEM(IsAssocDev, "IsAssocDev", 10),
};

HRESULT CRadioInstanceList::OnInitCtrl()
{
    auto exStyle = LVS_EX_CHECKBOXES | LVS_EX_AUTOSIZECOLUMNS | LVS_EX_FULLROWSELECT;
    SetExtendedStyle(exStyle | GetExtendedStyle());

    // Setup colums
    setupColumns(columns);

    // Setup image list for Bluetooth on/off icon.
    static const UINT bitmaps[] = {IDB_BITMAP_RADIO_ON, IDB_BITMAP_RADIO_OFF};
    setupImageList(bitmaps);

    return S_OK;
}

static LPCTSTR stateToString(DEVICE_RADIO_STATE state) { return ((state == DRS_RADIO_ON) ? _T("ON") : _T("OFF")); }

HRESULT CRadioInstanceList::Add(const RadioInstanceData& data, BOOL isChecked)
{
    auto& pair = m_datas.insert({data.id, data});

    auto nItem = addItem(data.id);
    SetItemData(nItem, (DWORD_PTR)&pair.first->second);
    Update(data, UpdateMask::All);
    SetCheck(nItem, isChecked);

    return S_OK;
}

HRESULT CRadioInstanceList::Remove(const CString& radioInstanceId)
{
    removeItem(radioInstanceId);

    HR_ASSERT(0 < m_datas.erase(radioInstanceId), HRESULT_FROM_WIN32(ERROR_NOT_FOUND));
    return S_OK;
}

HRESULT CRadioInstanceList::StateChange(const CString& radioInstanceId, DEVICE_RADIO_STATE radioState)
{
    // Update internal data.
    auto& data = m_datas[radioInstanceId];
    data.state = radioState;

    // Update state of the ListCtrl item.
    HR_ASSERT_OK(Update(data, UpdateMask::State));

    return S_OK;
}

HRESULT CRadioInstanceList::For(std::function<HRESULT(RadioInstanceData&)> func, bool onlyChecked /*= true*/)
{
    auto hr = S_FALSE;
    auto cItems = GetItemCount();
    for(auto i = 0; i < cItems; i++) {
        if(!onlyChecked || GetCheck(i)) {
            auto id(GetItemText(i, 0));
            auto& data = m_datas[id];
            if(FAILED(hr = func(data))) break;
        }
    }

    return hr;
}

HRESULT CRadioInstanceList::For(std::function<HRESULT(int, BOOL)> func)
{
    auto hr = S_FALSE;
    auto cItems = GetItemCount();
    for(auto nItem = 0; nItem < cItems; nItem++) {
        if(FAILED(hr = func(nItem, GetCheck(nItem)))) break;
    }

    return hr;
}

RadioInstanceData* CRadioInstanceList::GetSelectedInstance()
{
    RadioInstanceData* ret = nullptr;
    auto pos = GetFirstSelectedItemPosition();
    if(pos) {
        auto i = GetNextSelectedItem(pos);
        ret = (RadioInstanceData*)GetItemData(i);
    }
    return ret;
}

RadioInstanceData* CRadioInstanceList::GetInstance(const CString& radioInstanceId)
{
    auto it = m_datas.find(radioInstanceId);
    return (it != m_datas.end()) ? &(it->second) : nullptr;
}

// Updates ListCtrl item.
HRESULT CRadioInstanceList::Update(const RadioInstanceData& data, UpdateMask mask)
{
    auto nItem = Find(data.id);
    HR_ASSERT(-1 < nItem, HRESULT_FROM_WIN32(ERROR_NOT_FOUND));

    // NOTE: ID is set by InsertItem() and is never changed, so updating ID is not neccessary.
    if(mask & UpdateMask::Name) { SetItemText(nItem, int(Column::Name), data.name); }
    if(mask & UpdateMask::StateIcon) { setItemImage(nItem, (data.state == DRS_RADIO_ON) ? IDB_BITMAP_RADIO_ON : IDB_BITMAP_RADIO_OFF); }
    if(mask & UpdateMask::StateText) { SetItemText(nItem, int(Column::State), stateToString(data.state)); }
    if(mask & UpdateMask::IsMultiComm) { SetItemText(nItem, int(Column::IsMultiComm), boolToString(data.isMultiComm)); }
    if(mask & UpdateMask::IsAssocDev) { SetItemText(nItem, int(Column::IsAssocDev), boolToString(data.isAssociatingDevice)); }

    return S_OK;
}

// Finds ListCtrl item with id as item text, and returns index of found item.
// Returns -1 if the item is not found.
int CRadioInstanceList::Find(const CString& id)
{
    return CItemList::findItem(id.GetString());
}

UINT CRadioInstanceList::getContextMenuId() const
{
    return IDR_MENU_RADIO_LIST;
}
