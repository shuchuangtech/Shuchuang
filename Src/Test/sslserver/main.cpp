#include "Common/PrintLog.h"
#include "Poco/Types.h"
#include "Poco/Net/Context.h"
#include "Poco/Net/SecureServerSocket.h"
#include "Poco/Net/SocketAddress.h"
#include "Poco/Net/StreamSocket.h"
#include "Poco/Net/SecureStreamSocket.h"
#include "Poco/Timespan.h"

//static RSA *tmp_rsa_cb(SSL *s, int is_export, int keylength);
/*
static DH *get_dh2048(void);
static unsigned char dh2048_p[] = {
	0xF6,0x42,0x57,0xB7,0x08,0x7F,0x08,0x17,0x72,0xA2,0xBA,0xD6,
    0xA9,0x42,0xF3,0x05,0xE8,0xF9,0x53,0x11,0x39,0x4F,0xB6,0xF1,
    0x6E,0xB9,0x4B,0x38,0x20,0xDA,0x01,0xA7,0x56,0xA3,0x14,0xE9,
    0x8F,0x40,0x55,0xF3,0xD0,0x07,0xC6,0xCB,0x43,0xA9,0x94,0xAD,
    0xF7,0x4C,0x64,0x86,0x49,0xF8,0x0C,0x83,0xBD,0x65,0xE9,0x17,
    0xD4,0xA1,0xD3,0x50,0xF8,0xF5,0x59,0x5F,0xDC,0x76,0x52,0x4F,
    0x3D,0x3D,0x8D,0xDB,0xCE,0x99,0xE1,0x57,0x92,0x59,0xCD,0xFD,
    0xB8,0xAE,0x74,0x4F,0xC5,0xFC,0x76,0xBC,0x83,0xC5,0x47,0x30,
    0x61,0xCE,0x7C,0xC9,0x66,0xFF,0x15,0xF9,0xBB,0xFD,0x91,0x5E,
    0xC7,0x01,0xAA,0xD3,0x5B,0x9E,0x8D,0xA0,0xA5,0x72,0x3A,0xD4,
    0x1A,0xF0,0xBF,0x46,0x00,0x58,0x2B,0xE5,0xF4,0x88,0xFD,0x58,
    0x4E,0x49,0xDB,0xCD,0x20,0xB4,0x9D,0xE4,0x91,0x07,0x36,0x6B,
    0x33,0x6C,0x38,0x0D,0x45,0x1D,0x0F,0x7C,0x88,0xB3,0x1C,0x7C,
    0x5B,0x2D,0x8E,0xF6,0xF3,0xC9,0x23,0xC0,0x43,0xF0,0xA5,0x5B,
    0x18,0x8D,0x8E,0xBB,0x55,0x8C,0xB8,0x5D,0x38,0xD3,0x34,0xFD,
    0x7C,0x17,0x57,0x43,0xA3,0x1D,0x18,0x6C,0xDE,0x33,0x21,0x2C,
    0xB5,0x2A,0xFF,0x3C,0xE1,0xB1,0x29,0x40,0x18,0x11,0x8D,0x7C,
    0x84,0xA7,0x0A,0x72,0xD6,0x86,0xC4,0x03,0x19,0xC8,0x07,0x29,
    0x7A,0xCA,0x95,0x0C,0xD9,0x96,0x9F,0xAB,0xD0,0x0A,0x50,0x9B,
    0x02,0x46,0xD3,0x08,0x3D,0x66,0xA4,0x5D,0x41,0x9F,0x9C,0x7C,
    0xBD,0x89,0x4B,0x22,0x19,0x26,0xBA,0xAB,0xA2,0x5E,0xC3,0x55,
    0xE9,0x32,0x0B,0x3B,

};
static unsigned char dh2048_g[] = {
	0x02,
};

DH* get_dh2048()
{
	DH * dh;
	if((dh = DH_new()) == NULL)
		return NULL;
	dh->p=BN_bin2bn(dh2048_p, sizeof(dh2048_p), NULL);
	dh->g=BN_bin2bn(dh2048_g, sizeof(dh2048_g), NULL);
	if(dh->p == NULL || dh->g == NULL)
	{
		DH_free(dh);
		return NULL;
	}
	return dh;
}
*/
/*
static RSA *tmp_rsa_cb(SSL *s, int is_export, int keylength)
{
	BIGNUM *bn = NULL;
	static RSA *rsa_tmp = NULL;
    if (!rsa_tmp && ((bn = BN_new()) == NULL))
        printf("Allocation error in generating RSA key\n");
    if (!rsa_tmp && bn) {
        printf("Generating temp (%d bit) RSA key...", keylength);
        if (!BN_set_word(bn, RSA_F4) || ((rsa_tmp = RSA_new()) == NULL) ||
            !RSA_generate_key_ex(rsa_tmp, keylength, bn, NULL)) {
            if (rsa_tmp)
                RSA_free(rsa_tmp);
            rsa_tmp = NULL;
        }
		printf("\n");
        BN_free(bn);
    }
    return (rsa_tmp);
}

int rand_load_file()
{
	char buffer[200];
	const char* file = NULL;
	file = RAND_file_name(buffer, sizeof buffer);
	if(file == NULL || !RAND_load_file(file, -1))
	{
		if(RAND_status() == 0)
		{
			printf("unable to load 'random state'\n");
			printf("This means that the random number generator has not been seeded\n");
			printf("with much random data.\n");
		}
	}
}
*/
/*
void listenSslSocket()
{
	SSL_load_error_strings();
	OpenSSL_add_ssl_algorithms();
	const SSL_METHOD *meth = NULL;
	SSL_CTX *ctx = NULL;
	SSL_CTX *ctx2 = NULL;
	BIO* bio_err = BIO_new_fp(stderr, BIONOCLOSE);

	meth = SSLv23_server_method();

	CRYPTO_malloc_init();
	ERR_load_crypto_strings();
	EVP_PKEY *s_key = NULL;
	X509 *s_cert = NULL;
	
	SSL_CONF_CTX *cctx = NULL;
	cctx = SSL_CONF_CTX_new();
	if(!cctx)
	{
		printf("cctx error\n");
		goto bad;
	}
	SSL_CONF_CTX_set_flags(cctx, SSL_CONF_FLAG_SERVER);
	SSL_CONF_CTX_set_flags(cctx, SSL_CONF_FLAG_CMDLINE);

	s_key = load_key(bio_err, "my.key", FORMAT_PEM, 0, NULL, NULL, "server certificate private key file");
	if(!s_key)
	{
		printf("s_key error\n");
		goto bad;
	}

	s_cert = load_cert(bio_err, "my.crt", FORMAT_PEM, NULL, NULL, "server certificate file");
	if(!s_cert)
	{
		printf("s_cert error\n");
		goto bad;
	}

	if(!rand_load_file() && !RAND_status())
	{
		printf("warning, not much extra random data.\n");
		goto bad;
	}

	ctx = SSL_CTX_new(meth);
	if(ctx == NULL)
	{
		printf("ctx error\n");
		goto bad;
	}

	SSL_CTX_set_quite_shutdown(ctx, 1);
	SSL_CTX_sess_set_cache_size(ctx, 128);

	if(!SSL_CTX_set_default_verify_paths(ctx))
	{
		printf("load default verify paths error\n");
	}

	SSL_CONF_CTX_set_ssl_ctx(cctx, ctx);
	if(SSL_CONF_cmd(cctx, "-named_curve", "P-256") <= 0)
	{
		printf("Error setting EC curve\n");
		goto bad;
	}
	if(!SSL_CONF_CTX_finish(cctx))
	{
		printf("Error finishing context\n");
		goto bad;
	}

	

bad:
	if(cctx)
		SSL_CONF_CTX_free(cctx);
	if(ctx != NULL)
		SSL_CTX_free(ctx);
	if(s_cert)
		X509_free(s_cert);
	if(s_key)
		EVP_PKEY_free(s_key);
	EVP_cleanup();
	CRYPTO_cleanup_all_ex_data();
	RAND_cleanup();
	ERR_free_strings();

}
*/
void listenPocoSocket(int argc, char** argv)
{
	Poco::Net::Context::Ptr pContext = NULL;
	std::string ciphers;
	//ciphers = "ALL:!LOW:!EXP:!MD5:!@STRENGTH";
	ciphers = "HIGH";
//	ciphers = "EECDH+aRSA+AESGCM";
	pContext = new Poco::Net::Context(Poco::Net::Context::SERVER_USE,
										"./my.key",
										"./my.crt",
										"",
										Poco::Net::Context::VERIFY_RELAXED,
										9,
										false,
										ciphers);
	
	
	SSL_CTX * ssl_context = pContext->sslContext();
/*
	DH * dh = get_dh2048();
	if(dh == NULL)
	{
		printf("dh is NULL\n");
		return;
	}
	SSL_CTX_set_tmp_dh(ssl_context, dh);
	//SSL_CTX_set_tmp_rsa_callback(ssl_context, tmp_rsa_cb);
*/
	/*
	EC_KEY *ecdh = NULL;

	ecdh = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
	if(ecdh == NULL)
	{
		printf("ecdh error\n");
		return;
	}
	SSL_CTX_set_tmp_ecdh(ssl_context, ecdh);
	EC_KEY_free(ecdh);
	*/
	
	
	SSL_CONF_CTX * cctx = NULL;
	cctx = SSL_CONF_CTX_new();
	SSL_CONF_CTX_set_flags(cctx, SSL_CONF_FLAG_SERVER);
	SSL_CONF_CTX_set_flags(cctx, SSL_CONF_FLAG_CMDLINE);
	SSL_CONF_CTX_set_ssl_ctx(cctx, ssl_context);
	if(SSL_CONF_cmd(cctx, "-named_curve", "P-256") <= 0)
	{
		printf("Error setting EC curve\n");
		SSL_CONF_CTX_free(cctx);
		return;
	}
	if(!SSL_CONF_CTX_finish(cctx))
	{
		printf("Error finishing config context\n");
		SSL_CONF_CTX_free(cctx);
		return;
	}
	SSL_CONF_CTX_free(cctx);
	

	/*
	SSL* con;
	con = SSL_new(ssl_context);
	STACK_OF(SSL_CIPHER) * sk;
	sk = SSL_get_ciphers(con);
	int j = sk_SSL_CIPHER_num(sk);
	const SSL_CIPHER * c;
	for(int i = 0; i < j; i++)
	{
		c = sk_SSL_CIPHER_value(sk, i);
		printf("%-11s:%-25s\n", SSL_CIPHER_get_version(c), SSL_CIPHER_get_name(c));
	}
	SSL_free(con);
	*/
	if(argc < 2)
	{
	//	DH_free(dh);
		return;
	}
	Poco::UInt16 port = atoi(argv[1]);
	Poco::Net::SecureServerSocket* ssl_acceptor = NULL;
	try
	{
		ssl_acceptor = new Poco::Net::SecureServerSocket(port, 64, pContext);
	}
	catch(Poco::Exception& e)
	{
		printf("error secure server socket, %s\n", e.message().c_str());
		return;
	}
	Poco::Net::SocketAddress clientAddress;
	Poco::Net::SecureStreamSocket* ss = NULL;
	try
	{
		printf("liten port %d.\n", port);
		Poco::Net::SecureStreamSocket tss = ssl_acceptor->acceptConnection(clientAddress);
		printf("accept connection.\n");
		ss = new Poco::Net::SecureStreamSocket(tss);;
	}
	catch(Poco::Exception& e)
	{
		printf("accept error, %s.\n", e.message().c_str());
		return;
	}
	if(ss->poll(Poco::Timespan(10,0), Poco::Net::Socket::SELECT_READ) > 0)
	{
		char buf[512] = {0, };
		try
		{
			ss->receiveBytes(buf, 512);
		}
		catch(Poco::Exception& e)
		{
			printf("recv error %s\n", e.message().c_str());
			//DH_free(dh);
			return;
		}
		printf("recv buf:%s", buf);
		ss->sendBytes(buf, 512);
	}
//	DH_free(dh);
}

int main(int argc, char** argv)
{
	initPrintLogger("");
	listenPocoSocket(argc, argv);
	return 0;
}

