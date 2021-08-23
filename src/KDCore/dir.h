#ifndef KDCORE_DIR_H
#define KDCORE_DIR_H

#include <KDCore/kdcore_export.h>
#include <string>
#include <filesystem>

namespace KDCore {

class KDCORE_EXPORT Dir
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

} // namespace KDCore

#endif // KUESA_COREUTILS_DIR_H
