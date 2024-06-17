#include "AST/AstType.hpp"

DataType::DataType(int type) : type(type), dims(nullptr) { setTypeStr(); }

DataType::DataType(int type, std::vector<int> *dims)
	: type(type), dims(dims) { setTypeStr(); }

DataType::~DataType()
{
	if (dims != nullptr)
	{
		delete dims;
	}
}

int DataType::getType() const { return type; }

std::vector<int> *DataType::getDims() const { return dims; }

const char *DataType::getTypeCString()
{
	return type_str.c_str();
}

void DataType::setTypeStr()
{
	switch (type)
	{
	case ScalarDataType::INT_TYPE:
		type_str = "integer";
		break;
	case ScalarDataType::REAL_TYPE:
		type_str = "real";
		break;
	case ScalarDataType::BOOL_TYPE:
		type_str = "boolean";
		break;
	case ScalarDataType::STRING_TYPE:
		type_str = "string";
		break;
	case ScalarDataType::VOID:
		type_str = "void";
		break;
	}

	if (dims != nullptr)
	{
		type_str += " ";
		for (int dim : *dims)
		{
			type_str += "[";
			type_str += std::to_string(dim);
			type_str += "]";
		}
	}
}