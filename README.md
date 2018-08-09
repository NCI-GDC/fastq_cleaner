# fastq_cleaner

A multi-threaded (~180% cpu for compressed files when pairs are input) tool to remove filtered fastq reads.

The tool looks for filtered reads in either read of a pair, and will then remove both reads of the pair.

Memory usage is tunable. Aproximately 250M RAM per 100,000 read pairs.

cwl included.
