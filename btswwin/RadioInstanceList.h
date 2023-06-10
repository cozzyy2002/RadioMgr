#pragma once

#include "ItemList.h"

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
	BOOL isMultiComm{};
	BOOL isAssociatingDevice{};
	DEVICE_RADIO_STATE state{};			// Updated by StateChange() method.
	DEVICE_RADIO_STATE savedState{};	// Used by owner to save state at some point.
};

class CRadioInstanceList : public CItemList
{
public:
	// Column index(Sub item index)
	enum class Column {
		Id = 0,		// ID is used as item text to find ListView item. So this must be 0.
		Name,
		State,
		IsMultiComm,
		IsAssocDev,
	};

	HRESULT OnInitCtrl();
	HRESULT Add(const RadioInstanceData&, BOOL);
	HRESULT Remove(const CString&);
	HRESULT StateChange(const CString&, DEVICE_RADIO_STATE);

	HRESULT For(std::function<HRESULT(RadioInstanceData&)> data, bool onlyChecked = true);
	RadioInstanceData* GetSelectedInstance();
	RadioInstanceData* GetInstance(const CString&);

	enum class UpdateMask {
		None = 0,
		Id = 1,
		Name = 2,
		StateIcon = 4,
		StateText = 8,
		State = StateIcon | StateText,
		IsMultiComm = 0x10,
		IsAssocDev = 0x20,
		All = 0xff
	};
	HRESULT Update(const RadioInstanceData& data, UpdateMask mask);

protected:
	std::map<CString, RadioInstanceData> m_datas;

	int Find(const CString& id);

	virtual UINT getContextMenuId() const override;
};

#pragma region Operator functions for CRadioInstanceList::UpdateMask to be used as flag.
inline bool operator&(CRadioInstanceList::UpdateMask a, CRadioInstanceList::UpdateMask b)
{
	return bool(int(a) & int (b));
}

inline CRadioInstanceList::UpdateMask operator|(CRadioInstanceList::UpdateMask a, CRadioInstanceList::UpdateMask b)
{
	return CRadioInstanceList::UpdateMask(int(a) | int(b));
}

inline CRadioInstanceList::UpdateMask& operator|=(CRadioInstanceList::UpdateMask& _this, CRadioInstanceList::UpdateMask _that)
{
	_this = _this | _that;
	return _this;
}
#pragma endregion
