#include "Poco/Util/JSONConfiguration.h"
#include "Poco/FileStream.h"
#include "Poco/JSON/Object.h"
#include "Poco/JSON/Array.h"
using namespace Poco;
int main()
{
	JSON::Object::Ptr pObj = new JSON::Object;
	pObj->set("version", "6c9923ca");
	pObj->set("buildtime", "2016-03-07 19:11:41 +0800");
	pObj->set("checksum", "asdfghjkllq1");
	JSON::Array::Ptr pArr = new JSON::Array;
	pArr->add("1.new1");
	pArr->add("2.new2");
	pArr->add("3.new3");
	pObj->set("newfeature", pArr);
	Util::JSONConfiguration updateConf(pObj);
	FileOutputStream confFile("./Info");
	updateConf.save(confFile);
	return 0;
}

