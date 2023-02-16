#pragma once

#include <afxcmn.h>
#include <RadioMgr.h>
#include <atlbase.h>
#include <map>
#include <functional>

// Structure contains value of IRadioInstance.
// Initialized by Add() method.
struct RadioInstanceData
{
	const CComPtr<IRadioInstance> radioInstance;
	const CString id;
	const CString name;
	const BOOL isMultiComm{};
	const BOOL isAssociatingDevice{};
	DEVICE_RADIO_STATE state;		// Updated by StateChange() method.
	DEVICE_RADIO_STATE savedState;	// Used by owner to save state at some point.
};

class CRadioInstanceList : public CListCtrl
{
public:
	// Column index(Sub item index)
	enum {
		Column_id = 0,		// ID is used as item text to find ListView item. So this must be 0.
		Column_state,
		Column_isMultiComm,
		Column_isAssociatingDevice,
	};

	HRESULT OnInitCtrl();
	HRESULT Add(IRadioInstance*, RadioInstanceData** = nullptr);
	HRESULT Remove(const CString&);
	HRESULT StateChange(const CString&, DEVICE_RADIO_STATE);

	HRESULT For(std::function<HRESULT(RadioInstanceData&)> data, bool onlyChecked = true);

protected:
	std::map<CString, RadioInstanceData> m_datas;
	CImageList m_imageList;

	int Find(const CString& id);
};
