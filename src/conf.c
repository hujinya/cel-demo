#include "conf.h"
#include "cel/json.h"
#include "cel/crypto/rc4.h"

static TCHAR s_key[8][8] = 
{
    { 0x09, 0x78, 0x1A, 0x4C, 0xD5, 0xAB, 0xE6, 0x34 }, 
    { 0x0E, 0x38, 0x8A, 0x49, 0x77, 0x7D, 0x45, 0x74 }, 
    { 0x88, 0x60, 0x1C, 0xCC, 0xE4, 0x36, 0x89, 0x4E }, 
    { 0x6D, 0x7F, 0x9E, 0x3C, 0x9C, 0x11, 0x3C, 0xFD }, 
    { 0x56, 0xF8, 0xE4, 0x33, 0x66, 0x34, 0x96, 0x25 }, 
    { 0xEA, 0xDE, 0x38, 0x26, 0xD7, 0xD7, 0x48, 0x56 }, 
    { 0xDC, 0xCF, 0x2E, 0x33, 0x09, 0x88, 0xC2, 0x5F }, 
    { 0x49, 0x7A, 0xDA, 0xDE, 0x55, 0x03, 0xAA, 0x67 }
};

Conf conf;

static int conf_decrypt(TCHAR *in, TCHAR *out, size_t out_len)
{
    int in_len;
    CelRc4Context ctx;
    BYTE _out[1024];

    if (in[0] == '$')
    {
        cel_arc4_setup(&ctx, (unsigned char *)s_key, 64);
        in_len = _tcslen((TCHAR *)&in[1]);
        cel_hextobin((TBYTE *)&in[1], in_len, (TBYTE *)_out, &out_len);
        cel_arc4_crypt(&ctx, out_len, (BYTE *)_out, (BYTE *)out);
        out[out_len] = _T('\0');
    }
    else
    {
        _sntprintf(out, out_len, _T("%s"), in);
    }

    return 0;
}

static int conf_encrypt(TCHAR *in, TCHAR *out, size_t out_len)
{
    int in_len;
    CelRc4Context ctx;
    BYTE _out[1024];

    if (in[0] != '$')
    {
        cel_arc4_setup(&ctx, (unsigned char *)s_key, 64);
        in_len = _tcslen(in);
        cel_arc4_crypt(&ctx, in_len, (BYTE *)in, (BYTE *)_out);
        _out[in_len] = '\0';
        //_tprintf(_T("encrypt bin %s, len = %d\r\n"), obuf, len);
        out[0] = '$';
        cel_bintohex((TBYTE *)_out, in_len, (TBYTE *)&out[1], &out_len, 0);
    }
    else
    {
        _sntprintf(out, out_len, _T("%s"), in);
    }

    return 0;
}

int conf_read(Conf *conf, const char *file)
{
    CelJson *json;
    CelJsonNode *root_node, *node1, *node2;
    TCHAR encrypt_buf[1024];

    if ((json = cel_json_new_file(file)) == NULL
        || (root_node = cel_json_root_node(json)) == NULL)
        return -1;
    if ((node1 = cel_json_object_get(root_node, _T("wmip-server"))) != NULL)
    {
        cel_json_object_get_string(node1, 
            _T("address"), conf->api_server.address, CEL_ADDRLEN);
         if ((node2 = cel_json_object_get(node1, _T("ssl"))) != NULL)
        {
            cel_json_object_get_int(node2, _T("on"), &(conf->api_server.ssl.on));
            cel_json_object_get_string(node2, 
                _T("protocols"), conf->api_server.ssl.protocols, 32);
            cel_json_object_get_string(node2, 
                _T("cert"), conf->api_server.ssl.cert, CEL_PATHLEN);
            cel_json_object_get_string(node2, 
                _T("key"), conf->api_server.ssl.key, CEL_PATHLEN);
            cel_json_object_get_string(node2, 
                _T("key-pswd"), encrypt_buf, CEL_PWLEN);
            conf_decrypt(encrypt_buf, conf->api_server.ssl.key_pswd, CEL_PWLEN);
            cel_json_object_get_string(node2, _T("ciphers"), conf->api_server.ssl.ciphers, 64);
        }
    }
    if ((node1 = cel_json_object_get(root_node, _T("log"))) != NULL)
    {
        cel_json_object_get_int(node1, _T("level"), &(conf->log.level));
        cel_json_object_get_string(node1, _T("path"), conf->log.path, CEL_PATHLEN);
    }
    cel_json_free(json);

    return 0;
}

int conf_write(Conf *conf, const char *file)
{
    CelJson *json;
    CelJsonNode *root_node, *node1, *node2;
    TCHAR encrypt_buf[1024];

    json = cel_json_new();

    root_node = cel_json_object_new();
    json->root_node = root_node;

    node1 = cel_json_object_new();
    cel_json_object_add(root_node, _T("api-server"), node1);
    cel_json_object_add_string(node1, _T("address"), conf->api_server.address);
    node2 = cel_json_object_new();
    cel_json_object_add(node1, _T("ssl"), node2);
    cel_json_object_add_int(node2, _T("on"), conf->api_server.ssl.on);
    cel_json_object_add_string(node2, _T("protocols"), conf->api_server.ssl.protocols);
    cel_json_object_add_string(node2, _T("cert"), conf->api_server.ssl.cert);
    cel_json_object_add_string(node2, _T("key"), conf->api_server.ssl.key);
    conf_encrypt(conf->api_server.ssl.key_pswd, encrypt_buf, 1024);
    cel_json_object_add_string(node2, _T("key-pswd"), encrypt_buf);
    cel_json_object_add_string(node2, _T("ciphers"), conf->api_server.ssl.ciphers);


    node1 = cel_json_object_new();
    cel_json_object_add(root_node, _T("log"), node1);
    cel_json_object_add_int(node1, _T("level"), conf->log.level);
    cel_json_object_add_string(node1, _T("path"), conf->log.path);

    cel_json_serialize_file(json, file, 1);
    cel_json_free(json);
   
    return 0;
}
