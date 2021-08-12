#include "dir.h"
#include <filesystem>

#include <whereami.h>

#include <iostream>

namespace KDCore {

Dir::Dir()
{
}

Dir::Dir(const char *path)
    : m_path(path)
{
}

Dir::Dir(const std::string &path)
    : m_path(path)
{
}

Dir::Dir(const std::filesystem::path &path)
    : m_path(path.string())
{
}

bool Dir::exists() const
{
    std::error_code e;
    const std::filesystem::path dPath{ m_path };
    return std::filesystem::exists(dPath, e) && std::filesystem::is_directory(m_path, e);
}

bool Dir::mkdir()
{
    std::error_code e;
    const std::filesystem::path dPath{ m_path };
    return std::filesystem::create_directory(dPath, e);
}

bool Dir::rmdir()
{
    std::error_code e;
    const std::filesystem::path dPath{ m_path };
    return std::filesystem::remove_all(dPath, e) > 0;
}

std::string Dir::path() const
{
    return m_path;
}

std::string Dir::dirName() const
{
    const std::filesystem::path p{ m_path };
    return p.parent_path().filename().u8string();
}

std::string Dir::absoluteFilePath(const std::string &file) const
{
    std::error_code e;
    const std::filesystem::path dPath{ m_path };
    const std::filesystem::path fPath{ file };
    return std::filesystem::absolute(dPath / fPath, e).u8string();
}

Dir Dir::applicationDir()
{
    const int length = wai_getExecutablePath(NULL, 0, NULL);
    if (length >= 0) {
        std::string appPath;
        appPath.resize(length);
        wai_getExecutablePath(appPath.data(), length, NULL);

        const std::filesystem::path appFSPath(appPath);
        return Dir(appFSPath.parent_path().append(""));
    }

    return {};
}

bool Dir::operator==(const Dir &other) const
{
    return std::filesystem::path(m_path) == std::filesystem::path(other.m_path);
}

} // namespace KDCore
