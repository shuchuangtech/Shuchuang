#include "Poco/JSON/Array.h"
#include "Poco/Dynamic/Struct.h"
#include "Poco/JSON/Object.h"
#include "Poco/Dynamic/Var.h"
#include <stdio.h>
using namespace Poco;
typedef std::vector<Dynamic::Var> MyVector;
void displayMyVector(MyVector& vec)
{
	printf("display my vector, vector size:%d\n", vec.size());
	for(unsigned int i = 0; i < vec.size(); i++)
	{
		Dynamic::Var var = vec.at(i);
		Poco::DynamicStruct ds = var.extract<DynamicStruct>();
		printf("Array[%u]:%s\n", i, ds.toString().c_str());
	}
}

void displayArray(JSON::Array::Ptr pArray)
{
	printf("display array, array size:%d\n", pArray->size());
	for(unsigned int i = 0; i < pArray->size(); i++)
	{
		Dynamic::Var var = pArray->get(i);
		Poco::DynamicStruct ds = var.extract<DynamicStruct>();
		printf("Array[%u]:%s\n", i, ds.toString().c_str());
	}
}

int main(int argc, char** argv)
{
//	MyVector vec;
	Poco::JSON::Array::Ptr pArray = new Poco::JSON::Array;
	Poco::DynamicStruct ds1;
	ds1["aaa"] = "a1";
	ds1["bbb"] = 10;
	ds1["ccc"] = 111;
	pArray->add(ds1);
	displayArray(pArray);
//	vec.push_back(ds1);
//	displayMyVector(vec);
	Poco::DynamicStruct ds2;
	ds2["aaa"] = "a2";
	ds2["bbb"] = 20;
	ds2["ccc"] = 222;
	pArray->add(ds2);
	displayArray(pArray);
//	vec.push_back(ds2);
//	displayMyVector(vec);
	Poco::DynamicStruct ds3;
	ds3["aaa"] = "a3";
	ds3["bbb"] = 30;
	ds3["ccc"] = 333;
	pArray->add(ds3);
	displayArray(pArray);
//	vec.push_back(ds3);
//	displayMyVector(vec);
	return 0;
}

