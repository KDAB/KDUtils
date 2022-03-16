#ifndef KDUTILS_FILE_H
#define KDUTILS_FILE_H

#include <KDUtils/kdutils_export.h>
#include <KDUtils/bytearray.h>
#include <iostream>
#include <fstream>
#include <filesystem>

namespace KDUtils {

class KDUTILS_EXPORT File
{
public:
    File(const std::string &path);
    ~File();

    // Can't be copied
    File(File &) = delete;
    File &operator=(File &) = delete;

    bool exists() const;

    bool open(std::ios_base::openmode mode);
    bool isOpen() const;
    void flush();
    void close();
    bool remove();

    ByteArray readAll();
    void write(const ByteArray &data);
    std::string fileName() const;
    const std::string &path() const;

    std::uintmax_t size() const;

private:
    std::string m_path;
    std::fstream m_stream;
};

} // namespace KDUtils

#endif // KUESA_COREUTILS_FILE_H
