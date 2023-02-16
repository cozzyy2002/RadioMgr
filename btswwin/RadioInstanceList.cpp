#include "pch.h"
#include "RadioInstanceList.h"
#include "../Common/Assert.h"
#include "resource.h"

struct ColumnTitle
{
    LPCTSTR title;
    int pixelWidth;
};

#define COLUMN_TITLE_ITEM(s, l) {_T(s), l * 8}
static ColumnTitle columns[] = {
    COLUMN_TITLE_ITEM("Signature/ID", 20),
    COLUMN_TITLE_ITEM("State", 8),
    COLUMN_TITLE_ITEM("IsMultiComm", 10),
    COLUMN_TITLE_ITEM("IsAssocDev", 10),
};

HRESULT CRadioInstanceList::OnInitCtrl()
{
    auto exStyle = LVS_EX_CHECKBOXES | LVS_EX_AUTOSIZECOLUMNS | LVS_EX_FULLROWSELECT;
    SetExtendedStyle(exStyle | GetExtendedStyle());

    // Setup colums
    int nCol = 0;
    for(auto& c : columns) {
        InsertColumn(nCol++, c.title, LVCFMT_LEFT, c.pixelWidth);
    }

    // Setup image list for Bluetooth on/off icon.
    static const UINT bitmaps[] = {IDB_BITMAP_RADIO_ON, IDB_BITMAP_RADIO_OFF};
    m_imageList.Create(16, 16, ILC_COLOR, ARRAYSIZE(bitmaps), 2);
    for(auto b : bitmaps) {
        CBitmap bm;
        bm.LoadBitmap(b);
        m_imageList.Add(&bm, RGB(0, 0, 0));
    }
    SetImageList(&m_imageList, LVSIL_SMALL);

    return S_OK;
}

static int stateToImageIndex(DEVICE_RADIO_STATE state) { return ((state == DRS_RADIO_ON) ? 0 : 1); }
static LPCTSTR stateToString(DEVICE_RADIO_STATE state) { return ((state == DRS_RADIO_ON) ? _T("ON") : _T("OFF")); }
static LPCTSTR boolToString(BOOL b) { return ((b) ? _T("Yes") : _T("No")); }

HRESULT CRadioInstanceList::Add(IRadioInstance* radioInstance, RadioInstanceData** pData /*= nullptr*/)
{
    // Retrieve FriendlyName, Signature and RadioState from IRadioInstance object.
    BSTR friendlyName;
    radioInstance->GetFriendlyName(1033, &friendlyName);
    BSTR id;
    HR_ASSERT_OK(radioInstance->GetInstanceSignature(&id));
    DEVICE_RADIO_STATE state;
    HR_ASSERT_OK(radioInstance->GetRadioState(&state));
    RadioInstanceData data(
        {
            radioInstance, id, friendlyName,
            radioInstance->IsMultiComm(),
            radioInstance->IsAssociatingDevice(),
            state, state
        });
    auto& pair = m_datas.insert({data.id, data});

    auto nItem = GetItemCount();
    InsertItem(LVIF_TEXT | LVIF_IMAGE, nItem, data.id.GetString(), 0, 0, stateToImageIndex(state), 0);
    SetItemText(nItem, Column_state, stateToString(state));
    SetItemText(nItem, Column_isMultiComm, boolToString(data.isMultiComm));
    SetItemText(nItem, Column_isAssociatingDevice, boolToString(data.isAssociatingDevice));
    SetCheck(nItem);
    
    if(pData) { *pData = &(pair.first->second); }

    return S_OK;
}

HRESULT CRadioInstanceList::Remove(const CString& radioInstanceId)
{
    auto index = Find(radioInstanceId);
    HR_ASSERT(-1 < index, HRESULT_FROM_WIN32(ERROR_NOT_FOUND));
    DeleteItem(index);

    HR_ASSERT(0 < m_datas.erase(radioInstanceId), HRESULT_FROM_WIN32(ERROR_NOT_FOUND));
    return S_OK;
}

HRESULT CRadioInstanceList::StateChange(const CString& radioInstanceId, DEVICE_RADIO_STATE radioState)
{
    // Update state of the ListCtrl item.
    auto index = Find(radioInstanceId);
    HR_ASSERT(-1 < index, HRESULT_FROM_WIN32(ERROR_NOT_FOUND));
    SetItem(index, Column_id, LVIF_IMAGE, nullptr, stateToImageIndex(radioState), 0, 0, 0);
    SetItemText(index, Column_state, stateToString(radioState));

    // Update internal data.
    auto& data = m_datas[radioInstanceId];
    data.state = radioState;

    return S_OK;
}

HRESULT CRadioInstanceList::For(std::function<HRESULT(RadioInstanceData&)> func, bool onlyChecked /*= true*/)
{
    auto cItems = GetItemCount();
    for(auto i = 0; i < cItems; i++) {
        if(!onlyChecked || GetCheck(i)) {
            auto id(GetItemText(i, 0));
            auto& data = m_datas[id];
            HR_ASSERT_OK(func(data));
        }
    }

    return (0 < cItems) ? S_OK : S_FALSE;
}

// Finds ListCtrl item with id as item text, and returns index of found item.
// Returns -1 if the item is not found.
int CRadioInstanceList::Find(const CString& id)
{
    LVFINDINFO fi = {LVFI_STRING, id.GetString()};
    return FindItem(&fi);
}
