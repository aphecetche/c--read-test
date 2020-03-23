#include "RAWDataHeader.h"
#include <array>
#include <chrono>
#include <cstddef>
#include <cstdio>
#include <functional>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

class MyTimer {
public:
    MyTimer(size_t fileSizeInBytes)
        : fsize{ fileSizeInBytes }
        , start{ std::chrono::high_resolution_clock::now() }
    {
    }
    ~MyTimer()
    {
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        double mb = fsize / (1024 * 1024.0);
        std::cout << ": " << fsize << " bytes in "
                  << std::fixed << std::setw(6) << std::setprecision(1)
                  << elapsed_seconds.count() << " seconds "
                  << std::fixed << std::setw(5) << std::setprecision(0)
                  << mb / elapsed_seconds.count() << " MB/s\n";
    }

private:
    size_t fsize;
    std::chrono::high_resolution_clock::time_point start;
};

size_t readAll(std::string filename)
{
    FILE* f = fopen(filename.c_str(), "r");
    o2::header::RAWDataHeaderV4 rdh;
    std::array<std::byte, 8192> dummy;
    size_t nbytes{ 0 };

    while (fread(&rdh, 1, sizeof(rdh), f) == sizeof(rdh)) {
        size_t sizeToRead = rdh.offsetToNext - sizeof(rdh);
        auto nread = fread(&dummy[0], 1, sizeToRead, f);
        if (nread < sizeToRead) {
            break;
        }
        nbytes += nread + sizeof(rdh);
    }
    fclose(f);
    return nbytes;
}

size_t seekSet(std::string filename)
{
    FILE* f = fopen(filename.c_str(), "r");
    o2::header::RAWDataHeaderV4 rdh;
    std::array<std::byte, 8192> dummy;
    size_t nbytes{ 0 };
    uint64_t posInFile{ 0 };

    while (fread(&rdh, 1, sizeof(rdh), f) == sizeof(rdh)) {
        size_t sizeToRead = rdh.offsetToNext - sizeof(rdh);
        if (fseek(f, posInFile, SEEK_SET)) {
            break;
        }
        posInFile += rdh.offsetToNext;
    }
    posInFile = ftell(f);
    fclose(f);
    return posInFile;
}

size_t seekCur(std::string filename)
{
    FILE* f = fopen(filename.c_str(), "r");
    o2::header::RAWDataHeaderV4 rdh;
    std::array<std::byte, 8192> dummy;
    size_t nbytes{ 0 };
    uint64_t posInFile{ 0 };

    while (fread(&rdh, 1, sizeof(rdh), f) == sizeof(rdh)) {
        size_t sizeToRead = rdh.offsetToNext - sizeof(rdh);
        if (fseek(f, rdh.offsetToNext - sizeof(rdh), SEEK_CUR)) {
            break;
        }
    }
    posInFile = ftell(f);
    fclose(f);
    return posInFile;
}

constexpr const char* BIGFILENAME = "dummy-big-file.txt";

void createBigFile(const size_t fileSizeInGB = 20)
{
    std::cout << "Creating bigfile " << BIGFILENAME << " of size " << fileSizeInGB << " GB ..." << std::flush;
    std::stringstream cmd;
    cmd << "dd if=/dev/zero of=" << BIGFILENAME << " count=" << fileSizeInGB * 1024 << " bs=1048576 2> /dev/null";
    system(cmd.str().c_str());
}

void readBigFile()
{
    // trying to wipe the SSD read cache
    std::stringstream cmd;
    cmd << "dd if=" << BIGFILENAME << " of=/dev/null bs=8192 2> /dev/null";
    system(cmd.str().c_str());
}

void wipeCache(size_t fileSizeInGB)
{
    std::cout << "Reading " << BIGFILENAME << "..." << std::flush;
    MyTimer t{ fileSizeInGB * 1024 * 1024 * 1024 };
    readBigFile();
}

size_t file_size(std::string filename)
{
    struct stat s;
    stat(filename.c_str(), &s);
    return s.st_size;
}

int main(int argc, char** argv)
{
    std::string filename = argv[1];

    size_t fileSizeInGB = 20;

    if (argc > 2) {
        fileSizeInGB = atoi(argv[2]);
    }

    struct call {
        std::string name;
        std::function<size_t(std::string)> func;
    };

    std::array<call, 3> calls = {
        call{ "all", readAll },
        call{ "set", seekSet },
        call{ "cur", seekCur }
    };

    {
        MyTimer creationTime{ fileSizeInGB * 1024 * 1024 * 1024 };
        createBigFile(fileSizeInGB);
    }

    std::array<int, 8> order = { 0, 1, 2, 0, 2, 1, 2, 0 };

    auto inputFileSize = file_size(filename);

    for (auto o : order) {
        const auto& c = calls[o];
        wipeCache(fileSizeInGB);
        std::cout << "Reading " << filename << "(method " << c.name << ")..." << std::flush;
        MyTimer t{ inputFileSize };
        c.func(filename);
    }

    return 0;
}
