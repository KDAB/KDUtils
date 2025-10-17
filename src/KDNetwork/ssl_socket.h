/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sean Harmer <sean.harmer@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDNetwork/tcp_socket.h>
#include <KDNetwork/kdnetwork_export.h>
#include <string>
#include <memory>
#include <vector>

// Forward declare OpenSSL types to avoid including OpenSSL headers in our public API
struct ssl_st;
struct ssl_ctx_st;
struct x509_st;
typedef struct ssl_st SSL;
typedef struct ssl_ctx_st SSL_CTX;
typedef struct x509_st X509;

namespace KDNetwork {

class KDNETWORK_EXPORT SslSocket : public TcpSocket
{
public:
    enum class VerificationMode {
        VerifyNone, // Don't verify peer certificate
        VerifyPeer, // Verify peer certificate and fail if invalid
        VerifyPeerIfPresent // Verify peer certificate if presented but don't fail if not present
    };

    SslSocket();
    virtual ~SslSocket();

    // Is not copyable
    SslSocket(const SslSocket &other) = delete;
    SslSocket &operator=(const SslSocket &other) = delete;

    // Is movable
    SslSocket(SslSocket &&other) noexcept;
    SslSocket &operator=(SslSocket &&other) noexcept;

    // Override TcpSocket connection methods to add SSL handshake
    bool connectToHost(const std::string &host, std::uint16_t port) override;
    bool connectToHost(const IpAddress &address, std::uint16_t port) override;
    void disconnectFromHost() override;

    // SSL configuration
    void setVerificationMode(VerificationMode mode);
    VerificationMode verificationMode() const;

    // Override write methods to handle SSL encryption
    std::int64_t write(const KDUtils::ByteArray &data) override;
    std::int64_t write(const std::uint8_t *data, std::int64_t size) override;

    void setPeerVerifyName(const std::string &hostName);
    std::string peerVerifyName() const;

    // Add a certificate authority for peer verification
    bool addCaCertificate(const std::vector<uint8_t> &certData);

    // Set client certificate for mutual TLS
    bool setClientCertificate(const std::vector<uint8_t> &certData, const std::vector<uint8_t> &keyData, const std::string &keyPassword = {});

    // Certificate details
    bool peerVerificationSucceeded() const;
    std::string peerVerificationErrorString() const;
    std::vector<std::string> peerCertificateChain() const; // Returns PEM formatted certificates

    KDBindings::Signal<> handshakeCompleted;
    KDBindings::Signal<const std::string &> handshakeError;

protected:
    // Override Socket notification handlers
    void onReadReady() override;
    void onWriteReady() override;

private:
    // Initialize SSL library and context
    bool initSsl();

    // Start SSL handshake after TCP connection is established
    bool startHandshake();

    // Continue SSL handshake when socket is ready for read/write
    void continueHandshake();

    // Read encrypted data from socket, decrypt and process
    void handleSslRead();

    // Encrypt and send data
    bool handleSslWrite();

    // Check verification result
    bool verifySslCertificate();

    // Get information about the current SSL connection
    std::string sslVersion() const;
    std::string sslCipher() const;

    // Returns detailed information about the current SSL connection for debugging
    std::string sslConnectionInfo() const;

    // Class implementation details
    class SslSocketPrivate;
    std::unique_ptr<SslSocketPrivate> d;

    // Helper method to flush data from the network BIO to the socket
    void flushNetworkBIO();
};

} // namespace KDNetwork
