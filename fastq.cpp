#include <iostream>
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
#include <boost/filesystem.hpp>
#include <boost/iostreams/filter/zlib.hpp>

int run_pe(cxxopts::ParseResult result)
{
    return 0;
}

int run_se(cxxopts::ParseResult result)
{
    std::ofstream fastq_out;
    fastq_out.open("example.fastq");
    
    // std::ifstream fastq_file {result["fastq"].as<std::string>(), std::ios::binary};
    const auto sequence  = bioio::read_fastq(result["fastq"].as<std::string>());

    for (auto it = begin(sequence); it != end(sequence); ++it) {
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

bool get_input_paths_valid(cxxopts::ParseResult result)
{
    using namespace boost::filesystem;
    path fastq_path = absolute(result["fastq"].as<std::string>());
    path cwd = current_path();
    if (fastq_path.parent_path() == cwd) {
        std::cerr << "input fastq should not be located in cwd\n"
                  << "\tcwd: " << cwd
                  << "\n\tfastq absolute path: " << fastq_path << std::endl;
        return false;
    }
    if (result.count("fastq2")) {
        path fastq2_path = absolute(result["fastq"].as<std::string>());
        if (fastq2_path.parent_path() == cwd) {
            std::cerr << "input fastq2 should not be located in cwd\n"
                      << "\tcwd: " << cwd
                      << "\n\tfastq2 absolute path: " << fastq2_path << std::endl;
            return false;
        }
    }

    bool fastq_exists = exists(fastq_path);
    if (!fastq_exists) {
        std::cerr << "input fastq does not exist at specified path:\n\t"
                  << fastq_path << std::endl;
        return false;
    }

    if (result.count("fastq2")) {
        bool fastq2_exists = exists(fastq2_path);
        if (!fastq2_exists) {
            std::cerr << "input fastq2 does not exist at specified path:\n\t"
                      << fastq2_path << std::endl;
            return false;
        }
    }
    return true;
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
    bool input_paths_valid = get_input_paths_valid(result);
    if (!input_paths_valid) {
        return false;
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
