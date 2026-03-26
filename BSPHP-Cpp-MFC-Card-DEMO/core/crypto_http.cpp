/*
 * BSPHP HTTP 加密通信实现 | BSPHP HTTP 加密通訊實作 | BSPHP HTTP Encryption Implementation
 * 含 MD5/AES/RSA/Base64 及 WinHTTP POST
 */
#include "http.h"
#include <windows.h>
#include <bcrypt.h>
#include <wincrypt.h>
#include <winhttp.h>
#include <ctime>
#include <sstream>
#include <cctype>
#include <iomanip>
#include <vector>
#include <cstring>
#include <cwchar>
#include <ncrypt.h>
// Note: std::cout debug prints replaced by OutputDebugString (DebugLogUtf8).

#include "debug_log.h"

#pragma comment(lib, "bcrypt.lib")
#pragma comment(lib, "crypt32.lib")
#pragma comment(lib, "ncrypt.lib")
#pragma comment(lib, "winhttp.lib")

namespace bsphp {

std::string md5_hex(const std::string& input) {
    BCRYPT_ALG_HANDLE hAlg = nullptr;
    BCRYPT_HASH_HANDLE hHash = nullptr;
    NTSTATUS status = BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_MD5_ALGORITHM, nullptr, 0);
    if (status != 0) return "";

    DWORD cbHashObject = 0, cbData = 0;
    if (BCryptGetProperty(hAlg, BCRYPT_OBJECT_LENGTH, (PUCHAR)&cbHashObject, sizeof(DWORD), &cbData, 0) != 0) {
        BCryptCloseAlgorithmProvider(hAlg, 0);
        return "";
    }
    std::vector<UCHAR> hashObject(cbHashObject);

    DWORD cbHash = 0;
    if (BCryptGetProperty(hAlg, BCRYPT_HASH_LENGTH, (PUCHAR)&cbHash, sizeof(DWORD), &cbData, 0) != 0) {
        BCryptCloseAlgorithmProvider(hAlg, 0);
        return "";
    }
    std::vector<UCHAR> hash(cbHash);

    status = BCryptCreateHash(hAlg, &hHash, hashObject.data(), cbHashObject, nullptr, 0, 0);
    if (status != 0) { BCryptCloseAlgorithmProvider(hAlg, 0); return ""; }

    status = BCryptHashData(hHash, (PUCHAR)input.data(), (ULONG)input.size(), 0);
    if (status != 0) { BCryptDestroyHash(hHash); BCryptCloseAlgorithmProvider(hAlg, 0); return ""; }
    status = BCryptFinishHash(hHash, hash.data(), cbHash, 0);
    BCryptDestroyHash(hHash);
    BCryptCloseAlgorithmProvider(hAlg, 0);
    if (status != 0) return "";

    std::string hex;
    for (DWORD i = 0; i < cbHash; ++i) {
        char buf[4];
        sprintf_s(buf, "%02x", hash[i]);
        hex += buf;
    }
    return hex;
}

static std::string md5(const std::string& input) { return md5_hex(input); }

static std::string base64_encode(const unsigned char* data, size_t len) {
    DWORD cbOut = 0;
    CryptBinaryToStringA(data, (DWORD)len, CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, nullptr, &cbOut);
    if (cbOut == 0) return "";
    std::vector<char> buf(cbOut);
    if (!CryptBinaryToStringA(data, (DWORD)len, CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, buf.data(), &cbOut))
        return "";
    return std::string(buf.data());
}

static std::string base64_decode(const std::string& input) {
    DWORD cbOut = 0;
    CryptStringToBinaryA(input.c_str(), (DWORD)input.size(), CRYPT_STRING_BASE64, nullptr, &cbOut, nullptr, nullptr);
    if (cbOut == 0) return "";
    std::vector<UCHAR> buf(cbOut);
    if (!CryptStringToBinaryA(input.c_str(), (DWORD)input.size(), CRYPT_STRING_BASE64, buf.data(), &cbOut, nullptr, nullptr))
        return "";
    return std::string((char*)buf.data(), cbOut);
}

static std::string url_encode(const std::string& value) {
    std::ostringstream escaped;
    escaped.fill('0');
    escaped << std::hex;
    for (unsigned char c : value) {
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~')
            escaped << c;
        else
            escaped << '%' << std::setw(2) << (int)c;
    }
    return escaped.str();
}

static std::string url_decode(const std::string& value) {
    std::string result;
    for (size_t i = 0; i < value.size(); ++i) {
        if (value[i] == '%' && i + 2 < value.size()) {
            int hex;
            sscanf_s(value.substr(i + 1, 2).c_str(), "%x", &hex);
            result += (char)hex;
            i += 2;
        } else if (value[i] == '+') {
            result += ' ';
        } else {
            result += value[i];
        }
    }
    return result;
}

static std::string get_time_str() {
    time_t t = time(nullptr);
    struct tm tm;
    localtime_s(&tm, &t);
    char buf[32];
    sprintf_s(buf, "%04d-%02d-%02d %02d:%02d:%02d",
        tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
        tm.tm_hour, tm.tm_min, tm.tm_sec);
    return std::string(buf);
}

static std::string pkcs7_pad(const std::string& data, int block_size) {
    int pad_len = block_size - (int)(data.size() % block_size);
    std::string result = data;
    result.append(pad_len, (char)pad_len);
    return result;
}

static std::string pkcs7_unpad(const std::string& data) {
    if (data.empty()) return "";
    int pad_len = (unsigned char)data.back();
    if (pad_len <= 0 || pad_len > 16 || data.size() < (size_t)pad_len) return data;
    return data.substr(0, data.size() - pad_len);
}

static std::string aes_cbc_encrypt(const std::string& plain, const std::string& key_hex) {
    std::string key = key_hex.substr(0, 16);
    std::string padded = pkcs7_pad(plain, 16);

    BCRYPT_ALG_HANDLE hAlg = nullptr;
    NTSTATUS status = BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_AES_ALGORITHM, nullptr, 0);
    if (status != 0) return "";

    if (BCryptSetProperty(hAlg, BCRYPT_CHAINING_MODE, (PUCHAR)BCRYPT_CHAIN_MODE_CBC, (ULONG)((wcslen(BCRYPT_CHAIN_MODE_CBC) + 1) * sizeof(WCHAR)), 0) != 0) {
        BCryptCloseAlgorithmProvider(hAlg, 0);
        return "";
    }

    DWORD cbKeyObject = 0, cbData = 0;
    if (BCryptGetProperty(hAlg, BCRYPT_OBJECT_LENGTH, (PUCHAR)&cbKeyObject, sizeof(DWORD), &cbData, 0) != 0) {
        BCryptCloseAlgorithmProvider(hAlg, 0);
        return "";
    }
    std::vector<UCHAR> keyObject(cbKeyObject);

    BCRYPT_KEY_HANDLE hKey = nullptr;
    status = BCryptGenerateSymmetricKey(hAlg, &hKey, keyObject.data(), cbKeyObject, (PUCHAR)key.c_str(), 16, 0);
    BCryptCloseAlgorithmProvider(hAlg, 0);
    if (status != 0) return "";

    std::vector<UCHAR> iv(16);
    memcpy(iv.data(), key.c_str(), 16);

    ULONG cbResult = 0;
    status = BCryptEncrypt(hKey, (PUCHAR)padded.data(), (ULONG)padded.size(), nullptr, iv.data(), 16, nullptr, 0, &cbResult, BCRYPT_BLOCK_PADDING);
    if (status != 0) { BCryptDestroyKey(hKey); return ""; }
    std::vector<UCHAR> out(cbResult);
    memcpy(iv.data(), key.c_str(), 16);
    status = BCryptEncrypt(hKey, (PUCHAR)padded.data(), (ULONG)padded.size(), nullptr, iv.data(), 16, out.data(), cbResult, &cbResult, BCRYPT_BLOCK_PADDING);
    BCryptDestroyKey(hKey);
    if (status != 0) return "";

    return base64_encode(out.data(), cbResult);
}

static std::string aes_cbc_decrypt(const std::string& b64, const std::string& key_hex) {
    std::string key = key_hex.substr(0, 16);
    std::string ct = base64_decode(b64);
    if (ct.empty()) return "";

    BCRYPT_ALG_HANDLE hAlg = nullptr;
    NTSTATUS status = BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_AES_ALGORITHM, nullptr, 0);
    if (status != 0) return "";

    if (BCryptSetProperty(hAlg, BCRYPT_CHAINING_MODE, (PUCHAR)BCRYPT_CHAIN_MODE_CBC, (ULONG)((wcslen(BCRYPT_CHAIN_MODE_CBC) + 1) * sizeof(WCHAR)), 0) != 0) {
        BCryptCloseAlgorithmProvider(hAlg, 0);
        return "";
    }

    DWORD cbKeyObject = 0, cbData = 0;
    if (BCryptGetProperty(hAlg, BCRYPT_OBJECT_LENGTH, (PUCHAR)&cbKeyObject, sizeof(DWORD), &cbData, 0) != 0) {
        BCryptCloseAlgorithmProvider(hAlg, 0);
        return "";
    }
    std::vector<UCHAR> keyObject(cbKeyObject);

    BCRYPT_KEY_HANDLE hKey = nullptr;
    status = BCryptGenerateSymmetricKey(hAlg, &hKey, keyObject.data(), cbKeyObject, (PUCHAR)key.c_str(), 16, 0);
    BCryptCloseAlgorithmProvider(hAlg, 0);
    if (status != 0) return "";

    std::vector<UCHAR> iv(16);
    memcpy(iv.data(), key.c_str(), 16);

    ULONG cbResult = 0;
    status = BCryptDecrypt(hKey, (PUCHAR)ct.data(), (ULONG)ct.size(), nullptr, iv.data(), 16, nullptr, 0, &cbResult, BCRYPT_BLOCK_PADDING);
    if (status != 0) { BCryptDestroyKey(hKey); return ""; }
    std::vector<UCHAR> out(cbResult);
    memcpy(iv.data(), key.c_str(), 16);
    status = BCryptDecrypt(hKey, (PUCHAR)ct.data(), (ULONG)ct.size(), nullptr, iv.data(), 16, out.data(), cbResult, &cbResult, BCRYPT_BLOCK_PADDING);
    BCryptDestroyKey(hKey);
    if (status != 0) return "";

    return pkcs7_unpad(std::string((char*)out.data(), cbResult));
}

// 最小 DER 解析：从 SPKI 提取 RSA n,e | 最小 DER 解析：從 SPKI 提取 RSA n,e | Minimal DER parse: extract RSA n,e from SPKI
static bool parse_spki_rsa(const unsigned char* der, size_t len, std::vector<UCHAR>& n, std::vector<UCHAR>& e) {
    if (len < 2) return false;
    size_t i = 0;
    if (der[i++] != 0x30) return false;  // SEQUENCE
    size_t seqLen = der[i++];
    if (seqLen & 0x80) { int nBytes = seqLen & 0x7f; seqLen = 0; for (int j = 0; j < nBytes && i < len; ++j) seqLen = (seqLen << 8) | der[i++]; }
    size_t seqEnd = i + seqLen;
    if (i >= len) return false;
    if (der[i++] != 0x30) return false;  // algorithm SEQUENCE - skip
    size_t alglen = der[i++];
    if (alglen & 0x80) { int nb = alglen & 0x7f; alglen = 0; for (int j = 0; j < nb && i < len; ++j) alglen = (alglen << 8) | der[i++]; }
    i += alglen;
    if (i >= len || der[i++] != 0x03) return false;  // BIT STRING
    size_t blen = der[i++];
    if (blen & 0x80) { int nb = blen & 0x7f; blen = 0; for (int j = 0; j < nb && i < len; ++j) blen = (blen << 8) | der[i++]; }
    if (der[i] == 0) ++i;  // unused bits
    const unsigned char* rsaDer = der + i;
    size_t rsaLen = len - i;
    if (rsaLen < 2 || rsaDer[0] != 0x30) return false;
    i = 1;
    size_t rsaSeqLen = rsaDer[i++];
    if (rsaSeqLen & 0x80) { int nb = rsaSeqLen & 0x7f; rsaSeqLen = 0; for (int j = 0; j < nb && i < rsaLen; ++j) rsaSeqLen = (rsaSeqLen << 8) | rsaDer[i++]; }
    auto readInt = [&]() -> std::vector<UCHAR> {
        if (i >= rsaLen || rsaDer[i++] != 0x02) return {};
        size_t ilen = rsaDer[i++];
        if (ilen & 0x80) { int nb = ilen & 0x7f; ilen = 0; for (int j = 0; j < nb && i < rsaLen; ++j) ilen = (ilen << 8) | rsaDer[i++]; }
        if (i + ilen > rsaLen) return {};
        std::vector<UCHAR> v(rsaDer + i, rsaDer + i + ilen);
        i += ilen;
        if (!v.empty() && v[0] == 0) v.erase(v.begin());  // 去除前导 0
        return v;
    };
    n = readInt();
    e = readInt();
    return !n.empty() && !e.empty();
}

// 前向声明 | 前向宣告 | Forward declaration
static bool parse_pkcs1_rsa_private(const unsigned char* der, size_t len, std::vector<UCHAR>& n, std::vector<UCHAR>& e, std::vector<UCHAR>& d,
    std::vector<UCHAR>* p, std::vector<UCHAR>* q, std::vector<UCHAR>* exp1, std::vector<UCHAR>* exp2, std::vector<UCHAR>* coeff);

// 从 PKCS#8 提取 RSA 私钥 | 從 PKCS#8 提取 RSA 私鑰 | Extract RSA private key from PKCS#8
static bool parse_pkcs8_rsa(const unsigned char* der, size_t len, std::vector<UCHAR>& n, std::vector<UCHAR>& e, std::vector<UCHAR>& d,
    std::vector<UCHAR>* p, std::vector<UCHAR>* q, std::vector<UCHAR>* exp1, std::vector<UCHAR>* exp2, std::vector<UCHAR>* coeff) {
    if (len < 2 || der[0] != 0x30) return false;
    size_t i = 1;
    size_t seqLen = der[i++];
    if (seqLen & 0x80) { int nb = seqLen & 0x7f; seqLen = 0; for (int j = 0; j < nb && i < len; ++j) seqLen = (seqLen << 8) | der[i++]; }
    if (i >= len) return false;
    if (der[i++] != 0x02) return false;  // version
    size_t vlen = der[i++];
    i += vlen;
    if (i >= len || der[i++] != 0x30) return false;  // algorithm
    size_t alen = der[i++];
    if (alen & 0x80) { int nb = alen & 0x7f; alen = 0; for (int j = 0; j < nb && i < len; ++j) alen = (alen << 8) | der[i++]; }
    i += alen;
    if (i >= len || der[i++] != 0x04) return false;  // OCTET STRING
    size_t olen = der[i++];
    if (olen & 0x80) { int nb = olen & 0x7f; olen = 0; for (int j = 0; j < nb && i < len; ++j) olen = (olen << 8) | der[i++]; }
    return parse_pkcs1_rsa_private(der + i, olen, n, e, d, p, q, exp1, exp2, coeff);
}

static bool parse_pkcs1_rsa_private(const unsigned char* der, size_t len, std::vector<UCHAR>& n, std::vector<UCHAR>& e, std::vector<UCHAR>& d,
    std::vector<UCHAR>* p = nullptr, std::vector<UCHAR>* q = nullptr, std::vector<UCHAR>* exp1 = nullptr, std::vector<UCHAR>* exp2 = nullptr, std::vector<UCHAR>* coeff = nullptr) {
    if (len < 2 || der[0] != 0x30) return false;
    size_t i = 1;
    size_t seqLen = der[i++];
    if (seqLen & 0x80) { int nb = seqLen & 0x7f; seqLen = 0; for (int j = 0; j < nb && i < len; ++j) seqLen = (seqLen << 8) | der[i++]; }
    auto readInt = [&]() -> std::vector<UCHAR> {
        if (i >= len || der[i++] != 0x02) return {};
        size_t ilen = der[i++];
        if (ilen & 0x80) { int nb = ilen & 0x7f; ilen = 0; for (int j = 0; j < nb && i < len; ++j) ilen = (ilen << 8) | der[i++]; }
        if (i + ilen > len) return {};
        std::vector<UCHAR> v(der + i, der + i + ilen);
        i += ilen;
        if (!v.empty() && v[0] == 0) v.erase(v.begin());
        return v;
    };
    readInt();  // version
    n = readInt();
    e = readInt();
    d = readInt();
    if (p && q && exp1 && exp2 && coeff) {
        *p = readInt();
        *q = readInt();
        *exp1 = readInt();
        *exp2 = readInt();
        *coeff = readInt();
    }
    return !n.empty() && !e.empty() && !d.empty();
}

static std::string rsa_encrypt(const std::string& plain, const std::string& key_b64) {
    std::string keyDer = base64_decode(key_b64);
    if (keyDer.empty()) return "";
    std::vector<UCHAR> n, e;
    if (!parse_spki_rsa((const unsigned char*)keyDer.data(), keyDer.size(), n, e)) return "";

    BCRYPT_ALG_HANDLE hAlg = nullptr;
    NTSTATUS status = BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_RSA_ALGORITHM, nullptr, 0);
    if (status != 0) return "";

    DWORD keySize = (DWORD)(n.size() * 8);
    DWORD cbBlob = sizeof(BCRYPT_RSAKEY_BLOB) + (DWORD)e.size() + (DWORD)n.size();
    std::vector<UCHAR> blob(cbBlob);
    BCRYPT_RSAKEY_BLOB* pBlob = (BCRYPT_RSAKEY_BLOB*)blob.data();
    pBlob->Magic = BCRYPT_RSAPUBLIC_MAGIC;
    pBlob->BitLength = keySize;
    pBlob->cbPublicExp = (DWORD)e.size();
    pBlob->cbModulus = (DWORD)n.size();
    memcpy(blob.data() + sizeof(BCRYPT_RSAKEY_BLOB), e.data(), e.size());
    memcpy(blob.data() + sizeof(BCRYPT_RSAKEY_BLOB) + e.size(), n.data(), n.size());

    BCRYPT_KEY_HANDLE hKey = nullptr;
    status = BCryptImportKeyPair(hAlg, nullptr, BCRYPT_RSAPUBLIC_BLOB, &hKey, blob.data(), cbBlob, 0);
    BCryptCloseAlgorithmProvider(hAlg, 0);
    if (status != 0) return "";

    DWORD cbResult = 0;
    status = BCryptEncrypt(hKey, (PUCHAR)plain.data(), (ULONG)plain.size(), nullptr, nullptr, 0, nullptr, 0, &cbResult, BCRYPT_PAD_PKCS1);
    if (status != 0) { BCryptDestroyKey(hKey); return ""; }
    std::vector<UCHAR> out(cbResult);
    status = BCryptEncrypt(hKey, (PUCHAR)plain.data(), (ULONG)plain.size(), nullptr, nullptr, 0, out.data(), cbResult, &cbResult, BCRYPT_PAD_PKCS1);
    BCryptDestroyKey(hKey);
    if (status != 0) return "";

    return base64_encode(out.data(), cbResult);
}

static std::string rsa_decrypt(const std::string& b64, const std::string& key_b64) {
    std::string keyDer = base64_decode(key_b64);
    if (keyDer.empty()) return "";
    std::vector<UCHAR> n, e, d, p, q, exp1, exp2, coeff;
    bool ok = false;
    if (keyDer.size() > 26 && (unsigned char)keyDer[0] == 0x30) {
        ok = parse_pkcs8_rsa((const unsigned char*)keyDer.data(), keyDer.size(), n, e, d, &p, &q, &exp1, &exp2, &coeff);
        if (!ok) ok = parse_pkcs1_rsa_private((const unsigned char*)keyDer.data(), keyDer.size(), n, e, d, &p, &q, &exp1, &exp2, &coeff);
    }
    if (!ok || p.empty() || q.empty()) return "";

    BCRYPT_ALG_HANDLE hAlg = nullptr;
    NTSTATUS status = BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_RSA_ALGORITHM, nullptr, 0);
    if (status != 0) return "";

    DWORD keySize = (DWORD)(n.size() * 8);
    DWORD cbBlob = sizeof(BCRYPT_RSAKEY_BLOB) + (DWORD)(e.size() + n.size() + p.size() + q.size() + exp1.size() + exp2.size() + coeff.size() + d.size());
    std::vector<UCHAR> blob(cbBlob);
    BCRYPT_RSAKEY_BLOB* pBlob = (BCRYPT_RSAKEY_BLOB*)blob.data();
    pBlob->Magic = BCRYPT_RSAPRIVATE_MAGIC;
    pBlob->BitLength = keySize;
    pBlob->cbPublicExp = (DWORD)e.size();
    pBlob->cbModulus = (DWORD)n.size();
    pBlob->cbPrime1 = (DWORD)p.size();
    pBlob->cbPrime2 = (DWORD)q.size();
    size_t off = sizeof(BCRYPT_RSAKEY_BLOB);
    memcpy(blob.data() + off, e.data(), e.size()); off += e.size();
    memcpy(blob.data() + off, n.data(), n.size()); off += n.size();
    memcpy(blob.data() + off, p.data(), p.size()); off += p.size();
    memcpy(blob.data() + off, q.data(), q.size()); off += q.size();
    memcpy(blob.data() + off, exp1.data(), exp1.size()); off += exp1.size();
    memcpy(blob.data() + off, exp2.data(), exp2.size()); off += exp2.size();
    memcpy(blob.data() + off, coeff.data(), coeff.size()); off += coeff.size();
    memcpy(blob.data() + off, d.data(), d.size());

    BCRYPT_KEY_HANDLE hKey = nullptr;
    status = BCryptImportKeyPair(hAlg, nullptr, BCRYPT_RSAPRIVATE_BLOB, &hKey, blob.data(), cbBlob, 0);
    BCryptCloseAlgorithmProvider(hAlg, 0);
    if (status != 0) return "";

    std::string ct = base64_decode(b64);
    if (ct.empty()) { BCryptDestroyKey(hKey); return ""; }
    DWORD cbResult = 0;
    status = BCryptDecrypt(hKey, (PUCHAR)ct.data(), (ULONG)ct.size(), nullptr, nullptr, 0, nullptr, 0, &cbResult, BCRYPT_PAD_PKCS1);
    if (status != 0) { BCryptDestroyKey(hKey); return ""; }
    std::vector<UCHAR> out(cbResult);
    status = BCryptDecrypt(hKey, (PUCHAR)ct.data(), (ULONG)ct.size(), nullptr, nullptr, 0, out.data(), cbResult, &cbResult, BCRYPT_PAD_PKCS1);
    BCryptDestroyKey(hKey);
    if (status != 0) return "";

    return std::string((char*)out.data(), cbResult);
}

static std::string http_post(const std::string& url, const std::string& post_data) {
    std::string host, path;
    int port = 80;
    bool use_ssl = (url.find("https://") == 0);
    if (use_ssl) port = 443;

    size_t start = url.find("://");
    if (start != std::string::npos) start += 3;
    else start = 0;
    size_t path_start = url.find('/', start);
    if (path_start != std::string::npos) {
        host = url.substr(start, path_start - start);
        path = url.substr(path_start);
    } else {
        host = url.substr(start);
        path = "/";
    }
    size_t port_colon = host.find(':');
    if (port_colon != std::string::npos) {
        port = std::stoi(host.substr(port_colon + 1));
        host = host.substr(0, port_colon);
    }

    std::wstring whost(host.begin(), host.end());
    std::wstring wpath(path.begin(), path.end());

    HINTERNET hSession = WinHttpOpen(L"BsPhpCpp/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) return "";
    HINTERNET hConnect = WinHttpConnect(hSession, whost.c_str(), port, 0);
    if (!hConnect) { WinHttpCloseHandle(hSession); return ""; }
    DWORD flags = use_ssl ? WINHTTP_FLAG_SECURE : 0;
    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"POST", wpath.c_str(), nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, flags);
    if (!hRequest) { WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession); return ""; }

    std::string body = "parameter=" + post_data;
    std::wstring wheaders = L"Content-Type: application/x-www-form-urlencoded\r\n";
    BOOL ok = WinHttpSendRequest(hRequest, wheaders.c_str(), (DWORD)wheaders.size(), (LPVOID)body.c_str(), (DWORD)body.size(), (DWORD)body.size(), 0);
    if (!ok) { WinHttpCloseHandle(hRequest); WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession); return ""; }
    ok = WinHttpReceiveResponse(hRequest, nullptr);
    if (!ok) { WinHttpCloseHandle(hRequest); WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession); return ""; }

    std::string response;
    do {
        DWORD len = 0;
        WinHttpQueryDataAvailable(hRequest, &len);
        if (len == 0) break;
        std::vector<char> buf(len);
        DWORD read = 0;
        WinHttpReadData(hRequest, buf.data(), len, &read);
        if (read > 0) response.append(buf.data(), read);
    } while (true);

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
    return response;
}

std::string send_data(const std::string& url, const std::string& data,
    const std::string& client_public_key, const std::string& server_private_key) {
    try {
        std::string time_str = get_time_str();
        std::string time_md5 = md5(time_str);
        std::string aes_key_full = md5(server_private_key + time_md5);
        std::string aes_key = aes_key_full.substr(0, 16);

        DebugLogUtf8("[api] APIdata: ", data);

      



        std::string encrypted_b64 = aes_cbc_encrypt(data, aes_key);
        if (encrypted_b64.empty()) return "";

        std::string sig_md5 = md5(encrypted_b64);
        std::string signature_content = "0|AES-128-CBC|" + aes_key + "|" + sig_md5 + "|xml";

        std::string rsa_b64 = rsa_encrypt(signature_content, client_public_key);
        if (rsa_b64.empty()) return "";

        std::string request_payload = encrypted_b64 + "|" + rsa_b64;
        std::string request_encoded = url_encode(request_payload);

        DebugLogUtf8("[api] APIRequest_encoded: ", request_encoded);

        std::string response_body = http_post(url, request_encoded);
        if (response_body.empty()) return "";
        DebugLogUtf8("[api] APIresponse_body: ", response_body);

        std::string raw = url_decode(response_body);
        size_t pipe = raw.find('|');
        if (pipe == std::string::npos) return "";

        std::string resp_enc_b64;
        std::string resp_rsa_b64;

        // 兼容服务端前缀：OKxxx|enc|rsa
        size_t second_pipe = raw.find('|', pipe + 1);
        std::string first_part = raw.substr(0, pipe);
        bool has_ok_prefix = first_part.size() >= 2 &&
            std::tolower(static_cast<unsigned char>(first_part[0])) == 'o' &&
            std::tolower(static_cast<unsigned char>(first_part[1])) == 'k';

        if (has_ok_prefix && second_pipe != std::string::npos) {
            resp_enc_b64 = raw.substr(pipe + 1, second_pipe - pipe - 1);
            resp_rsa_b64 = raw.substr(second_pipe + 1);
        }
        else {
            resp_enc_b64 = first_part;
            resp_rsa_b64 = raw.substr(pipe + 1);
        }

        std::string sig_dec = rsa_decrypt(resp_rsa_b64, server_private_key);
        if (sig_dec.empty()) return "";

        size_t p1 = sig_dec.find('|');
        if (p1 == std::string::npos) return "";
        size_t p2 = sig_dec.find('|', p1 + 1);
        if (p2 == std::string::npos) return "";
        size_t p3 = sig_dec.find('|', p2 + 1);
        if (p3 == std::string::npos) return "";
        std::string resp_aes_key_full = sig_dec.substr(p2 + 1, p3 - p2 - 1);

        std::string decrypted = aes_cbc_decrypt(resp_enc_b64, resp_aes_key_full);
        if (decrypted.empty()) return "";

        DebugLogUtf8("[api] APIdecrypted: ", decrypted);

        return decrypted;
    } catch (...) {
        return "";
    }
}

}  // namespace bsphp