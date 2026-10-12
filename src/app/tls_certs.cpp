#include "tls_certs.hpp"
#include "esp_log.h"
#include "mbedtls/pk.h"
#include "mbedtls/x509_crt.h"
#include "psa/crypto.h"
#include <cstring>
#include <ctime>

static const char* TAG = "TlsCerts";

namespace app::tls_certs {

bool valid_pair(const std::string& certificate_pem, const std::string& private_key_pem)
{
    if (certificate_pem.empty() || private_key_pem.empty()) return false;
    mbedtls_x509_crt certificate;
    mbedtls_pk_context private_key;
    mbedtls_x509_crt_init(&certificate);
    mbedtls_pk_init(&private_key);
    const int cert_result = mbedtls_x509_crt_parse(
        &certificate, reinterpret_cast<const unsigned char*>(certificate_pem.c_str()),
        certificate_pem.size() + 1);
    const int key_result = mbedtls_pk_parse_key(
        &private_key, reinterpret_cast<const unsigned char*>(private_key_pem.c_str()),
        private_key_pem.size() + 1, nullptr, 0);
    const bool valid = cert_result == 0 && key_result == 0 &&
        mbedtls_pk_check_pair(&certificate.pk, &private_key) == 0;
    mbedtls_pk_free(&private_key);
    mbedtls_x509_crt_free(&certificate);
    return valid;
}

static int64_t days_from_civil(int y, int m, int d)
{
    y -= m <= 2;
    const int era = (y >= 0 ? y : y - 399) / 400;
    const int yoe = y - era * 400;
    const int doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
    const int doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return static_cast<int64_t>(era) * 146097 + doe - 719468;
}

bool certificate_info(const std::string& certificate_pem, int64_t& not_after_epoch,
                      std::string& subject_cn, bool& staging_issuer)
{
    if (certificate_pem.empty()) return false;
    mbedtls_x509_crt certificate;
    mbedtls_x509_crt_init(&certificate);
    bool ok = mbedtls_x509_crt_parse(
        &certificate, reinterpret_cast<const unsigned char*>(certificate_pem.c_str()),
        certificate_pem.size() + 1) == 0;
    if (ok) {
        const mbedtls_x509_time& t = certificate.valid_to;
        not_after_epoch = days_from_civil(t.year, t.mon, t.day) * 86400 + t.hour * 3600 + t.min * 60 + t.sec;
        char buffer[160]{};
        subject_cn.clear();
        if (mbedtls_x509_dn_gets(buffer, sizeof(buffer), &certificate.subject) > 0) {
            const char* cn = strstr(buffer, "CN=");
            if (cn) {
                subject_cn = cn + 3;
                const size_t comma = subject_cn.find(',');
                if (comma != std::string::npos) subject_cn.resize(comma);
            }
        }
        std::memset(buffer, 0, sizeof(buffer));
        staging_issuer = mbedtls_x509_dn_gets(buffer, sizeof(buffer), &certificate.issuer) > 0 &&
                         strstr(buffer, "STAGING") != nullptr;
    }
    mbedtls_x509_crt_free(&certificate);
    return ok;
}

bool generate_self_signed(const std::string& fqdn, std::string& certificate_pem,
                          std::string& private_key_pem)
{
    mbedtls_pk_context key;
    mbedtls_x509write_cert writer;
    mbedtls_pk_init(&key);
    mbedtls_x509write_crt_init(&writer);

    int result = psa_crypto_init() == PSA_SUCCESS ? 0 : -1;
    mbedtls_svc_key_id_t key_id = MBEDTLS_SVC_KEY_ID_INIT;
    if (result == 0) {
        psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;
        psa_set_key_type(&attributes, PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1));
        psa_set_key_bits(&attributes, 256);
        psa_set_key_usage_flags(&attributes, PSA_KEY_USAGE_SIGN_HASH | PSA_KEY_USAGE_EXPORT);
        psa_set_key_algorithm(&attributes, PSA_ALG_ECDSA(PSA_ALG_SHA_256));
        if (psa_generate_key(&attributes, &key_id) != PSA_SUCCESS) result = -2;
    }
    if (result == 0) result = mbedtls_pk_copy_from_psa(key_id, &key);
    if (key_id != MBEDTLS_SVC_KEY_ID_INIT) psa_destroy_key(key_id);

    unsigned char serial[8]{};
    if (result == 0 && psa_generate_random(serial, sizeof(serial)) != PSA_SUCCESS) result = -3;
    serial[0] &= 0x7f;
    serial[0] |= 0x01;

    const std::string subject = "CN=" + fqdn;
    mbedtls_x509write_crt_set_version(&writer, MBEDTLS_X509_CRT_VERSION_3);
    if (result == 0) result = mbedtls_x509write_crt_set_serial_raw(&writer, serial, sizeof(serial));
    if (result == 0) result = mbedtls_x509write_crt_set_subject_name(&writer, subject.c_str());
    if (result == 0) result = mbedtls_x509write_crt_set_issuer_name(&writer, subject.c_str());
    mbedtls_x509write_crt_set_subject_key(&writer, &key);
    mbedtls_x509write_crt_set_issuer_key(&writer, &key);
    mbedtls_x509write_crt_set_md_alg(&writer, MBEDTLS_MD_SHA256);
    if (result == 0) result = mbedtls_x509write_crt_set_basic_constraints(&writer, 0, -1);
    if (result == 0) result = mbedtls_x509write_crt_set_key_usage(&writer,
        MBEDTLS_X509_KU_DIGITAL_SIGNATURE | MBEDTLS_X509_KU_KEY_ENCIPHERMENT);

    mbedtls_x509_san_list san{};
    san.node.type = MBEDTLS_X509_SAN_DNS_NAME;
    san.node.san.unstructured_name.tag = MBEDTLS_ASN1_IA5_STRING;
    san.node.san.unstructured_name.p =
        reinterpret_cast<unsigned char*>(const_cast<char*>(fqdn.c_str()));
    san.node.san.unstructured_name.len = fqdn.size();
    san.next = nullptr;
    if (result == 0) result = mbedtls_x509write_crt_set_subject_alternative_name(&writer, &san);

    // Without a synchronized clock a fixed past start date keeps the certificate usable.
    time_t now = time(nullptr);
    if (now < 1700000000) now = 1735689600;
    struct tm not_before_tm{}, not_after_tm{};
    gmtime_r(&now, &not_before_tm);
    now += 5LL * 365LL * 24LL * 60LL * 60LL;
    gmtime_r(&now, &not_after_tm);
    char not_before[16]{}, not_after[16]{};
    strftime(not_before, sizeof(not_before), "%Y%m%d%H%M%S", &not_before_tm);
    strftime(not_after, sizeof(not_after), "%Y%m%d%H%M%S", &not_after_tm);
    if (result == 0) result = mbedtls_x509write_crt_set_validity(&writer, not_before, not_after);

    unsigned char certificate_buffer[2048]{};
    unsigned char key_buffer[1024]{};
    if (result == 0) result = mbedtls_x509write_crt_pem(&writer, certificate_buffer,
        sizeof(certificate_buffer));
    if (result == 0) result = mbedtls_pk_write_key_pem(&key, key_buffer, sizeof(key_buffer));
    if (result == 0) {
        certificate_pem = reinterpret_cast<char*>(certificate_buffer);
        private_key_pem = reinterpret_cast<char*>(key_buffer);
    } else {
        ESP_LOGE(TAG, "self-signed certificate generation failed: %d", result);
    }

    mbedtls_x509write_crt_free(&writer);
    mbedtls_pk_free(&key);
    return result == 0;
}

} // namespace app::tls_certs
