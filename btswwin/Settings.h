#pragma once

#include "../Common/Assert.h"
#include <vector>
#include <typeinfo>

class CSettings
{
public:
	class IValue
	{
	public:
		// Reads value from profile storage.
		virtual void read(CSettings* settings) = 0;

		// Writes value to profile storage.
		// Note: Writing value is performed regardless the value has been changed or not.
		virtual void write(CSettings* settings) = 0;

		// Returns true if the value is changed and write() is not called yet.
		virtual bool isChanged() const = 0;

		// Returns string that consist of it's name and value.
		virtual CString toString() const = 0;
	};

	template<typename T>
	class Value : public IValue
	{
	public:
#pragma region Implementation of IValue interface.
		void read(CSettings* settings) override
		{
			m_value = settings->read(*this);
			m_savedValue = m_value;
		}
		void write(CSettings* settings) override
		{
			settings->write(*this);
			m_savedValue = m_value;
		}
		bool isChanged() const override { return (m_value != m_savedValue); }
		virtual CString toString() const override;	// Implemented for each type T.
#pragma endregion

		explicit Value(LPCTSTR name, const T& defaultValue) { construct(name, defaultValue); }
		explicit Value(LPCTSTR name) { construct(name, T()); }

		T& operator =(const T& newValue) { return (m_value = newValue); }
		T& operator *() { return m_value; }
		T* operator ->() { return &m_value; }
		operator T() const { return m_value; }

		LPCTSTR getName() const { return m_name.GetString(); }
		const T& getDefault() const { return m_defaultValue; }

		static const DWORD RegType;

	protected:
		void construct(LPCTSTR name, const T& defaultValue)
		{
			m_name = name;
			m_value = m_savedValue = m_defaultValue = defaultValue;
		}

		CString m_name;
		T m_value;				// Current value.
		T m_savedValue;			// Value saved in profile storage.
		T m_defaultValue;		// Value to be set to m_value if the name doesn't exist when read() is called.
	};

	template<typename T>
	class EnumValue : public Value<int>
	{
	public:
		explicit EnumValue(LPCTSTR name, const T& defaultValue) : Value(name, (int)defaultValue) {}
		explicit EnumValue(LPCTSTR name) : Value(name, (int)T()) {}

		EnumValue& operator =(const T& newValue)
		{
			m_value = (int)newValue;
			return *this;
		}
		operator T() const { return (T)m_value; }
		operator int() const { return (int)m_value; }
		T getDefault() const { return (T)m_defaultValue; }
	};

	template<typename T>
	class BinaryValue : public IValue
	{
	public:
#pragma region Implementation of IValue interface.
		void read(CSettings* settings) override;
		void write(CSettings* settings) override;
		bool isChanged() const override;
		CString toString() const override;
#pragma endregion

		class IValueHandler
		{
		public:
			virtual void copy(T& dest, const T& source) = 0;
			virtual bool isChanged(const T& a, const T& b) = 0;
			virtual CString valueToString(const BinaryValue<T>& value) const = 0;
		};

		class DefaultValueHandler : public IValueHandler
		{
		public:
			virtual void copy(T& dest, const T& source) override;
			virtual bool isChanged(const T& a, const T& b) override;
			virtual CString valueToString(const BinaryValue<T>& value) const override;
		};

		explicit BinaryValue(LPCTSTR name, IValueHandler* valueHandler = &m_defaultValueHandler)
			: m_name(name), m_value(T()), m_savedValue(T()), m_valueHandler(valueHandler) {}

		T& operator *() { return m_value; }
		T* operator ->() { return &m_value; }
		operator T&() { return m_value; }
		operator T*() { return &m_value; }
		operator const T& () const { return m_value; }
		operator const T* () const { return &m_value; }

		LPCTSTR getName() const { return m_name.GetString(); }

		static const DWORD RegType;

	protected:
		CString m_name;
		T m_value;				// Current value.
		T m_savedValue;			// Value saved in profile storage.
		IValueHandler* m_valueHandler;
		static DefaultValueHandler m_defaultValueHandler;
	};

	explicit CSettings(LPCTSTR companyName, LPCTSTR applicationName);

	template<size_t size>
	HRESULT load(IValue* (&valueList)[size]) { return load(valueList, size); }

	HRESULT load(IValue** valueList, size_t size);
	HRESULT save(bool force = false);

	bool isChanged() const;

protected:
	template<typename T> T read(Value<T>& value);
	template<typename T> void write(Value<T>& value);
	template<typename T> BYTE* read(BinaryValue<T>& value);
	template<typename T> void write(BinaryValue<T>& value);
	HRESULT read(LPCTSTR valueName, DWORD expectedType, std::unique_ptr<BYTE[]>& data, DWORD expectedSize = 0);
	HRESULT write(LPCTSTR valueName, DWORD type, const BYTE* data, DWORD size);

protected:
	HKEY m_hKey;
	std::vector<IValue*> m_valueList;
};


#pragma region Implementation for general type T using BinaryValue<T> class.

template<typename T>
void CSettings::BinaryValue<T>::read(CSettings* settings)
{
	std::unique_ptr<BYTE[]> p(settings->read(*this));
	if(p) {
		auto src = *(T*)p.get();
		m_valueHandler->copy(m_value, src);
		m_valueHandler->copy(m_savedValue, src);
	}
}

template<typename T>
void CSettings::BinaryValue<T>::write(CSettings* settings)
{
	settings->write(*this);
	m_valueHandler->copy(m_savedValue, m_value);
}

template<typename T>
bool CSettings::BinaryValue<T>::isChanged() const
{
	return m_valueHandler->isChanged(m_value, m_savedValue);
}

template<typename T>
CString CSettings::BinaryValue<T>::toString() const
{
	CString str;
	str.Format(_T("%s: %s"), m_name.GetString(), m_valueHandler->valueToString(*this).GetString());
	return str;
}

template<typename T>
void CSettings::BinaryValue<T>::DefaultValueHandler::copy(T& dest, const T& source)
{
	dest = source;
}

template<typename T>
bool CSettings::BinaryValue<T>::DefaultValueHandler::isChanged(const T& a, const T& b)
{
	// Compare as byte array.
	auto p1 = (BYTE*)&a;
	auto p2 = (BYTE*)&b;
	for(int i = 0; i < sizeof(T); i++)
	{
		if(*(p1++) != *(p2++)) return true;
	}
	return false;
}

template<typename T>
CString CSettings::BinaryValue<T>::DefaultValueHandler::valueToString(const BinaryValue<T>& value) const
{
	CString typeName(typeid(T).name());
	CString str;
	str.Format(_T("%s Size=%d byte"), typeName.GetString(), (int)sizeof(T));
	return str;
}

template<typename T>
BYTE* CSettings::read(BinaryValue<T>& value)
{
	std::unique_ptr<BYTE[]> data;
	if(SUCCEEDED(read(value.getName(), value.RegType, data, sizeof(T)))) {
		return data.release();
	} else {
		return nullptr;
	}
}

template<typename T>
void CSettings::write(BinaryValue<T>& value)
{
	write(value.getName(), value.RegType, (BYTE*)(T*)value, sizeof(T));
}

#pragma endregion

template<typename T>
int operator &(const CSettings::EnumValue<T>& a, T b) { return (int)a & (int)b; }

template<typename T>
int operator &(T a, const CSettings::EnumValue<T>& b) { return (int)a & (int)b; }

template<typename T>
int operator &(const CSettings::EnumValue<T>& a, const CSettings::EnumValue<T>& b) { return (int)a & (int)b; }
