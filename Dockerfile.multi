ARG REPOSITORY=docker.osdc.io
ARG BUILDER_TAG=3.1.0

### Download tarball to scratch layer
FROM scratch AS src
ARG VERSION

FROM ${REPOSITORY}/ncigdc/amzn2023-builder:${BUILDER_TAG} AS builder

RUN --mount=from=src,target=/src <<EOF
dnf update --refresh --best --allowerasing -y
dnf --assumeyes install \
  boost \
  boost-devel \
  gcc-c++ \
  git \
  make
EOF

COPY . /fastq_cleaner

WORKDIR /fastq_cleaner

RUN <<EOF
make
cp fastq_cleaner /usr/local/bin/
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
