#!/usr/bin/env cwl-runner

cwlVersion: v1.0

requirements:
  - class: DockerRequirement
    dockerPull: quay.io/ncigdc/fastq_cleaner:dba8a6e2cdba27cc1bfac6455c707e541dd4ca50198373b2c36cb5f6ac461e9d
  - class: InlineJavascriptRequirement
  - class: ResourceRequirement
    coresMin: 1
    coresMax: 1
    ramMin: 20000
    ramMax: 20000
    tmpdirMin: $(Math.ceil(1.1 * inputs.fastq.size / 1048576))
    tmpdirMax: $(Math.ceil(1.1 * inputs.fastq.size / 1048576))
    outdirMin: $(Math.ceil(1.1 * inputs.fastq.size / 1048576))
    outdirMax: $(Math.ceil(1.1 * inputs.fastq.size / 1048576))

class: CommandLineTool

inputs:
  - id: fastq
    type: File
    format: "edam:format_2182"
    inputBinding:
      prefix: --fastq

  - id: reads_in_memory
    type: long
    default: 500000
    inputBinding:
      prefix: --reads_in_memory

outputs:
  - id: cleaned_fastq
    type: File
    format: "edam:format_2182"
    outputBinding:
      glob: $(inputs.fastq.basename)

  - id: result_json
    type: File
    format: "edam:format_3464"
    outputBinding:
      glob: "result.json"

baseCommand: [/usr/local/bin/fastq_cleaner]
