#pragma once

#include "RegistryKey.h"
#include "../Common/Assert.h"
#include <vector>
#include <typeinfo>

class CSettings
{
public:
	class IValue
	{
	public:
		virtual ~IValue() {}

		// Reads value from profile storage.
		virtual void read(CSettings* settings) = 0;

		// Writes value to profile storage.
		// Note: Writing value is performed regardless the value has been changed or not.
		virtual void write(CSettings* settings) = 0;

		// Returns true if the value is changed and write() is not called yet.
		virtual bool isChanged() const = 0;

		// Returns string that consist of it's name and value.
		virtual CString toString(bool isRelativeKey = false) const = 0;

		virtual CRegKey& getHKey() = 0;
		virtual const CString& getName() const = 0;
		virtual DWORD getRegType() const = 0;
	};

protected:
	// Common base class for classes that implement IValue.
	class ValueBase : public IValue
	{
	public:
		ValueBase(CRegistryKey& registryKey, LPCTSTR name)
			: m_registryKey(registryKey), m_name(name) {}

#pragma region Implementation of IValue interface
		CString toString(bool isRelativeKey = false) const override;
		CRegKey& getHKey() override { return m_registryKey.getHKey(); }
		const CString& getName() const override { return m_name; }
#pragma endregion

	protected:
		// Returns value as string to implement toString() method.
		virtual CString valueToString() const = 0;

		CRegistryKey& m_registryKey;
		const CString m_name;
	};

public:
	template<typename T>
	class Value : public ValueBase
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
		CString valueToString() const override;	// Implemented for each type T.
		DWORD getRegType() const override { return RegType; }
#pragma endregion

		explicit Value(CRegistryKey& registryKey, LPCTSTR name, const T& defaultValue = T())
			: ValueBase(registryKey, name)
			, m_value(defaultValue), m_savedValue(defaultValue), m_defaultValue(defaultValue)
		{}

		T& operator =(const T& newValue) { return (m_value = newValue); }
		T& operator *() { return m_value; }
		T* operator ->() { return &m_value; }
		operator T() const { return m_value; }

		const T& getDefault() const { return m_defaultValue; }

	protected:
		T m_value;				// Current value.
		T m_savedValue;			// Value saved in profile storage.
		T m_defaultValue;		// Value to be set to m_value if the name doesn't exist when read() is called.
		static const DWORD RegType;
	};

	template<typename T>
	class EnumValue : public ValueBase
	{
	public:
#pragma region Implementation of IValue interface.
		void read(CSettings* settings) override { m_intValue.read(settings); }
		void write(CSettings* settings) override { m_intValue.write(settings); }
		bool isChanged() const override { return m_intValue.isChanged(); }
		CString valueToString() const override { return m_intValue.valueToString(); }
		DWORD getRegType() const override { return m_intValue.getRegType(); }
#pragma endregion

		// Note: A default value of EnumValue should be specified explicitly.
		explicit EnumValue(CRegistryKey& registryKey, LPCTSTR name, const T& defaultValue)
			: ValueBase(registryKey, name), m_intValue(registryKey, name, (int)defaultValue) {}

		EnumValue& operator =(const T& newValue)
		{
			m_intValue = (int)newValue;
			return *this;
		}
		T operator *() { return (T)(int)m_intValue; }
		operator T() const { return (T)(int)m_intValue; }
		operator int() const { return (int)m_intValue; }
		T getDefault() const { return (T)m_intValue.getDefault(); }

	protected:
		Value<int> m_intValue;
	};

	template<typename T>
	class BinaryValue : public ValueBase
	{
	public:
#pragma region Implementation of IValue interface.
		void read(CSettings* settings) override;
		void write(CSettings* settings) override;
		bool isChanged() const override;
		CString valueToString() const override { return m_valueHandler->valueToString(*this); }
		DWORD getRegType() const override { return RegType; }
#pragma endregion

		class IValueHandler
		{
		public:
			virtual ~IValueHandler() {}

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

		explicit BinaryValue(CRegistryKey& registryKey, LPCTSTR name, IValueHandler* valueHandler = &m_defaultValueHandler)
			: ValueBase(registryKey, name), m_value(T()), m_savedValue(T()), m_valueHandler(valueHandler) {}

		T& operator *() { return m_value; }
		T* operator ->() { return &m_value; }
		operator T&() { return m_value; }
		operator T*() { return &m_value; }
		operator const T& () const { return m_value; }
		operator const T* () const { return &m_value; }

	protected:
		T m_value;				// Current value.
		T m_savedValue;			// Value saved in profile storage.
		IValueHandler* m_valueHandler;
		static DefaultValueHandler m_defaultValueHandler;
		static const DWORD RegType;
	};

	template<size_t size>
	HRESULT load(IValue* (&valueList)[size]) { return load(valueList, size); }

	HRESULT load(IValue** valueList, size_t size);
	HRESULT save(bool force = false);

	bool isChanged() const;
	const auto& getValueList() const { return m_valueList; }

protected:
	template<typename T> T read(Value<T>& value);
	template<typename T> void write(Value<T>& value);
	template<typename T> BYTE* read(BinaryValue<T>& value);
	template<typename T> void write(BinaryValue<T>& value);
	HRESULT read(IValue& value, std::unique_ptr<BYTE[]>& data, DWORD expectedSize = 0);
	HRESULT write(IValue& value, const BYTE* data, DWORD size);

protected:
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
CString CSettings::BinaryValue<T>::DefaultValueHandler::valueToString(const BinaryValue<T>&) const
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
	if(SUCCEEDED(read(value, data, sizeof(T)))) {
		return data.release();
	} else {
		return nullptr;
	}
}

template<typename T>
void CSettings::write(BinaryValue<T>& value)
{
	write(value, (BYTE*)(T*)value, sizeof(T));
}

#pragma endregion

template<typename T>
int operator &(const CSettings::EnumValue<T>& a, T b) { return (int)a & (int)b; }

template<typename T>
int operator &(T a, const CSettings::EnumValue<T>& b) { return (int)a & (int)b; }

template<typename T>
int operator &(const CSettings::EnumValue<T>& a, const CSettings::EnumValue<T>& b) { return (int)a & (int)b; }
