ARG REPOSITORY=docker.osdc.io
ARG BUILDER_TAG=3.1.0

FROM ${REPOSITORY}/ncigdc/amzn2023-builder:${BUILDER_TAG} AS builder

RUN <<EOF
dnf update --refresh --best --allowerasing -y
dnf --assumeyes install \
  boost \
  boost-devel \
  gcc-c++ \
  git \
  make
EOF

COPY . /fastq_cleaner

WORKDIR /fastq_cleaner/1.0.0

RUN <<EOF
make
cp fastq_cleaner /usr/local/bin/fastq_cleaner
EOF


FROM ${REPOSITORY}/ncigdc/amzn2023:${BUILDER_TAG}

COPY --from=builder /usr/local/bin/fastq_cleaner /usr/local/bin/fastq_cleaner

RUN <<EOF
dnf install -y \
  boost-system \
  boost-filesystem \
  boost-iostreams

dnf clean all
rm -rf /var/cache/dnf /tmp/* /var/tmp/*
EOF
