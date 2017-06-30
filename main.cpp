// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <iostream>
#include <string>
#include <cctype>
#include <fstream>
#include <algorithm>
#include <stdexcept>
#include <map>
#include <vector>
#include <stdlib.h>

static const std::string DomainSpecials(".-");
static const std::string PathSpecials(".,/+_");

using Dictionary = std::map<std::string, int>;

struct Statistics
{
    Statistics() : totalUrls(0) {}

    int totalUrls;
    Dictionary domains;
    Dictionary paths;
};

struct Config
{
    int topN;
    const char* inputFileName;
    const char* outputFileName;
};

struct TopRecord
{
    TopRecord(const std::string& text, int count)
        : text(text), count(count) {}

    std::string text;
    int count;
};

using TopData = std::vector<TopRecord>;

void usage()
{
    std::cout << "Usage:" << std::endl;
    std::cout << "logparser [-n NNN] <input file> [<output file>]" << std::endl;
    exit(1);
}

void parseArgs(int argc, char* argv[], Config& config)
{
    if (argc < 3 || argc > 5)
        usage();

    config.topN = 5;

    int argNo = 1;

    if (!std::string(argv[argNo]).compare("-n"))
    {
        argNo++;
        config.topN = std::atoi(argv[argNo++]);
    }

    config.inputFileName = argv[argNo++];
    config.outputFileName = argv[argNo++];
}

// Ищет url, в случае успеха возвращает индекс на байт после "http://" или "https://",
// иначе возвращает std::string::npos
size_t findUrl(const std::string& data, size_t start)
{
    while((data.size() - start) > 7) // strlen("http://")
    {
        size_t pos = data.find("http", start);
        if (pos == std::string::npos)
            break;

        pos += 4;
        if (data[pos] == 's')
            pos++;

        if (!data.compare(pos, 3, "://"))
        {
            pos += 3;
            // проверяем есть ли еще данные в буфере после ://
            if (data.size() > pos)
                return pos;
        }

        start = pos;
    }

    return std::string::npos;
}

bool isTokenSymbol(char c, const std::string& specials)
{
    return std::isalnum(c) || (specials.find(c) != std::string::npos);
}

size_t getToken(const std::string& data, size_t start, const std::string& specials, std::string& token)
{
    size_t pos = start;
    while((pos < data.size()) && isTokenSymbol(data[pos], specials))
        pos++;

    token = data.substr(start, pos - start);
    return pos;
}

void processLine(const std::string& data, Statistics& stat)
{
    size_t pos = 0;
    while((pos = findUrl(data, pos)) != std::string::npos)
    {
        std::string domain, path;

        size_t domainEnd = getToken(data, pos, DomainSpecials, domain);
        pos = getToken(data, domainEnd, PathSpecials, path);
        if (path.empty())
            path = "/";

        stat.totalUrls++;
        stat.domains[domain]++;
        stat.paths[path]++;
    }
}

void printTop(const TopData& top, int topN, std::ostream& out)
{
    int count = 0;
    for(auto i = std::begin(top); (i != std::end(top)) && (count < topN) ; ++i, ++count)
        out << i->count << ' ' << i->text << std::endl;
}

TopData buildTop(const Dictionary& dict)
{
    TopData top;
    for(const auto& i : dict)
        top.emplace_back(i.first, i.second);

    std::sort(std::begin(top), std::end(top), [](const TopRecord& a, const TopRecord& b){
        if (a.count == b.count)
            return a.text < b.text;
        return a.count > b.count;
    });

    return top;
}

void printStatistics(const Statistics& stat, int topN, std::ostream& out)
{
    out << "total urls " << stat.totalUrls
        << ", domains " << stat.domains.size()
        << ", paths " << stat.paths.size() << std::endl << std::endl;

    out << "top domains" << std::endl;
    printTop(buildTop(stat.domains), topN, out);

    out << std::endl << "top paths" << std::endl;
    printTop(buildTop(stat.paths), topN, out);
}

int main(int argc, char* argv[])
{
    Config config;
    parseArgs(argc, argv, config);

    std::ifstream inDataStream(config.inputFileName);
    if (!inDataStream.good())
        throw std::runtime_error("File not found");

    Statistics stat;
    std::string dataBuffer;
    do {
        std::getline(inDataStream, dataBuffer);
        processLine(dataBuffer, stat);
    } while(inDataStream.good());

    std::ofstream outDataStream(config.outputFileName);
    printStatistics(stat, config.topN, outDataStream);

    return 0;
}
