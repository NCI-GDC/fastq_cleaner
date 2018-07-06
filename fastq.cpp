// #include <iostream>
// #include <fstream>
// #include <string>
// #include <cstddef>
// #include <stdexcept>
// #include <algorithm>
// #include <tuple>
// #include <typeinfo>

#include "bioio.hpp"
#include "cxxopts.hpp"
#include <boost/algorithm/string.hpp>
#include <boost/iostreams/filter/zlib.hpp>

int run_pe(cxxopts::ParseResult result)
{
    std::ofstream fastq_out;
    fastq_out.open("example.fastq");
    
    // std::ifstream fastq_file {result["fastq"].as<std::string>(), std::ios::binary};
    const auto sequence  = bioio::read_fastq(result["fastq"].as<std::string>());

    for (auto it = begin (sequence); it != end(sequence); ++it) {
        if (it->name.size() > 0) {
            bool write_record = true;
            std::vector<std::string> name_vector;            
            boost::split(name_vector, it->name, [](char c){return c == ' ';});
            
            if (name_vector.size() == 2) {
                std::vector<std::string> rfcs_vector;            
                boost::split(rfcs_vector, name_vector.back(), [](char c){return c == ':';});
                if ((std::string)rfcs_vector.at(1) == "Y") {
                    write_record = false;
                }
            }
            if (write_record) {
                fastq_out << it->name << '\n'
                          << it->seq << '\n'
                          << it->thirdline << '\n'
                          << it->qual << '\n';
            }
        }
    }
    fastq_out.close();
    return 0;
}

int run_se(cxxopts::ParseResult result)
{
    return 0;
}

int get_options_valid(cxxopts::ParseResult result)
{
    if (result.count("fastq") != 1) {
        std::cerr << "--fastq option must be specified once" << std::endl;
        return false;
    }
    if (result.count("fastq2")) {
        if (result.count("fastq2") != 1) {
            std::cerr << "--fastq2 option can only be specified once" << std::endl;
            return false;
        }
    }
    return true;
}

int main(int argc, char **argv)
{
    cxxopts::Options options("Fastq Cleaner", "Removes qcfail, and duplicate qnames");
    options.add_options()
      ("fastq", "single fastq, or first of pair", cxxopts::value<std::string>())
      ("fastq2", "second of pair (optional)", cxxopts::value<std::string>())
      ;
    cxxopts::ParseResult result = options.parse(argc, argv);
    bool options_valid = get_options_valid(result);
    if (!options_valid) {
        std::cerr << "Please use correct parameters" << std::endl;
        return 1;
    }


    if (result.count("fastq2") == 1) {
        run_pe(result);
    } else {
        run_se(result);
    }

    return 0;
}
