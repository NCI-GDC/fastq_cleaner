all:
	g++ fastq.cpp -O3 -std=c++2a -lboost_system -lboost_filesystem -lboost_iostreams -o fastq
