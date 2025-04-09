/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDNetwork/ssl_socket.h>
#include <KDUtils/logging.h>

// Include OpenSSL headers
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/pem.h>
#include <openssl/bio.h>
#include <openssl/x509_vfy.h>

#include <iomanip>
#include <mutex>
#include <sstream>

#if defined(KD_PLATFORM_WIN32)
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

namespace {
// Global OpenSSL initialization
class OpenSslInitializer
{
public:
    OpenSslInitializer()
    {
        OPENSSL_init_ssl(OPENSSL_INIT_LOAD_SSL_STRINGS | OPENSSL_INIT_LOAD_CRYPTO_STRINGS, nullptr);
    }

    ~OpenSslInitializer()
    {
        // Modern versions of OpenSSL handle cleanup automatically
    }
};

// Ensure OpenSSL is initialized once
static std::once_flag opensslInitFlag;
static OpenSslInitializer *opensslInit = nullptr;

void ensureOpenSslInitialized()
{
    std::call_once(opensslInitFlag, []() {
        opensslInit = new OpenSslInitializer();
    });
}

// Helper to convert OpenSSL errors to readable strings
std::string getOpenSslErrorString()
{
    std::string result;
    unsigned long err;

    while ((err = ERR_get_error()) != 0) {
        char buf[256];
        ERR_error_string_n(err, buf, sizeof(buf));
        if (!result.empty())
            result += "; ";
        result += buf;
    }

    return result.empty() ? "Unknown SSL error" : result;
}

// Convert X509 certificate to PEM format string
std::string x509ToPem(X509 *cert)
{
    BIO *bio = BIO_new(BIO_s_mem());
    if (!bio)
        return {};

    PEM_write_bio_X509(bio, cert);

    char *data = nullptr;
    long length = BIO_get_mem_data(bio, &data);
    std::string result(data, length);

    BIO_free(bio);
    return result;
}

// Parse and format X509 certificate information for debugging
std::string formatCertificateDetails(X509 *cert)
{
    if (!cert) {
        return "No certificate";
    }

    std::stringstream ss;

    // Get subject
    char subjectName[256];
    X509_NAME_oneline(X509_get_subject_name(cert), subjectName, sizeof(subjectName));
    ss << "Subject: " << subjectName << "\n";

    // Get issuer
    char issuerName[256];
    X509_NAME_oneline(X509_get_issuer_name(cert), issuerName, sizeof(issuerName));
    ss << "Issuer: " << issuerName << "\n";

    // Get validity period
    ASN1_TIME *notBefore = X509_get_notBefore(cert);
    ASN1_TIME *notAfter = X509_get_notAfter(cert);

    BIO *bio = BIO_new(BIO_s_mem());

    if (bio) {
        ss << "Valid from: ";
        ASN1_TIME_print(bio, notBefore);
        char *dateStr;
        long dateLen = BIO_get_mem_data(bio, &dateStr);
        ss << std::string(dateStr, dateLen) << " to ";

        BIO_reset(bio);
        ASN1_TIME_print(bio, notAfter);
        dateLen = BIO_get_mem_data(bio, &dateStr);
        ss << std::string(dateStr, dateLen) << "\n";

        BIO_free(bio);
    }

    // Get fingerprint
    unsigned char md[EVP_MAX_MD_SIZE];
    unsigned int md_len;
    if (X509_digest(cert, EVP_sha256(), md, &md_len)) {
        ss << "SHA-256 fingerprint: ";
        for (unsigned int i = 0; i < md_len; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(md[i]);
            if (i < md_len - 1)
                ss << ":";
        }
        ss << std::dec << "\n";
    }

    // Get Subject Alternative Names (SANs)
    GENERAL_NAMES *sans = static_cast<GENERAL_NAMES *>(
            X509_get_ext_d2i(cert, NID_subject_alt_name, nullptr, nullptr));

    if (sans) {
        ss << "Subject Alternative Names:\n";
        int num_sans = sk_GENERAL_NAME_num(sans);

        for (int i = 0; i < num_sans; i++) {
            GENERAL_NAME *current = sk_GENERAL_NAME_value(sans, i);

            if (current->type == GEN_DNS) {
                ASN1_STRING *dns = current->d.dNSName;
                ss << "  DNS: " << std::string((char *)ASN1_STRING_get0_data(dns), ASN1_STRING_length(dns)) << "\n";
            } else if (current->type == GEN_IPADD) {
                ASN1_STRING *ip = current->d.iPAddress;

                if (ip->length == 4) { // IPv4
                    const unsigned char *ipData = ASN1_STRING_get0_data(ip);
                    ss << "  IP: " << static_cast<int>(ipData[0]) << "." << static_cast<int>(ipData[1]) << "."
                       << static_cast<int>(ipData[2]) << "." << static_cast<int>(ipData[3]) << "\n";
                }
                // Could add IPv6 handling if needed
            }
        }

        GENERAL_NAMES_free(sans);
    }

    return ss.str();
}

} // anonymous namespace

namespace KDNetwork {

class SslSocket::SslSocketPrivate
{
public:
    SslSocketPrivate()
        : ctx(nullptr)
        , ssl(nullptr)
        , networkBio(nullptr)
        , verificationMode(SslSocket::VerificationMode::VerifyPeer)
        , handshakeComplete(false)
        , verificationSucceeded(false)
        , readBufferEncrypted(4096) // Initial buffer size
    {
    }

    ~SslSocketPrivate()
    {
        cleanup();
    }

    void cleanup()
    {
        if (ssl) {
            SSL_free(ssl); // This will free the internal BIO
            ssl = nullptr;
        }

        // The network BIO is freed explicitly since it's not attached to the SSL object
        if (networkBio) {
            BIO_free(networkBio);
            networkBio = nullptr;
        }

        if (ctx) {
            SSL_CTX_free(ctx);
            ctx = nullptr;
        }

        handshakeComplete = false;
        verificationSucceeded = false;
    }

    SSL_CTX *ctx;
    SSL *ssl;
    BIO *networkBio;
    VerificationMode verificationMode;
    std::string peerVerifyHostname;
    bool handshakeComplete;
    bool verificationSucceeded;
    std::string verificationError;

    // Buffer for encrypted data read from socket
    std::vector<uint8_t> readBufferEncrypted;

    // To be written through SSL
    KDUtils::ByteArray pendingWriteBuffer;

    // For client certificates
    std::vector<uint8_t> clientCertData;
    std::vector<uint8_t> clientKeyData;
    std::string clientKeyPassword;
};

SslSocket::SslSocket()
    : TcpSocket()
    , d(std::make_unique<SslSocketPrivate>())
{
    m_type = SocketType::SslTcp;
    ensureOpenSslInitialized();
    initSsl();
}

SslSocket::~SslSocket()
{
    disconnectFromHost();
}

SslSocket::SslSocket(SslSocket &&other) noexcept
    : TcpSocket(std::move(other))
    , handshakeCompleted(std::move(other.handshakeCompleted))
    , handshakeError(std::move(other.handshakeError))
    , d(std::move(other.d))
{
    // All members moved
}

SslSocket &SslSocket::operator=(SslSocket &&other) noexcept
{
    if (this != &other) {
        TcpSocket::operator=(std::move(other));
        handshakeCompleted = std::move(other.handshakeCompleted);
        handshakeError = std::move(other.handshakeError);
        d = std::move(other.d);
    }
    return *this;
}

bool SslSocket::initSsl()
{
    // Clean up any existing SSL context
    if (d->ctx) {
        SSL_CTX_free(d->ctx);
        d->ctx = nullptr;
    }

    // Create a new SSL context
    d->ctx = SSL_CTX_new(TLS_client_method());
    if (!d->ctx) {
        KDUtils::Logger::logger("KDNetwork")->error("Failed to create SSL context: " + getOpenSslErrorString());
        setError(SocketError::SslError);
        return false;
    }

    // Set modern security options
    SSL_CTX_set_min_proto_version(d->ctx, TLS1_2_VERSION);
    SSL_CTX_set_options(d->ctx, SSL_OP_NO_COMPRESSION);

    // Set default verification options
    SSL_CTX_set_verify(d->ctx, SSL_VERIFY_PEER, nullptr);

// Use system default CA certificates
#if defined(KD_PLATFORM_WIN32)
    if (SSL_CTX_load_verify_store(d->ctx, "org.openssl.winstore://") != 1) {
        KDUtils::Logger::logger("KDNetwork")->warn("Failed to load Windows certificate store");
    }
#else
    if (SSL_CTX_set_default_verify_paths(d->ctx) != 1) {
        KDUtils::Logger::logger("KDNetwork")->warn("Failed to set default verify paths");
    }
#endif

    return true;
}

bool SslSocket::connectToHost(const std::string &host, std::uint16_t port)
{
    // Store the hostname for certificate verification
    if (d->peerVerifyHostname.empty()) {
        d->peerVerifyHostname = host;
    }

    // Perform TCP connection first
    return TcpSocket::connectToHost(host, port);
}

bool SslSocket::connectToHost(const IpAddress &address, std::uint16_t port)
{
    // Perform TCP connection first
    return TcpSocket::connectToHost(address, port);
}

void SslSocket::disconnectFromHost()
{
    // If we have an SSL connection, attempt a clean shutdown
    if (d->ssl && d->handshakeComplete) {
        // Non-blocking SSL_shutdown
        int ret = SSL_shutdown(d->ssl);
        if (ret == 0) {
            // First stage of shutdown completed, ideally wait for second stage
            // but we'll complete immediately for simplicity
        }
    }

    // Clean up SSL resources
    d->cleanup();

    // Call base class to close the TCP socket
    TcpSocket::disconnectFromHost();
}

void SslSocket::setVerificationMode(SslSocket::VerificationMode mode)
{
    d->verificationMode = mode;

    if (!d->ctx) {
        return;
    }

    int verifyMode = SSL_VERIFY_NONE;
    switch (mode) {
    case VerificationMode::VerifyNone:
        verifyMode = SSL_VERIFY_NONE;
        break;
    case VerificationMode::VerifyPeer:
        verifyMode = SSL_VERIFY_PEER;
        break;
    case VerificationMode::VerifyPeerIfPresent:
        verifyMode = SSL_VERIFY_PEER;
        break;
    }

    SSL_CTX_set_verify(d->ctx, verifyMode, nullptr);
}

SslSocket::VerificationMode SslSocket::verificationMode() const
{
    return d->verificationMode;
}

void SslSocket::setPeerVerifyName(const std::string &hostName)
{
    d->peerVerifyHostname = hostName;
}

std::string SslSocket::peerVerifyName() const
{
    return d->peerVerifyHostname;
}

bool SslSocket::addCaCertificate(const std::vector<uint8_t> &certData)
{
    if (!d->ctx || certData.empty()) {
        return false;
    }

    // Create a BIO for reading the cert
    BIO *certBio = BIO_new_mem_buf(certData.data(), static_cast<int>(certData.size()));
    if (!certBio) {
        return false;
    }

    // Read cert into X509 structure
    X509 *cert = PEM_read_bio_X509(certBio, nullptr, nullptr, nullptr);
    BIO_free(certBio);

    if (!cert) {
        return false;
    }

    // Get the store from context
    X509_STORE *store = SSL_CTX_get_cert_store(d->ctx);
    if (!store) {
        X509_free(cert);
        return false;
    }

    // Add the certificate
    int result = X509_STORE_add_cert(store, cert);
    X509_free(cert);

    return result == 1;
}

bool SslSocket::setClientCertificate(const std::vector<uint8_t> &certData,
                                     const std::vector<uint8_t> &keyData,
                                     const std::string &keyPassword)
{
    if (!d->ctx || certData.empty() || keyData.empty()) {
        return false;
    }

    // Store for later use
    d->clientCertData = certData;
    d->clientKeyData = keyData;
    d->clientKeyPassword = keyPassword;

    // Create BIOs for reading
    BIO *certBio = BIO_new_mem_buf(certData.data(), static_cast<int>(certData.size()));
    BIO *keyBio = BIO_new_mem_buf(keyData.data(), static_cast<int>(keyData.size()));

    if (!certBio || !keyBio) {
        if (certBio)
            BIO_free(certBio);
        if (keyBio)
            BIO_free(keyBio);
        return false;
    }

    // Read cert and key
    X509 *cert = PEM_read_bio_X509(certBio, nullptr, nullptr, nullptr);
    EVP_PKEY *key = nullptr;

    if (!keyPassword.empty()) {
        key = PEM_read_bio_PrivateKey(keyBio, nullptr, nullptr,
                                      const_cast<char *>(keyPassword.c_str()));
    } else {
        key = PEM_read_bio_PrivateKey(keyBio, nullptr, nullptr, nullptr);
    }

    // Free BIOs
    BIO_free(certBio);
    BIO_free(keyBio);

    if (!cert || !key) {
        if (cert)
            X509_free(cert);
        if (key)
            EVP_PKEY_free(key);
        return false;
    }

    // Set the certificate and key
    int result = SSL_CTX_use_certificate(d->ctx, cert) &&
            SSL_CTX_use_PrivateKey(d->ctx, key) &&
            SSL_CTX_check_private_key(d->ctx);

    // Free resources
    X509_free(cert);
    EVP_PKEY_free(key);

    return result == 1;
}

bool SslSocket::peerVerificationSucceeded() const
{
    return d->verificationSucceeded;
}

std::string SslSocket::peerVerificationErrorString() const
{
    return d->verificationError;
}

std::vector<std::string> SslSocket::peerCertificateChain() const
{
    std::vector<std::string> chain;
    if (!d->ssl || !d->handshakeComplete)
        return chain;

    STACK_OF(X509) *peerCerts = SSL_get_peer_cert_chain(d->ssl);
    if (!peerCerts)
        return chain;

    for (int i = 0; i < sk_X509_num(peerCerts); ++i) {
        X509 *cert = sk_X509_value(peerCerts, i);
        if (cert) {
            chain.push_back(x509ToPem(cert));
        }
    }

    return chain;
}

void SslSocket::onReadReady()
{
    // Check if we're still in TCP connection phase
    if (state() == State::Connecting) {
        // Let TcpSocket handle the initial connection
        TcpSocket::onReadReady();

        // If TCP connected, start SSL handshake
        if (state() == State::Connected && !d->handshakeComplete) {
            startHandshake();
        }
        return;
    }

    // If connected but SSL handshake not complete
    if (state() == State::Connected && !d->handshakeComplete) {
        continueHandshake();
        return;
    }

    // Normal read operation after handshake
    if (state() == State::Connected && d->handshakeComplete) {
        handleSslRead();
        return;
    }

    // Otherwise, let the base class handle it
    TcpSocket::onReadReady();
}

void SslSocket::onWriteReady()
{
    // Check if we're still in TCP connection phase
    if (state() == State::Connecting) {
        // Let TcpSocket handle the initial connection
        TcpSocket::onWriteReady();

        // If TCP connected, start SSL handshake
        if (state() == State::Connected && !d->handshakeComplete) {
            startHandshake();
        }
        return;
    }

    // If connected but SSL handshake not complete
    if (state() == State::Connected && !d->handshakeComplete) {
        continueHandshake();
        return;
    }

    // Normal write operation after handshake
    if (state() == State::Connected && d->handshakeComplete) {
        // Try sending pending data
        handleSslWrite();
        return;
    }

    // Otherwise, let the base class handle it
    TcpSocket::onWriteReady();
}

bool SslSocket::startHandshake()
{
    // Make sure we have a TCP connection first
    if (state() != State::Connected) {
        return false;
    }

    // Clean up any previous SSL state
    if (d->ssl) {
        SSL_free(d->ssl);
        d->ssl = nullptr;
    }

    // Create new SSL connection object
    d->ssl = SSL_new(d->ctx);
    if (!d->ssl) {
        KDUtils::Logger::logger("KDNetwork")->error("Failed to create SSL object: " + getOpenSslErrorString());
        setError(SocketError::SslError);
        disconnectFromHost();
        return false;
    }

    // If we have a verification hostname, set it for SNI
    if (!d->peerVerifyHostname.empty()) {
        SSL_set_tlsext_host_name(d->ssl, d->peerVerifyHostname.c_str());
        // Set verification hostname
        X509_VERIFY_PARAM *param = SSL_get0_param(d->ssl);
        X509_VERIFY_PARAM_set_hostflags(param, X509_CHECK_FLAG_NO_PARTIAL_WILDCARDS);
        X509_VERIFY_PARAM_set1_host(param, d->peerVerifyHostname.c_str(), d->peerVerifyHostname.size());
    }

    // Create a BIO pair for handling SSL I/O
    BIO *internal_bio, *network_bio;
    if (BIO_new_bio_pair(&internal_bio, 0, &network_bio, 0) != 1) {
        KDUtils::Logger::logger("KDNetwork")->error("Failed to create BIO pair: " + getOpenSslErrorString());
        SSL_free(d->ssl);
        d->ssl = nullptr;
        setError(SocketError::SslError);
        disconnectFromHost();
        return false;
    }

    // Connect the internal BIO to the SSL object
    SSL_set_bio(d->ssl, internal_bio, internal_bio);

    // Store the network BIO for later use - we need to free this in cleanup
    d->networkBio = network_bio;

    // Set as client
    SSL_set_connect_state(d->ssl);

    // Start handshake
    continueHandshake();
    return true;
}

void SslSocket::continueHandshake()
{
    if (!d->ssl || d->handshakeComplete) {
        return;
    }

    // Try to read any available data before proceeding with handshake
    handleSslRead();

    int result = SSL_do_handshake(d->ssl);
    int sslError = SSL_get_error(d->ssl, result);

    // Always try to flush encrypted handshake data
    flushNetworkBIO();

    if (sslError == SSL_ERROR_NONE) {
        // Handshake complete
        KDUtils::Logger::logger("KDNetwork")->info("SSL handshake completed successfully");
        d->handshakeComplete = true;

        // Verify certification if required
        if (d->verificationMode != VerificationMode::VerifyNone) {
            if (!verifySslCertificate()) {
                std::string error = "Certificate verification failed: " + d->verificationError;
                KDUtils::Logger::logger("KDNetwork")->error(error);
                setError(SocketError::SslCertificateError);
                handshakeError.emit(error);
                disconnectFromHost();
                return;
            }
        }

        // Handshake successful
        d->verificationSucceeded = true;

        // Log successful connection details
        KDUtils::Logger::logger("KDNetwork")->debug("SSL connection established: " + sslConnectionInfo());

        handshakeCompleted.emit();

        // Handle any pending writes
        if (!d->pendingWriteBuffer.isEmpty()) {
            handleSslWrite();
        }

        // Check for pending reads
        handleSslRead();
    } else if (sslError == SSL_ERROR_WANT_READ) {
        // Need more data, waiting for read event
        KDUtils::Logger::logger("KDNetwork")->trace("SSL handshake waiting for read");
        setReadNotificationEnabled(true);
        setWriteNotificationEnabled(false);
    } else if (sslError == SSL_ERROR_WANT_WRITE) {
        // Need to write, waiting for write event
        KDUtils::Logger::logger("KDNetwork")->trace("SSL handshake waiting for write");
        setReadNotificationEnabled(false);
        setWriteNotificationEnabled(true);
    } else {
        // Handshake failed
        std::string errorMsg = getOpenSslErrorString();

        // Get certificate details if available, even if verification failed
        X509 *cert = SSL_get_peer_certificate(d->ssl);
        if (cert) {
            KDUtils::Logger::logger("KDNetwork")->debug("Server certificate details:\n" + formatCertificateDetails(cert));
            X509_free(cert);
        } else {
            KDUtils::Logger::logger("KDNetwork")->debug("No server certificate received");
        }

        std::string error = "SSL handshake failed: " + errorMsg;
        KDUtils::Logger::logger("KDNetwork")->error(error);
        setError(SocketError::SslError);
        handshakeError.emit(error);
        disconnectFromHost();
    }
}

void SslSocket::handleSslRead()
{
    if (!d->ssl || !d->networkBio) {
        return;
    }

    // Read encrypted data directly from socket
    d->readBufferEncrypted.resize(4096); // Reasonable buffer size

    ssize_t bytesRead = 0;
#if defined(KD_PLATFORM_WIN32)
    bytesRead = ::recv(m_socketFd, reinterpret_cast<char *>(d->readBufferEncrypted.data()),
                       d->readBufferEncrypted.size(), 0);
#else
    bytesRead = ::read(m_socketFd, d->readBufferEncrypted.data(), d->readBufferEncrypted.size());
#endif

    if (bytesRead > 0) {
        // Write the encrypted data to the network BIO
        int written = BIO_write(d->networkBio, d->readBufferEncrypted.data(), bytesRead);

        if (written <= 0) {
            // Error writing to BIO
            if (!BIO_should_retry(d->networkBio)) {
                KDUtils::Logger::logger("KDNetwork")->error("Error writing to network BIO: " + getOpenSslErrorString());
                setError(SocketError::SslError);
                disconnectFromHost();
                return;
            }
            // Otherwise, it's a temporary error, we can try again later
            return;
        }

        // If handshake is not complete, return now - the data is in the BIO for SSL to use
        // during the handshake process in continueHandshake()
        if (!d->handshakeComplete)
            return;

        // Try to decrypt and process data
        while (true) {
            char buffer[4096];
            int decrypted = SSL_read(d->ssl, buffer, sizeof(buffer));

            if (decrypted > 0) {
                // Process decrypted data - add to TcpSocket's read buffer
                KDUtils::ByteArray decryptedData(reinterpret_cast<const uint8_t *>(buffer), decrypted);
                TcpSocket::processReceivedData(decryptedData.constData(), decryptedData.size());
            } else {
                int sslError = SSL_get_error(d->ssl, decrypted);
                if (sslError == SSL_ERROR_WANT_READ) {
                    // Need more data
                    break;
                } else if (sslError == SSL_ERROR_ZERO_RETURN) {
                    // Clean SSL shutdown
                    disconnectFromHost();
                    break;
                } else if (sslError == SSL_ERROR_SYSCALL) {
                    // I/O error
                    if (decrypted == 0) {
                        // EOF
                        disconnectFromHost();
                    } else {
                        KDUtils::Logger::logger("KDNetwork")->error("SSL read error: " + getOpenSslErrorString());
                        setError(SocketError::ReadError);
                        disconnectFromHost();
                    }
                    break;
                } else {
                    // Other SSL error
                    KDUtils::Logger::logger("KDNetwork")->error("SSL read error: " + getOpenSslErrorString());
                    setError(SocketError::SslError);
                    disconnectFromHost();
                    break;
                }
            }
        }
    } else if (bytesRead == 0) {
        // EOF
        disconnectFromHost();
    } else {
        // Error
#if defined(KD_PLATFORM_WIN32)
        int error_code = WSAGetLastError();
        if (error_code == WSAEWOULDBLOCK) {
            // No data available, normal for non-blocking socket
            return;
        }
        setError(SocketError::ReadError, error_code);
#else
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // No data available, normal for non-blocking socket
            return;
        }
        setError(SocketError::ReadError, errno);
#endif
        disconnectFromHost();
    }
}

bool SslSocket::handleSslWrite()
{
    if (!d->ssl || !d->handshakeComplete || !d->networkBio) {
        return false;
    }

    // Check if we have data to write
    if (d->pendingWriteBuffer.isEmpty() && TcpSocket::bytesToWrite() == 0) {
        setWriteNotificationEnabled(false);
        return true;
    }

    // First, try to write any data in the pendingWriteBuffer through SSL
    while (!d->pendingWriteBuffer.isEmpty()) {
        int written = SSL_write(d->ssl, d->pendingWriteBuffer.constData(), d->pendingWriteBuffer.size());

        if (written > 0) {
            // Successfully encrypted some data
            int bytesWritten = written;
            d->pendingWriteBuffer.remove(0, bytesWritten);

            // Now flush the network BIO to get the encrypted data
            flushNetworkBIO();

            // Emit signal for successful write
            TcpSocket::bytesWritten.emit(bytesWritten);
        } else {
            int sslError = SSL_get_error(d->ssl, written);
            if (sslError == SSL_ERROR_WANT_WRITE) {
                // Need to flush the network BIO first
                flushNetworkBIO();
                setWriteNotificationEnabled(true);
                return false;
            } else if (sslError == SSL_ERROR_WANT_READ) {
                // Renegotiation or similar, need to read first
                setReadNotificationEnabled(true);
                return false;
            } else {
                // Fatal error
                KDUtils::Logger::logger("KDNetwork")->error("SSL write error: " + getOpenSslErrorString());
                setError(SocketError::WriteError);
                disconnectFromHost();
                return false;
            }
        }
    }

    // Try to flush any remaining data in the network BIO
    flushNetworkBIO();

    // If we've written all pending data and the network BIO is empty, disable write notification
    if (d->pendingWriteBuffer.isEmpty()) {
        setWriteNotificationEnabled(false);
    }

    return true;
}

// Helper method to flush data from the network BIO to the socket
void SslSocket::flushNetworkBIO()
{
    if (!d->networkBio) {
        return;
    }

    char buffer[4096];
    int pending = BIO_pending(d->networkBio);

    while (pending > 0) {
        int readSize = std::min(pending, static_cast<int>(sizeof(buffer)));
        int read = BIO_read(d->networkBio, buffer, readSize);

        if (read <= 0) {
            // No more data or error
            if (!BIO_should_retry(d->networkBio)) {
                KDUtils::Logger::logger("KDNetwork")->error("Error reading from network BIO: " + getOpenSslErrorString());
                setError(SocketError::SslError);
                disconnectFromHost();
            }
            break;
        }

        // Write the encrypted data to the socket
#if defined(KD_PLATFORM_WIN32)
        int sent = ::send(m_socketFd, buffer, read, 0);
#else
        int sent = ::send(m_socketFd, buffer, read, MSG_NOSIGNAL);
#endif

        if (sent <= 0) {
#if defined(KD_PLATFORM_WIN32)
            int error_code = WSAGetLastError();
            if (error_code == WSAEWOULDBLOCK) {
                // Would block, retry later
                // Put the data back into the BIO for later
                BIO_write(d->networkBio, buffer, read);
                setWriteNotificationEnabled(true);
                break;
            }
            setError(SocketError::WriteError, error_code);
#else
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // Would block, retry later
                // Put the data back into the BIO for later
                BIO_write(d->networkBio, buffer, read);
                setWriteNotificationEnabled(true);
                break;
            }
            setError(SocketError::WriteError, errno);
#endif
            disconnectFromHost();
            break;
        }

        // Update pending data count
        pending = BIO_pending(d->networkBio);
    }
}

std::int64_t SslSocket::write(const KDUtils::ByteArray &data)
{
    return write(data.constData(), static_cast<std::int64_t>(data.size()));
}

std::int64_t SslSocket::write(const std::uint8_t *data, std::int64_t size)
{
    // Can only write if connected and handshake complete
    if (state() != State::Connected || !d->handshakeComplete) {
        if (state() == State::Connected && !d->handshakeComplete) {
            // If connected but handshake not complete, queue the data
            d->pendingWriteBuffer.append(data, size);
            return size; // Accept the data, but queue it
        }
        setError(SocketError::WriteError);
        return -1;
    }

    if (!data || size <= 0) {
        return 0; // Nothing to write
    }

    // Try to write directly through SSL
    int bytesWritten = SSL_write(d->ssl, data, static_cast<int>(size));

    if (bytesWritten > 0) {
        // Successful write, flush the network BIO to send the actual encrypted data
        flushNetworkBIO();

        // Emit the signal
        TcpSocket::bytesWritten.emit(bytesWritten);
        return bytesWritten;
    } else {
        int sslError = SSL_get_error(d->ssl, bytesWritten);
        if (sslError == SSL_ERROR_WANT_WRITE || sslError == SSL_ERROR_WANT_READ) {
            // Connection would block, queue the data
            d->pendingWriteBuffer.append(data, size);

            // Enable appropriate notification
            if (sslError == SSL_ERROR_WANT_WRITE) {
                flushNetworkBIO(); // Try to flush any pending data
                setWriteNotificationEnabled(true);
            } else {
                setReadNotificationEnabled(true);
            }

            return size; // Accept all data for writing
        } else {
            // Fatal error
            KDUtils::Logger::logger("KDNetwork")->error("SSL write error: " + getOpenSslErrorString());
            setError(SocketError::WriteError);
            return -1;
        }
    }
}

bool SslSocket::verifySslCertificate()
{
    if (!d->ssl) {
        d->verificationError = "No SSL connection established";
        return false;
    }

    X509 *cert = SSL_get_peer_certificate(d->ssl);
    if (!cert) {
        // No certificate provided by peer
        if (d->verificationMode == VerificationMode::VerifyPeer) {
            d->verificationError = "Peer did not provide a certificate";
            return false;
        }
        // For VerifyPeerIfPresent, this is okay
        return true;
    }

    // Log certificate details for debugging
    KDUtils::Logger::logger("KDNetwork")->debug("Server certificate details:\n" + formatCertificateDetails(cert));

    // Check verification result
    long verifyResult = SSL_get_verify_result(d->ssl);
    if (verifyResult != X509_V_OK) {
        d->verificationError = X509_verify_cert_error_string(verifyResult);

        // Add more detail to the error message
        if (verifyResult == X509_V_ERR_HOSTNAME_MISMATCH) {
            d->verificationError += " (Expected hostname: " + d->peerVerifyHostname + ")";

            // Extract the Common Name (CN) from the certificate for comparison
            X509_NAME *subject = X509_get_subject_name(cert);
            if (subject) {
                char commonName[256];
                if (X509_NAME_get_text_by_NID(subject, NID_commonName, commonName, sizeof(commonName)) > 0) {
                    d->verificationError += " (Certificate CN: " + std::string(commonName) + ")";
                }
            }
        } else if (verifyResult == X509_V_ERR_CERT_NOT_YET_VALID || verifyResult == X509_V_ERR_CERT_HAS_EXPIRED) {
            BIO *bio = BIO_new(BIO_s_mem());
            if (bio) {
                ASN1_TIME *notBefore = X509_get_notBefore(cert);
                ASN1_TIME *notAfter = X509_get_notAfter(cert);

                char *dateStr;
                d->verificationError += " (Valid from: ";
                ASN1_TIME_print(bio, notBefore);
                long dateLen = BIO_get_mem_data(bio, &dateStr);
                d->verificationError += std::string(dateStr, dateLen);

                BIO_reset(bio);
                d->verificationError += " to ";
                ASN1_TIME_print(bio, notAfter);
                dateLen = BIO_get_mem_data(bio, &dateStr);
                d->verificationError += std::string(dateStr, dateLen) + ")";

                BIO_free(bio);
            }
        } else if (verifyResult == X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN ||
                   verifyResult == X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT ||
                   verifyResult == X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY) {
            // Get the issuer name to help identify missing CA certificates
            X509_NAME *issuer = X509_get_issuer_name(cert);
            if (issuer) {
                char issuerName[256];
                X509_NAME_oneline(issuer, issuerName, sizeof(issuerName));
                d->verificationError += " (Certificate issuer: " + std::string(issuerName) + ")";
            }
        }

        // Log the detailed error
        KDUtils::Logger::logger("KDNetwork")->error("Certificate verification failed: " + d->verificationError);

        X509_free(cert);
        return false;
    }

    // Check hostname against certificate if we have a hostname for verification
    if (!d->peerVerifyHostname.empty()) {
        // Hostname verification should be automatic via SSL_get_verify_result when using X509_VERIFY_PARAM_set1_host,
        // but let's log the verification details for clarity
        KDUtils::Logger::logger("KDNetwork")->debug("Verified certificate for hostname: " + d->peerVerifyHostname);
    }

    X509_free(cert);
    return true;
}

std::string SslSocket::sslVersion() const
{
    if (d->ssl && d->handshakeComplete) {
        return SSL_get_version(d->ssl);
    }
    return {};
}

std::string SslSocket::sslCipher() const
{
    if (d->ssl && d->handshakeComplete) {
        const SSL_CIPHER *cipher = SSL_get_current_cipher(d->ssl);
        if (cipher) {
            char buf[128];
            SSL_CIPHER_description(cipher, buf, sizeof(buf));
            return buf;
        }
    }
    return {};
}

// Add a new method to provide connection info for debugging
std::string SslSocket::sslConnectionInfo() const
{
    if (!d->ssl || !d->handshakeComplete)
        return "No SSL connection";

    std::stringstream ss;

    // Protocol version
    ss << "Protocol: " << SSL_get_version(d->ssl) << ", ";

    // Cipher
    const SSL_CIPHER *cipher = SSL_get_current_cipher(d->ssl);
    if (cipher) {
        ss << "Cipher: " << SSL_CIPHER_get_name(cipher) << " ("
           << SSL_CIPHER_get_bits(cipher, nullptr) << " bits), ";
    }

// Compression (usually disabled in modern SSL/TLS)
#if OPENSSL_VERSION_NUMBER < 0x10100000L
    const COMP_METHOD *comp = SSL_get_current_compression(d->ssl);
    ss << "Compression: " << (comp ? SSL_COMP_get_name(comp) : "None");
#else
    ss << "Compression: None"; // TLS 1.3 doesn't use compression
#endif

    return ss.str();
}

} // namespace KDNetwork
