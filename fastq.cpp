#include <iostream>
#include <fstream>
//#include <ofstream>
// #include <string>
// #include <cstddef>
// #include <stdexcept>
// #include <algorithm>
// #include <tuple>
// #include <typeinfo>

#include "cxxopts.hpp"
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/filter/bzip2.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filter/lzma.hpp>
#include <boost/iostreams/filter/zlib.hpp>

int run_pe(cxxopts::ParseResult result)
{
    return 0;
}

int run_se(cxxopts::ParseResult result)
{
    boost::filesystem::path fastq_path = boost::filesystem::canonical(result["fastq"].as<std::string>());
    boost::filesystem::path fastq_filename = fastq_path.filename();
    std::string fastq_extention = boost::filesystem::extension(fastq_filename.string());

    std::ifstream fastq_in;
    boost::iostreams::filtering_streambuf<boost::iostreams::input> in;
    std::cout << "fastq_extention == " << fastq_extention << std::endl;

    std::ofstream fastq_out;
    boost::iostreams::filtering_streambuf<boost::iostreams::output> out;

    if (fastq_extention == ".gz") {
        fastq_in.open(result["fastq"].as<std::string>(), std::ios_base::in | std::ios_base::binary);
        in.push(boost::iostreams::gzip_decompressor());

        fastq_out.open(fastq_filename.string(), std::ios_base::out | std::ios_base::binary);
        out.push(boost::iostreams::gzip_compressor());
    }
    else if (fastq_extention == ".bz2") {
        fastq_in.open(result["fastq"].as<std::string>(), std::ios_base::in | std::ios_base::binary);
        in.push(boost::iostreams::bzip2_decompressor());

        fastq_out.open(fastq_filename.string(), std::ios_base::out | std::ios_base::binary);
        out.push(boost::iostreams::bzip2_compressor());
    }
    else if (fastq_extention == ".z" || fastq_extention == ".Z") {
        fastq_in.open(result["fastq"].as<std::string>(), std::ios_base::in | std::ios_base::binary);
        in.push(boost::iostreams::zlib_decompressor());

        fastq_out.open(fastq_filename.string(), std::ios_base::out | std::ios_base::binary);
        out.push(boost::iostreams::zlib_compressor());
    } else {
        fastq_in.open(result["fastq"].as<std::string>(), std::ios_base::in);

        fastq_out.open(fastq_filename.string(), std::ios_base::out);
    }

    in.push(fastq_in);
    std::istream in_stream(&in);
    out.push(fastq_out);
    std::ostream out_stream(&out);

    std::string line1;
    std::string line2;
    std::string line3;
    std::string line4;

    std::getline(in_stream, line1);
    while(in_stream.good()) {
        std::getline(in_stream, line2);
        std::getline(in_stream, line3);
        std::getline(in_stream, line4);

        bool write_record = true;
        std::vector<std::string> name_vector;            
        boost::split(name_vector, line1, [](char c){return c == ' ';});

        if (name_vector.size() == 2) {
            std::vector<std::string> rfcs_vector;
            boost::split(rfcs_vector, name_vector.back(), [](char c){return c == ':';});
            if ((std::string)rfcs_vector.at(1) == "Y") {
                write_record = false;
            }
        }
        if (write_record) {
            out_stream << line1+'\n'
                       << line2+'\n'
                       << line3+'\n'
                       << line4+'\n';
        }
        std::getline(in_stream, line1);
    }
    boost::iostreams::close(in);
    fastq_in.close();

    out_stream.flush();
    fastq_out.flush();
    boost::iostreams::close(out);
    fastq_out.close();

    return 0;
}

bool get_input_paths_valid(cxxopts::ParseResult result)
{
    using namespace boost::filesystem;
    path cwd = current_path();

    // fastq
    bool fastq_exists = exists(result["fastq"].as<std::string>());
    if (!fastq_exists) {
        std::cerr << "input fastq does not exist at specified path:\n\t"
                  << result["fastq"].as<std::string>() << std::endl;
        return false;
    }

    path fastq_path = system_complete(result["fastq"].as<std::string>());
    if (equivalent(fastq_path.parent_path(), cwd)) {
        std::cerr << "input fastq should not be located in cwd\n"
                  << "\tcwd: " << cwd
                  << "\n\tfastq absolute path: " << fastq_path << std::endl;
        return false;
    }

    // fastq2
    if (result.count("fastq2")) {
        bool fastq2_exists = exists(result["fastq2"].as<std::string>());
        if (!fastq2_exists) {
            std::cerr << "input fastq2 does not exist at specified path:\n\t"
                      << result["fastq2"].as<std::string>() << std::endl;
            return false;
        }

        path fastq2_path = system_complete(result["fastq2"].as<std::string>());
        if (equivalent(fastq2_path.parent_path(), cwd)) {
            std::cerr << "input fastq2 should not be located in cwd\n"
                      << "\tcwd: " << cwd
                      << "\n\tfastq2 absolute path: " << fastq2_path << std::endl;
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
        std::cout << "main else" << std::endl;
    }

    std::cout << "main return 0" << std::endl;
    return 0;
}
