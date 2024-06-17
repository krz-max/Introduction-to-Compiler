#ifndef AST_AST_TYPE_H
#define AST_AST_TYPE_H

#include <string>
#include <vector>

namespace ScalarDataType
{
	const int INT_TYPE = 0;
	const int REAL_TYPE = 1;
	const int BOOL_TYPE = 2;
	const int STRING_TYPE = 3;
	const int VOID = 4;
};

union ScalarValue
{
	int int_value;
	float real_value;
	bool bool_value;
	const char *string_value;
};

class DataType
{
public:
	DataType(int type);
	DataType(int type, std::vector<int> *dims);
	~DataType();

	int getType() const;
	std::vector<int> *getDims() const;
	const char *getTypeCString();

private:
	int type;
	std::vector<int> *dims;
	std::string type_str = "";

	void setTypeStr();
};

#endif