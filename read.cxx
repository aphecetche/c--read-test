#include "RAWDataHeader.h"
#include <array>
#include <chrono>
#include <cstddef>
#include <cstdio>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>

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

void separator()
{
    std::cout << "---------------\n";
}

void createBigFile(const size_t fileInGB = 20)
{
    auto start = std::chrono::system_clock::now();
    std::cout << "Creating bigfile " << BIGFILENAME << " of size " << fileInGB << " GB ..." << std::flush;
    auto fileSize = fileInGB * 1024 * 1024; // size in bytes;
    std::stringstream cmd;
    cmd << "dd if=/dev/zero of=" << BIGFILENAME << " count=" << fileSize << " bs=1024 2> /dev/null";
    system(cmd.str().c_str());
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;
    std::cout << ". Created in " << elapsed_seconds.count() << " seconds "
              << fileInGB * 1024 / elapsed_seconds.count() << "MB/s\n";
}

void readBigFile()
{
    // trying to wipe the SSD read cache
    auto start = std::chrono::system_clock::now();
    std::stringstream cmd;
    cmd << "dd if=" << BIGFILENAME << " of=/dev/null bs=1024 2> /dev/null";
    system(cmd.str().c_str());
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;
    std::cout << "-- wipe cache : " << BIGFILENAME << " read in " << elapsed_seconds.count() << " seconds\n";
}

void wipeCache()
{
    readBigFile();
}

void clock(std::string name, std::function<size_t(std::string)> func, std::string filename)
{
    wipeCache();
    separator();

    for (auto n = 0; n < 1; n++) {
        std::cout << "Reading " << filename << "(method " << name << ")..." << std::flush;
        auto start = std::chrono::system_clock::now();
        auto nread = func(filename);
        double mb = nread / (1024 * 1024.0);
        auto end = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        std::cout << ": " << nread << " bytes scanned in " << elapsed_seconds.count() << " seconds "
                  << " " << mb / elapsed_seconds.count() << " MB/s\n";
    }
}

int main(int argc, char** argv)
{
    std::string filename = argv[1];

    size_t fileInGB = 20;

    if (argc > 2) {
        fileInGB = atoi(argv[2]);
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

    createBigFile(fileInGB);

    std::array<int, 8> order = { 0, 1, 2, 0, 2, 1, 2, 0 };

    for (auto o : order) {
        const auto& c = calls[o];
        clock(c.name, c.func, filename);
    }

    return 0;
}
