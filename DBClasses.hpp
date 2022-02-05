#pragma once
#include <iostream>
#include <vector>

enum dbRetType
{
	None,
	Short,
	Int,
	Int64,
	String
};

class IFieldData
{
	public:
	IFieldData()=default;
	virtual ~IFieldData()
	{

	};
	std::string fieldName;
	dbRetType dbType;
	int fnumber;
	virtual std::string GetStrValue()
	{
		return "";
	}

};

template<typename T>
class FieldData : public IFieldData
{
	private:
	T value;

	public:
	T GetValue()
	{
		return value;
	}
	void SetValue(T pvalue)
	{
		value = pvalue;
	}

	virtual std::string GetStrValue()
	{
		std::ostringstream dtss;
		dtss << value;
		return dtss.str();
	}

	virtual ~FieldData()
	{

	}
};

class RowData
{
	public:
	RowData() = default;
	int RowNumber;
	std::vector<IFieldData*> Fields;
	~RowData()
	{
		for(auto i= Fields.begin(); i != Fields.end(); i++)
        {
            delete *i;
        }
		Fields.clear();
		Fields.shrink_to_fit();
	}
};