#include "Poco/MD5Engine.h"
#include "Poco/FileStream.h"
#include "Poco/JSON/Object.h"
#include "Poco/JSON/Array.h"
#include "Poco/Util/JSONConfiguration.h"
using namespace Poco;
std::string fileMD5(const std::string file)
{
	FileInputStream fis(file, std::ios::in|std::ios::binary);
	int length = 0;
	char buf[8192] = {0, };
	MD5Engine md5;
	while(1)
	{
		memset(buf, 0, 8192);
		fis.read(buf, 8192);
		length = fis.gcount();
		md5.update(buf, length);
		if(fis.eof())
			break;
	}
	const DigestEngine::Digest& digest = md5.digest();
	std::string md5str(DigestEngine::digestToHex(digest));
	return md5str;
}

int main(int argc, char** argv)
{
	if(argc < 2)
		return 0;
	std::string file = argv[1];
	std::string filemd5 = fileMD5(file);
	FileInputStream fis("/home/huang_jian/Dev_Env/Shuchuang/Include/release.h", std::ios::in);
	std::string version = "";
	char buf[128] = {0, };
	fis.getline(buf, 128, '\n');
	version = buf;
	memset(buf, 0, 128);
	fis.getline(buf, 128, '\n');
	std::string buildtime = "";
	memset(buf, 0, 128);
	fis.getline(buf, 128, '\n');
	buildtime = buf;
	JSON::Object::Ptr pInfo = new JSON::Object;
	pInfo->set("version", version.substr(21, 8));
	pInfo->set("buildtime", buildtime.substr(19, 25));
	pInfo->set("checksum", filemd5);
	JSON::Array::Ptr pArr = new JSON::Array;
	pArr->add("1.new1");
	pArr->add("2.new2");
	pArr->add("3.new3");
	pInfo->set("newfeature", pArr);
	Util::JSONConfiguration infoConf(pInfo);
	FileOutputStream confFile("./Info");
	infoConf.save(confFile);
}

