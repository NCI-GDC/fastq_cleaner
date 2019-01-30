all:
	g++ fastq.cpp -pthread -O3 -std=c++17 -lboost_system -lboost_filesystem -lboost_iostreams -o fastq_cleaner 
