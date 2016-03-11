#include "Poco/MD5Engine.h"
#include "Poco/FileStream.h"
#include <stdio.h>
#include <iostream>
#include <string.h>
int main(int argc, char** argv)
{
	if(argc < 2)
		return 0;
	Poco::FileInputStream fis(argv[1], std::ios::in|std::ios::binary);
	int length = 0;
	char buf[8192] = {0, };
	Poco::MD5Engine md5;
	while(1)
	{
		memset(buf, 0, 8192);
		fis.read(buf, 8192);
		length = fis.gcount();
		md5.update(buf, length);
		if(fis.eof())
			break;
	}
	const Poco::DigestEngine::Digest& digest = md5.digest();
	std::string md5str(Poco::DigestEngine::digestToHex(digest));
	printf("%s\n", md5str.c_str());

	return 0;
}

