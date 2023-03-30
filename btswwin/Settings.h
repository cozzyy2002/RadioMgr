#pragma once

#include <vector>

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
	};

	template<typename T>
	class Value : public IValue
	{
	public:
#pragma region Implementation of IValue interface.
		void read(CSettings* settings) override
		{
			m_value = settings->read(this);
			m_savedValue = m_value;
		}
		void write(CSettings* settings) override
		{
			settings->write(this);
			m_savedValue = m_value;
		}
		bool isChanged() const override { return (m_value != m_savedValue); }
#pragma endregion

		explicit Value(LPCTSTR name, const T& defaultValue) { construct(name, defaultValue); }
		explicit Value(LPCTSTR name) { construct(name, T()); }

		const T& get() const { return m_value; }
		void set(const T& value) { m_value = value; }
		LPCTSTR getName() const { return m_name.GetString(); }
		const T& getDefault() const { return m_defaultValue; }

	protected:
		void construct(LPCTSTR name, const T& defaultValue)
		{
			m_name = name;
			m_value = m_savedValue = m_defaultValue = defaultValue;
		}

		CString m_name;
		T m_value;				// Current value. Changed by set() and Returned by get().
		T m_savedValue;			// Value saved in profile storage.
		T m_defaultValue;		// Value to be set to m_value if the name doesn't exist when read() is called.
	};

	explicit CSettings(LPCTSTR sectionName);

	template<size_t size>
	HRESULT load(IValue* (&valueList)[size]) { return load(valueList, size); }

	HRESULT load(IValue** valueList, size_t size);
	HRESULT save(bool force = false);

protected:
	template<typename T> T read(Value<T>* value);
	template<typename T> void write(Value<T>* value);

protected:
	CString m_sectionName;
	std::vector<IValue*> m_valueList;
};

template<> bool CSettings::read(Value<bool>* value);
template<> void CSettings::write(Value<bool>* value);
template<> int CSettings::read(Value<int>* value);
template<> void CSettings::write(Value<int>* value);
template<> CString CSettings::read(Value<CString>* value);
template<> void CSettings::write(Value<CString>* value);
