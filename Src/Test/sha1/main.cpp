#include "Poco/SHA1Engine.h"
#include "Poco/MD5Engine.h"
#include <stdio.h>
using namespace Poco;

std::string generateMD5Password(std::string prefix, std::string password, std::string challenge)
{
	MD5Engine md5;
	SHA1Engine sha1;
	sha1.update(password);
	const DigestEngine::Digest& digestPassword = sha1.digest();
	std::string sha1pass(DigestEngine::digestToHex(digestPassword));

	printf("sha1(password) is:%s\n", sha1pass.c_str());
	std::string prefix_passwd = prefix + sha1pass;
	md5.update(prefix_passwd);
	const DigestEngine::Digest& digest = md5.digest();
	std::string prefixpassmd5(DigestEngine::digestToHex(digest));
	printf("md5(prefix + sha1password) is %s\n", prefixpassmd5.c_str());
	prefixpassmd5 += challenge;
	
	md5.reset();
	md5.update(prefixpassmd5);
	const DigestEngine::Digest& dg = md5.digest();
	std::string passs(DigestEngine::digestToHex(dg));
	printf("md5(md5(prefix + sha1password)) + challenge is:%s\n", passs.c_str());
	return passs;
}

int main(int argc, char** argv)
{
	generateMD5Password("login", "admin@shuchuang", "1809b11bd80cad0020226f8315e34086");
	return 0;
}

