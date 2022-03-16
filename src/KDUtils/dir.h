#ifndef KDUTILS_DIR_H
#define KDUTILS_DIR_H

#include <KDUtils/kdutils_export.h>
#include <string>
#include <filesystem>

namespace KDUtils {

class KDUTILS_EXPORT Dir
{
public:
    Dir();
    Dir(const char *path);
    Dir(const std::string &path);
    Dir(const std::filesystem::path &path);

    bool exists() const;
    bool mkdir();
    bool rmdir();
    const std::string &path() const;
    std::string dirName() const;
    std::string absoluteFilePath(const std::string &file) const;

    static Dir applicationDir();

    bool operator==(const Dir &other) const;

private:
    std::string m_path;
};

} // namespace KDUtils

#endif // KUESA_COREUTILS_DIR_H
