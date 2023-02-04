#pragma once

#include <afxcmn.h>
#include <RadioMgr.h>
#include <atlbase.h>
#include <map>

struct RadioInstanceData
{
	CComPtr<IRadioInstance> radioInstance;
	CString id;
	CString name;
	DEVICE_RADIO_STATE state;
};

class CRadioInstanceList : public CListCtrl
{
public:
	HRESULT OnInitCtrl();
	HRESULT Add(IRadioInstance*);
	HRESULT Remove(BSTR);
	HRESULT StateChange(BSTR, DEVICE_RADIO_STATE);

protected:
	std::map<CString, RadioInstanceData> datas;
};
