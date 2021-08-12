#ifndef KDCORE_URL_H
#define KDCORE_URL_H

#include <KDCore/kdcore_export.h>
#include <string>

namespace KDCore {

class KDCORE_EXPORT Url
{
public:
    Url() = default;
    explicit Url(const std::string &url);

    bool isLocalFile() const;
    std::string toLocalFile() const;
    std::string scheme() const;
    std::string fileName() const;
    std::string path() const;
    std::string url() const;
    bool empty() const { return m_url.empty(); }
    bool isEmpty() const { return empty(); }

    static Url fromLocalFile(const std::string &url);

private:
    std::string m_url;
    std::string m_fileName;
    std::string m_scheme;
    std::string m_path;
};

KDCORE_EXPORT bool operator==(const Url &a, const Url &b);
KDCORE_EXPORT bool operator!=(const Url &a, const Url &b);

} // namespace KDCore

#endif // KUESA_COREUTILS_URL_H
