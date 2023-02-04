#include "pch.h"
#include "RadioInstanceList.h"

static LPCTSTR columns[] = {
    _T("Signature/ID"),
    _T("State"),
};

HRESULT CRadioInstanceList::OnInitCtrl()
{
    auto exStyle = LVS_EX_CHECKBOXES | LVS_EX_AUTOSIZECOLUMNS;
    SetExtendedStyle(exStyle | GetExtendedStyle());

    int nCol = 0;
    for(auto c : columns) {
        InsertColumn(nCol++, c, LVCFMT_LEFT, 100);
    }

    return S_OK;
}

HRESULT CRadioInstanceList::Add(IRadioInstance* radioInstance)
{
    // Retrieve FriendlyName, Signature and RadioState from IRadioInstance object.
    BSTR friendlyName;
    radioInstance->GetFriendlyName(1033, &friendlyName);
    BSTR id;
    radioInstance->GetInstanceSignature(&id);
    CString _friendlyName(friendlyName);
    CString _id = id;
    CString name;
    name.Format(_T("%s:%s"), _friendlyName.GetString(), _id.GetString());
    DEVICE_RADIO_STATE state;
    radioInstance->GetRadioState(&state);

    auto nItem = GetItemCount();
    InsertItem(nItem, _id.GetString());
    SetItem(nItem, 1, LVIF_TEXT, (state == DRS_RADIO_ON) ? _T("ON") : _T("OFF"), 0, 0, 0, 0);
    SetCheck(nItem);
    
    return S_OK;
}

HRESULT CRadioInstanceList::Remove(BSTR bstrRadioInstanceId)
{
    return E_NOTIMPL;
}

HRESULT CRadioInstanceList::StateChange(BSTR bstrRadioInstanceId, DEVICE_RADIO_STATE radioState)
{
    return E_NOTIMPL;
}
