#include <algorithm>
#include <iostream>
#include <iterator>
#include <fstream>
#include <future>
#include <set>
#include <thread>

#include "cxxopts.hpp"
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/filter/bzip2.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filter/zlib.hpp>

std::pair<std::set<int>, std::vector<std::string>> get_fastq_pair(std::string fastq)
{
    boost::filesystem::path fastq_path = boost::filesystem::canonical(fastq);
    boost::filesystem::path fastq_filename = fastq_path.filename();
    std::string fastq_extention = boost::filesystem::extension(fastq_filename.string());

    std::ifstream fastq_in;
    boost::iostreams::filtering_streambuf<boost::iostreams::input> in;

    if (fastq_extention == ".gz") {
        fastq_in.open(fastq, std::ios_base::in | std::ios_base::binary);
        in.push(boost::iostreams::gzip_decompressor());
    }
    else if (fastq_extention == ".bz2") {
        fastq_in.open(fastq, std::ios_base::in | std::ios_base::binary);
        in.push(boost::iostreams::bzip2_decompressor());
    }
    else if (fastq_extention == ".z" || fastq_extention == ".Z") {
        fastq_in.open(fastq, std::ios_base::in | std::ios_base::binary);
        in.push(boost::iostreams::zlib_decompressor());
    } else {
        fastq_in.open(fastq, std::ios_base::in);
    }
    in.push(fastq_in);
    std::istream in_stream(&in);

    std::set<int> filtered_set;
    std::vector<std::string> fastq_vector;
    
    std::string line1;
    std::string line2;
    std::string line3;
    std::string line4;

    std::getline(in_stream, line1);
    int filtered_index = 0;
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
                filtered_set.insert(filtered_index);
            }
        }
        fastq_vector.push_back(line1+'\n'+
                               line2+'\n'+
                               line3+'\n'+
                               line4+'\n');
        std::getline(in_stream, line1);
        filtered_index++;
    }
    boost::iostreams::close(in);
    std::pair<std::set<int>, std::vector<std::string>> pair = {filtered_set, fastq_vector};
    return pair;
}


void write_fastq(std::vector<std::string> fastq_vector, std::set<int> filtered_set, std::string fastq)
{
    boost::filesystem::path fastq_path = boost::filesystem::canonical(fastq);
    boost::filesystem::path fastq_filename = fastq_path.filename();
    std::string fastq_extention = boost::filesystem::extension(fastq_filename.string());

    std::ofstream fastq_out;
    boost::iostreams::filtering_streambuf<boost::iostreams::output> out;

    if (fastq_extention == ".gz") {
        fastq_out.open(fastq_filename.string(), std::ios_base::out | std::ios_base::binary);
        out.push(boost::iostreams::gzip_compressor());
    }
    else if (fastq_extention == ".bz2") {
        fastq_out.open(fastq_filename.string(), std::ios_base::out | std::ios_base::binary);
        out.push(boost::iostreams::bzip2_compressor());
    }
    else if (fastq_extention == ".z" || fastq_extention == ".Z") {
        fastq_out.open(fastq_filename.string(), std::ios_base::out | std::ios_base::binary);
        out.push(boost::iostreams::zlib_compressor());
    } else {
        fastq_out.open(fastq_filename.string(), std::ios_base::out);
    }
    out.push(fastq_out);
    std::ostream out_stream(&out);
    
    int current_index = 0;
    for(std::vector<std::string>::iterator it = fastq_vector.begin(); it != fastq_vector.end(); ++it) {
        std::set<int>::const_iterator iit = filtered_set.find(current_index);

        if (iit != filtered_set.end()) {
            std::cout << "current_index exist in the set: " << current_index << std::endl;
	}
        else {
            out_stream << *it;
        }
        current_index++;
    }
    out_stream.flush();
    boost::iostreams::close(out);
    return;
}

int run_pe(cxxopts::ParseResult result)
{
    std::string fastq1_string = result["fastq"].as<std::string>();
    std::string fastq2_string = result["fastq2"].as<std::string>();

    std::packaged_task< std::pair<std::set<int>, std::vector<std::string>>(std::string) > task_get_fastq1_pair(get_fastq_pair);
    std::future<std::pair<std::set<int>, std::vector<std::string>>> fut_fastq1_pair = task_get_fastq1_pair.get_future();

    std::packaged_task< std::pair<std::set<int>, std::vector<std::string>>(std::string) > task_get_fastq2_pair(get_fastq_pair);
    std::future<std::pair<std::set<int>, std::vector<std::string>>> fut_fastq2_pair = task_get_fastq2_pair.get_future();

    std::thread worker1_thread(std::move(task_get_fastq1_pair), fastq1_string);
    std::thread worker2_thread(std::move(task_get_fastq2_pair), fastq2_string);

    std::pair<std::set<int>, std::vector<std::string>> pair1 = fut_fastq1_pair.get();
    std::pair<std::set<int>, std::vector<std::string>> pair2 = fut_fastq2_pair.get();

    worker1_thread.join();
    worker2_thread.join();
    std::cout << "finished reading input fastq pair" << std::endl;

    std::set<int> filtered_set;
    std::set_union(pair1.first.begin(), pair1.first.end(), pair2.first.begin(), pair2.first.end(), std::inserter(filtered_set, filtered_set.begin()));
    std::cout << "created filter set" << std::endl;

    std::thread write_fastq1(write_fastq, pair1.second, filtered_set, fastq1_string);
    std::thread write_fastq2(write_fastq, pair2.second, filtered_set, fastq2_string);

    write_fastq1.join();
    write_fastq2.join();
    std::cout << "wrote output fastq pair" << std::endl;
    return 0;
}

int run_se(cxxopts::ParseResult result)
{
    std::string fastq_string = result["fastq"].as<std::string>();
    std::pair<std::set<int>, std::vector<std::string>> pair1 = get_fastq_pair(fastq_string);
    std::set<int> filtered_set = pair1.first;
    write_fastq(pair1.second, filtered_set, fastq_string);
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
    }
    return 0;
}
