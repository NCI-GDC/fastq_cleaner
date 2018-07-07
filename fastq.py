#!/usr/bin/env python

import argparse
import os
from itertools import islice

def run_se(fastq):
    fastq_out = open(os.path.basename(fastq), 'w')
    
    with open(fastq, 'r') as f_in:
        while True:
            fastq_record = list(islice(f_in, 4))
            if not fastq_record:
                break
            name_list = fastq_record[0].split()
            if len(name_list) == 2:
                rfcs_list = name_list[1].split(':')
                if rfcs_list[1] == 'Y':
                    continue
            fastq_out.write(fastq_record[0]+'\n'+
                            fastq_record[1]+'\n'+
                            fastq_record[2]+'\n'+
                            fastq_record[3]+'\n')

    fastq_out.close()
    return

def main():
    parser = argparse.ArgumentParser('picard docker tool')
    parser.add_argument('--fastq',
                        required = True
    )
    parser.add_argument('--fastq2',
                        required = False
    )

    args = parser.parse_args()
    fastq = args.fastq
    fastq2 = args.fastq2

    if fastq2:
        run_pe(fastq, fastq2)
    else:
        run_se(fastq)

    return

if __name__ == '__main__':
    main()
