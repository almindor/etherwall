#ifndef CERT_H
#define CERT_H

#include <QString>

namespace Etherwall {

    // self signed cert for main server-info api, no need to use a public one here
    static const QString EtherWall_Cert = "-----BEGIN CERTIFICATE-----\n"
            "MIIDiDCCAnACCQCXJXqGOlAorjANBgkqhkiG9w0BAQsFADCBhTELMAkGA1UEBhMC\n"
            "Q0ExEDAOBgNVBAgMB0FsYmVydGExEDAOBgNVBAcMB0NhbGdhcnkxEjAQBgNVBAoM\n"
            "CUV0aGVyZHluZTEbMBkGA1UEAwwSZGF0YS5ldGhlcndhbGwuY29tMSEwHwYJKoZI\n"
            "hvcNAQkBFhJhbG1pbmRvckBnbWFpbC5jb20wHhcNMTYwODI4MjM1MjI5WhcNMjIw\n"
            "MjE4MjM1MjI5WjCBhTELMAkGA1UEBhMCQ0ExEDAOBgNVBAgMB0FsYmVydGExEDAO\n"
            "BgNVBAcMB0NhbGdhcnkxEjAQBgNVBAoMCUV0aGVyZHluZTEbMBkGA1UEAwwSZGF0\n"
            "YS5ldGhlcndhbGwuY29tMSEwHwYJKoZIhvcNAQkBFhJhbG1pbmRvckBnbWFpbC5j\n"
            "b20wggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQCsraaryUowJKQSQLeV\n"
            "4r833OvTrI5ylyww1s3m+aN0eGx2tQa+rVIeklu9vO0bz5qWc6CnWvOx1ZA35Ru+\n"
            "OXkYzUe63Bkt0MtBDvfhGQct3vJ5r+6dXf8TuPgSaqBCpGCG4DtIYUxuNVauJ2N7\n"
            "HX/KOOY5J2qHlWpas2TNrncbvfc55A8ezP2p3dwmbyITG7IPhwicDDez6xPz0SXH\n"
            "USsv3Y3bQlVAh+11tquGcKo14QbRtPIkmaEHuG1nN4LaFhQL7P2gH8x7g2lOmsE+\n"
            "JIDAdHV2ExewAfIf4Ep+twPo6jXf96cKLhtKCfVlUKUBEvBxsFmEFlwciW1R7d7R\n"
            "TXNBAgMBAAEwDQYJKoZIhvcNAQELBQADggEBAIMtS+szahT3ScKAsk3Y1F1Ac/E3\n"
            "t9JJZhEGS4b73vlna2Rl0wQd2rPtVMgdTarshhH/jAngWaf52xzMd0MKlYEkzRdx\n"
            "D1tSdwqDxQ+XKxMBXwiN2ffgc+r8xcFWK34BU32MC7reVL1sQtjYAiGfSuo4ZZqk\n"
            "0WThQ183ERexxwtYQ8qSn3L+kXCPJyVnazt7IJ3rylB9e6t6voaU/eNQUC7Mdwov\n"
            "Vw6Ar9fz+sQVccQQDREICKnnK1M+k8kk+g3c+rF3ISFlLPi981tWjGSTH685HH0q\n"
            "JkX2TxeYmZl+B/qvVorfPzWK7NoalCBvIxyxBeI3e67Ly0lRWAGIsWEtQP4=\n"
            "-----END CERTIFICATE-----\n";
}

#endif // CERT_H
