#include <algorithm>
#include <iostream>
#include <iterator>
#include <fstream>
#include <future>
#include <set>
#include <thread>
#include <stdlib.h>

#include "cxxopts.hpp"
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/filter/bzip2.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filter/zlib.hpp>

std::pair<std::set<int>, std::vector<std::string>> get_fastq_pair(std::istream& in_stream, unsigned long long int reads_in_memory)
{
    std::set<int> filtered_set;
    std::vector<std::string> fastq_vector;
    
    std::string line1;
    std::string line2;
    std::string line3;
    std::string line4;

    std::getline(in_stream, line1);
    unsigned long long int read_count = 0;
    while(in_stream.good()) {
        std::getline(in_stream, line2);
        std::getline(in_stream, line3);
        std::getline(in_stream, line4);

        std::vector<std::string> name_vector;            
        boost::split(name_vector, line1, [](char c){return c == ' ';});

        if (name_vector.size() == 2) {
            std::vector<std::string> rfcs_vector;
            boost::split(rfcs_vector, name_vector.back(), [](char c){return c == ':';});
            if ((std::string)rfcs_vector.at(1) == "Y") {
                filtered_set.insert(read_count);
            }
        }
        fastq_vector.push_back(line1+'\n'+
                               line2+'\n'+
                               line3+'\n'+
                               line4+'\n');
        if (read_count == reads_in_memory-1) {
            std::cout << "\t while() read_count == reads_in_memory" << std::endl;
            std::cout << "\t while() read_count == " << read_count << std::endl;
            std::cout << "\t while() reads_in_memory == " << reads_in_memory << std::endl;
            break;
        }
        std::getline(in_stream, line1);
        read_count++;
    }
    std::cout << "\tfastq_vector.size(): " << fastq_vector.size() << std::endl;
    std::pair<std::set<int>, std::vector<std::string>> pair = {filtered_set, fastq_vector};
    return pair;
}

void write_fastq(std::vector<std::string> fastq_vector, std::set<int> filtered_set, std::ostream& out_stream)
{
    
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
    return;
}

int run_pe(cxxopts::ParseResult parseresult)
{
    std::string fastq1_string = parseresult["fastq"].as<std::string>();
    std::string fastq2_string = parseresult["fastq2"].as<std::string>();

    //in fastq1
    boost::filesystem::path fastq1_path = boost::filesystem::canonical(fastq1_string);
    boost::filesystem::path fastq1_filename = fastq1_path.filename();
    std::string fastq1_extention = boost::filesystem::extension(fastq1_filename.string());

    std::ifstream fastq1_in;
    boost::iostreams::filtering_streambuf<boost::iostreams::input> in1;

    if (fastq1_extention == ".gz") {
        fastq1_in.open(fastq1_string, std::ios_base::in | std::ios_base::binary);
        in1.push(boost::iostreams::gzip_decompressor());
    }
    else if (fastq1_extention == ".bz2") {
        fastq1_in.open(fastq1_string, std::ios_base::in | std::ios_base::binary);
        in1.push(boost::iostreams::bzip2_decompressor());
    }
    else if (fastq1_extention == ".z" || fastq1_extention == ".Z") {
        fastq1_in.open(fastq1_string, std::ios_base::in | std::ios_base::binary);
        in1.push(boost::iostreams::zlib_decompressor());
    } else {
        fastq1_in.open(fastq1_string, std::ios_base::in);
    }
    in1.push(fastq1_in);
    std::istream in_stream1(&in1);
    //in fastq2
    boost::filesystem::path fastq2_path = boost::filesystem::canonical(fastq2_string);
    boost::filesystem::path fastq2_filename = fastq2_path.filename();
    std::string fastq2_extention = boost::filesystem::extension(fastq2_filename.string());

    std::ifstream fastq2_in;
    boost::iostreams::filtering_streambuf<boost::iostreams::input> in2;

    if (fastq2_extention == ".gz") {
        fastq2_in.open(fastq2_string, std::ios_base::in | std::ios_base::binary);
        in2.push(boost::iostreams::gzip_decompressor());
    }
    else if (fastq2_extention == ".bz2") {
        fastq2_in.open(fastq2_string, std::ios_base::in | std::ios_base::binary);
        in2.push(boost::iostreams::bzip2_decompressor());
    }
    else if (fastq2_extention == ".z" || fastq2_extention == ".Z") {
        fastq2_in.open(fastq2_string, std::ios_base::in | std::ios_base::binary);
        in2.push(boost::iostreams::zlib_decompressor());
    } else {
        fastq2_in.open(fastq2_string, std::ios_base::in);
    }
    in2.push(fastq2_in);
    std::istream in_stream2(&in2);


    //out fastq1
    std::ofstream fastq1_out;
    boost::iostreams::filtering_streambuf<boost::iostreams::output> out1;

    if (fastq1_extention == ".gz") {
        fastq1_out.open(fastq1_filename.string(), std::ios_base::out | std::ios_base::binary);
        out1.push(boost::iostreams::gzip_compressor());
    }
    else if (fastq1_extention == ".bz2") {
        fastq1_out.open(fastq1_filename.string(), std::ios_base::out | std::ios_base::binary);
        out1.push(boost::iostreams::bzip2_compressor());
    }
    else if (fastq1_extention == ".z" || fastq1_extention == ".Z") {
        fastq1_out.open(fastq1_filename.string(), std::ios_base::out | std::ios_base::binary);
        out1.push(boost::iostreams::zlib_compressor());
    } else {
        fastq1_out.open(fastq1_filename.string(), std::ios_base::out);
    }
    out1.push(fastq1_out);
    std::ostream out1_stream(&out1);
    
    //out fastq2
    std::ofstream fastq2_out;
    boost::iostreams::filtering_streambuf<boost::iostreams::output> out2;

    if (fastq2_extention == ".gz") {
        fastq2_out.open(fastq2_filename.string(), std::ios_base::out | std::ios_base::binary);
        out2.push(boost::iostreams::gzip_compressor());
    }
    else if (fastq2_extention == ".bz2") {
        fastq2_out.open(fastq2_filename.string(), std::ios_base::out | std::ios_base::binary);
        out2.push(boost::iostreams::bzip2_compressor());
    }
    else if (fastq2_extention == ".z" || fastq2_extention == ".Z") {
        fastq2_out.open(fastq2_filename.string(), std::ios_base::out | std::ios_base::binary);
        out2.push(boost::iostreams::zlib_compressor());
    } else {
        fastq2_out.open(fastq2_filename.string(), std::ios_base::out);
    }
    out2.push(fastq2_out);
    std::ostream out2_stream(&out2);

    unsigned long long int removed_read_pairs = 0;
    unsigned long long int kept_read_pairs = 0;
    unsigned long long int total_read_pairs = 0;
    int loop_count = 1;
    unsigned long long int reads_in_memory = parseresult["reads_in_memory"].as<unsigned long long int>();
    while(true) {
        std::cout << "loop: " << loop_count << std::endl;
        loop_count++;

        std::packaged_task< std::pair<std::set<int>, std::vector<std::string>>(std::istream&, unsigned long long int) > task_get_fastq1_pair(get_fastq_pair);
        std::packaged_task< std::pair<std::set<int>, std::vector<std::string>>(std::istream&, unsigned long long int) > task_get_fastq2_pair(get_fastq_pair);
        std::future<std::pair<std::set<int>, std::vector<std::string>>> fut_fastq1_pair = task_get_fastq1_pair.get_future();
        std::future<std::pair<std::set<int>, std::vector<std::string>>> fut_fastq2_pair = task_get_fastq2_pair.get_future();

        std::thread worker1_thread(std::move(task_get_fastq1_pair), std::ref(in_stream1), reads_in_memory);        
        std::thread worker2_thread(std::move(task_get_fastq2_pair), std::ref(in_stream2), reads_in_memory);

        std::pair<std::set<int>, std::vector<std::string>> pair1 = fut_fastq1_pair.get();
        std::pair<std::set<int>, std::vector<std::string>> pair2 = fut_fastq2_pair.get();

        std::cout << "begin reading input fastq pair" << std::endl;
        worker1_thread.join();
        worker2_thread.join();
        std::cout << "finished reading input fastq pair" << std::endl;

        std::set<int> filtered_set;
        std::set_union(pair1.first.begin(), pair1.first.end(), pair2.first.begin(), pair2.first.end(), std::inserter(filtered_set, filtered_set.begin()));
        std::cout << "created filter set" << std::endl;

        std::thread write_fastq1(std::move(write_fastq), pair1.second, filtered_set, std::ref(out1_stream));
        std::thread write_fastq2(std::move(write_fastq), pair2.second, filtered_set, std::ref(out2_stream));
        write_fastq1.join();
        write_fastq2.join();

        removed_read_pairs += filtered_set.size();
        kept_read_pairs += pair1.second.size() - filtered_set.size();
        total_read_pairs += pair1.second.size();
        if (pair1.second.size() != reads_in_memory) {
            std::cout << "reached end of file" << std::endl;
            break;
        }
    }

    boost::iostreams::close(in1);
    boost::iostreams::close(in2);
    out1_stream.flush();
    boost::iostreams::close(out1);
    out2_stream.flush();
    boost::iostreams::close(out2);

    std::cout << "wrote output fastq pair" << std::endl;

    std::string json_filename = "result.json";
    std::fstream out_json;
    out_json.open(json_filename, std::fstream::out);
    out_json << "{\n"
             << "\t\"removed_read_pairs\": " << removed_read_pairs << ",\n"
             << "\t\"kept_read_pairs\": " << kept_read_pairs << ",\n"
             << "\t\"total_read_pairs\": " << total_read_pairs << "\n"
             << "}";
    int sync_res = out_json.sync();
    if (sync_res != 0) {
      std::cout << "failed to sync: " << json_filename << std::endl;
      exit(EXIT_FAILURE);
    }
    out_json.close();
    if (out_json.fail()) {
      std::cout << "failed to close: " << json_filename << std:: endl;
      exit(EXIT_FAILURE);
    }

    return 0;
}

int run_se(cxxopts::ParseResult parseresult)
{
    std::string fastq1_string = parseresult["fastq"].as<std::string>();
    //fastq1
    boost::filesystem::path fastq1_path = boost::filesystem::canonical(fastq1_string);
    boost::filesystem::path fastq1_filename = fastq1_path.filename();
    std::string fastq1_extention = boost::filesystem::extension(fastq1_filename.string());

    std::ifstream fastq1_in;
    boost::iostreams::filtering_streambuf<boost::iostreams::input> in1;

    if (fastq1_extention == ".gz") {
        fastq1_in.open(fastq1_string, std::ios_base::in | std::ios_base::binary);
        in1.push(boost::iostreams::gzip_decompressor());
    }
    else if (fastq1_extention == ".bz2") {
        fastq1_in.open(fastq1_string, std::ios_base::in | std::ios_base::binary);
        in1.push(boost::iostreams::bzip2_decompressor());
    }
    else if (fastq1_extention == ".z" || fastq1_extention == ".Z") {
        fastq1_in.open(fastq1_string, std::ios_base::in | std::ios_base::binary);
        in1.push(boost::iostreams::zlib_decompressor());
    } else {
        fastq1_in.open(fastq1_string, std::ios_base::in);
    }
    in1.push(fastq1_in);
    std::istream in_stream1(&in1);

    //out fastq1
    std::ofstream fastq1_out;
    boost::iostreams::filtering_streambuf<boost::iostreams::output> out1;

    if (fastq1_extention == ".gz") {
        fastq1_out.open(fastq1_filename.string(), std::ios_base::out | std::ios_base::binary);
        out1.push(boost::iostreams::gzip_compressor());
    }
    else if (fastq1_extention == ".bz2") {
        fastq1_out.open(fastq1_filename.string(), std::ios_base::out | std::ios_base::binary);
        out1.push(boost::iostreams::bzip2_compressor());
    }
    else if (fastq1_extention == ".z" || fastq1_extention == ".Z") {
        fastq1_out.open(fastq1_filename.string(), std::ios_base::out | std::ios_base::binary);
        out1.push(boost::iostreams::zlib_compressor());
    } else {
        fastq1_out.open(fastq1_filename.string(), std::ios_base::out);
    }
    out1.push(fastq1_out);
    std::ostream out1_stream(&out1);

    unsigned long long int removed_read_singletons = 0;
    unsigned long long int kept_read_singletons = 0;
    unsigned long long int total_read_singletons = 0;
    int loop_count = 1;
    unsigned long long int reads_in_memory = parseresult["reads_in_memory"].as<unsigned long long int>();
    while(true) {
        std::cout << "loop: " << loop_count << std::endl;
        loop_count++;

        std::packaged_task< std::pair<std::set<int>, std::vector<std::string>>(std::istream&, unsigned long long int) > task_get_fastq1_pair(get_fastq_pair);
        std::future<std::pair<std::set<int>, std::vector<std::string>>> fut_fastq1_pair = task_get_fastq1_pair.get_future();
        std::thread worker1_thread(std::move(task_get_fastq1_pair), std::ref(in_stream1), reads_in_memory);
        std::pair<std::set<int>, std::vector<std::string>> pair1 = fut_fastq1_pair.get();

        std::cout << "begin reading input fastq singleton" << std::endl;
        worker1_thread.join();
        std::cout << "finished reading input fastq singleton" << std::endl;

        std::set<int> filtered_set = pair1.first;
        std::thread write_fastq1(std::move(write_fastq), pair1.second, filtered_set, std::ref(out1_stream));
        write_fastq1.join();

        removed_read_singletons += filtered_set.size();
        kept_read_singletons += pair1.second.size() - filtered_set.size();
        total_read_singletons += pair1.second.size();
        if (pair1.second.size() != reads_in_memory) {
            std::cout << "reached end of file" << std::endl;
            break;
        }
    }
    
    boost::iostreams::close(in1);
    out1_stream.flush();
    boost::iostreams::close(out1);
    
    std::cout << "wrote output fastq singleton" << std::endl;

    std::string json_filename = "result.json";
    std::fstream out_json;
    out_json.open(json_filename, std::fstream::out);
    out_json << "{\n"
             << "\t\"removed_read_singletons\": " << removed_read_singletons << ",\n"
             << "\t\"kept_read_singletons\": " << kept_read_singletons << ",\n"
             << "\t\"total_read_singletons\": " << total_read_singletons << "\n"
             << "}";
    int sync_res = out_json.sync();
    if (sync_res != 0) {
      std::cout << "failed to sync: " << json_filename << std::endl;
      exit(EXIT_FAILURE);
    }
    out_json.close();
    if (out_json.fail()) {
      std::cout << "failed to close: " << json_filename << std:: endl;
      exit(EXIT_FAILURE);
    }

    return 0;
}

bool get_input_paths_valid(cxxopts::ParseResult parseresult)
{
    using namespace boost::filesystem;
    path cwd = current_path();

    // fastq
    bool fastq_exists = exists(parseresult["fastq"].as<std::string>());
    if (!fastq_exists) {
        std::cerr << "input fastq does not exist at specified path:\n\t"
                  << parseresult["fastq"].as<std::string>() << std::endl;
        return false;
    }

    path fastq_path = system_complete(parseresult["fastq"].as<std::string>());
    if (equivalent(fastq_path.parent_path(), cwd)) {
        std::cerr << "input fastq should not be located in cwd\n"
                  << "\tcwd: " << cwd
                  << "\n\tfastq absolute path: " << fastq_path << std::endl;
        return false;
    }

    // fastq2
    if (parseresult.count("fastq2")) {
        bool fastq2_exists = exists(parseresult["fastq2"].as<std::string>());
        if (!fastq2_exists) {
            std::cerr << "input fastq2 does not exist at specified path:\n\t"
                      << parseresult["fastq2"].as<std::string>() << std::endl;
            return false;
        }

        path fastq2_path = system_complete(parseresult["fastq2"].as<std::string>());
        if (equivalent(fastq2_path.parent_path(), cwd)) {
            std::cerr << "input fastq2 should not be located in cwd\n"
                      << "\tcwd: " << cwd
                      << "\n\tfastq2 absolute path: " << fastq2_path << std::endl;
            return false;
        }
    }
    return true;
}

bool get_options_valid(cxxopts::ParseResult parseresult)
{
    if (parseresult.count("fastq") != 1) {
        std::cerr << "--fastq option must be specified once" << std::endl;
        return false;
    }
    if (parseresult.count("fastq2")) {
        if (parseresult.count("fastq2") != 1) {
            std::cerr << "--fastq2 option can only be specified once" << std::endl;
            return false;
        }
    }
    bool input_paths_valid = get_input_paths_valid(parseresult);
    if (!input_paths_valid) {
        return false;
    }
    if (parseresult.count("reads_in_memory") != 1) {
        std::cerr << "--reads_in_memory must be specified once" << std::endl;
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
      ("reads_in_memory", "how many read pairs to hold in memory(~250M per 100k reads)", cxxopts::value<unsigned long long int>())
      ;
    cxxopts::ParseResult parseresult = options.parse(argc, argv);
    bool options_valid = get_options_valid(parseresult);
    if (!options_valid) {
        std::cerr << "Please use correct parameters" << std::endl;
        return 1;
    }


    if (parseresult.count("fastq2") == 1) {
        run_pe(parseresult);
    } else {
        run_se(parseresult);
    }
    return 0;
}
