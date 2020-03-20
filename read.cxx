#include "RAWDataHeader.h"
#include <array>
#include <chrono>
#include <cstdio>
#include <fmt/format.h>
#include <iostream>
#include <string>

size_t test1(std::string filename)
{
    FILE* f = fopen(filename.c_str(), "r");
    o2::header::RAWDataHeaderV4 rdh;
    std::array<std::byte, 8192> dummy;
    size_t nbytes { 0 };

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

size_t test2(std::string filename)
{
    FILE* f = fopen(filename.c_str(), "r");
    o2::header::RAWDataHeaderV4 rdh;
    std::array<std::byte, 8192> dummy;
    size_t nbytes { 0 };
    uint64_t posInFile { 0 };

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

size_t test3(std::string filename)
{
    FILE* f = fopen(filename.c_str(), "r");
    o2::header::RAWDataHeaderV4 rdh;
    std::array<std::byte, 8192> dummy;
    size_t nbytes { 0 };
    uint64_t posInFile { 0 };

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

constexpr const char* BIGFILENAME = "bigfile.txt";

void wipeCache()
{
    // trying to wipe the SSD read cache
    system(fmt::format("dd if={} of=/dev/null bs=1024", BIGFILENAME).c_str());
}

void clock(std::string name, std::function<size_t(std::string)> func, std::string filename)
{
    wipeCache();
    for (auto n = 0; n < 1; n++) {
        auto start = std::chrono::system_clock::now();
        auto nread = func(filename);
        auto end = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        std::cout << fmt::format("{:10s} : {:10d} bytes scanned in {:7.2f} seconds\n",
            name, nread, elapsed_seconds.count());
    }
}

int main(int argc, char** argv)
{
    system(fmt::format("dd if=/dev/zero of={} count=10485760 bs=1024", BIGFILENAME).c_str()); // create a big file

    std::string filename = argv[1];
    std::cout << filename << "\n";

    clock("test1 read all", test3, filename);

    std::cout << "---------------\n";

    clock("test2 seek_set", test1, filename);

    std::cout << "---------------\n";

    clock("test3 seek_cur", test2, filename);

    std::cout << "---------------\n";

    clock("test1 read all", test1, filename);
    return 0;
}
