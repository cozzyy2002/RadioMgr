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
    COLUMN_TITLE_ITEM("Name", 12),
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

HRESULT CRadioInstanceList::Add(const RadioInstanceData& data)
{
    auto& pair = m_datas.insert({data.id, data});

    auto nItem = GetItemCount();
    InsertItem(nItem, data.id);
    SetItemText(nItem, Column_name, data.name);
    Update(data);
    SetCheck(nItem);

    return S_OK;
}

HRESULT CRadioInstanceList::Remove(const CString& radioInstanceId)
{
    auto nItem = Find(radioInstanceId);
    HR_ASSERT(-1 < nItem, HRESULT_FROM_WIN32(ERROR_NOT_FOUND));
    DeleteItem(nItem);

    HR_ASSERT(0 < m_datas.erase(radioInstanceId), HRESULT_FROM_WIN32(ERROR_NOT_FOUND));
    return S_OK;
}

HRESULT CRadioInstanceList::StateChange(const CString& radioInstanceId, DEVICE_RADIO_STATE radioState)
{
    // Update internal data.
    auto& data = m_datas[radioInstanceId];
    data.state = radioState;

    // Update state of the ListCtrl item.
    HR_ASSERT_OK(Update(data));

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

// Updates ListCtrl item.
HRESULT CRadioInstanceList::Update(const RadioInstanceData& data)
{
    auto nItem = Find(data.id);
    HR_ASSERT(-1 < nItem, HRESULT_FROM_WIN32(ERROR_NOT_FOUND));

    SetItem(nItem, Column_id, LVIF_IMAGE, nullptr, stateToImageIndex(data.state), 0, 0, 0);
    SetItemText(nItem, Column_state, stateToString(data.state));
    SetItemText(nItem, Column_isMultiComm, boolToString(data.isMultiComm));
    SetItemText(nItem, Column_isAssociatingDevice, boolToString(data.isAssociatingDevice));

    return S_OK;
}

// Finds ListCtrl item with id as item text, and returns index of found item.
// Returns -1 if the item is not found.
int CRadioInstanceList::Find(const CString& id)
{
    LVFINDINFO fi = {LVFI_STRING, id.GetString()};
    return FindItem(&fi);
}
