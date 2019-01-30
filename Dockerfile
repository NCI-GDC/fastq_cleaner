FROM fedora:29

MAINTAINER Jeremiah H. Savage <jeremiahsavage@gmail.com>

RUN dnf update --refresh --best --allowerasing -y \
    && dnf --assumeyes install \
       boost \
       boost-devel \
       gcc-c++ \
       git \
       make \
    && git clone https://github.com/NCI-GDC/fastq_cleaner.git \
    && cd fastq_cleaner/ \
    && make \
    && mv fastq_cleaner /usr/local/bin/ \
    && cd \
    && rm -rf fastq_cleaner \
    && dnf clean all \
    && rm -rf /tmp/* /var/tmp/*