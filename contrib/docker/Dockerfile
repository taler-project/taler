# Build stage for BerkeleyDB
FROM alpine as berkeleydb

RUN apk --no-cache add autoconf automake build-base openssl

ENV BERKELEYDB_VERSION=db-4.8.30.NC
ENV BERKELEYDB_PREFIX=/opt/${BERKELEYDB_VERSION}

RUN wget https://download.oracle.com/berkeley-db/${BERKELEYDB_VERSION}.tar.gz \
    && tar -xzf *.tar.gz \
    && sed s/__atomic_compare_exchange/__atomic_compare_exchange_db/g -i ${BERKELEYDB_VERSION}/dbinc/atomic.h \
    && mkdir -p ${BERKELEYDB_PREFIX}

WORKDIR /${BERKELEYDB_VERSION}/build_unix

RUN ../dist/configure --enable-cxx --disable-shared --with-pic --prefix=${BERKELEYDB_PREFIX}
RUN make
RUN make install
RUN rm -rf ${BERKELEYDB_PREFIX}/docs

# Build stage for Bitcoin Core
FROM alpine as taler-core

COPY --from=berkeleydb /opt /opt

RUN apk --no-cache add git autoconf automake boost-dev build-base chrpath file libevent-dev gnupg openssl openssl-dev libtool protobuf-dev zeromq-dev

ENV TALER_VERSION=0.16.3.1
ENV TALER_PREFIX=/opt/taler-${TALER_VERSION}

RUN git clone https://github.com/taler-project/taler.git --depth=1

WORKDIR /taler

RUN sed -i '/AC_PREREQ/a\AR_FLAGS=cr' src/univalue/configure.ac
RUN sed -i '/AX_PROG_CC_FOR_BUILD/a\AR_FLAGS=cr' src/secp256k1/configure.ac
RUN sed -i s:sys/fcntl.h:fcntl.h: src/compat.h

# Fix alpine segfault
RUN sed -i '/char\ scratchpad\[SCRYPT_SCRATCHPAD_SIZE\];/a\memset(scratchpad, 0, sizeof(scratchpad));' src/crypto/scrypt.cpp \
 && sed -i 's/char\ scratchpad\[SCRYPT_SCRATCHPAD_SIZE\];/static &/g' src/crypto/scrypt.cpp

RUN ./autogen.sh
RUN ./configure LDFLAGS=-L`ls -d /opt/db*`/lib/ CPPFLAGS=-I`ls -d /opt/db*`/include/ \
    --prefix=${TALER_PREFIX} \
    --mandir=/usr/share/man \
    --disable-tests \
    --disable-bench \
    --disable-ccache \
    --with-gui=no \
    --with-utils \
    --with-libs \
    --with-daemon \
    --disable-shared \
    --enable-static
RUN make
RUN make install
RUN strip ${TALER_PREFIX}/bin/taler-cli \
 && strip ${TALER_PREFIX}/bin/taler-tx \
 && strip ${TALER_PREFIX}/bin/talerd \
 && strip ${TALER_PREFIX}/lib/libbitcoinconsensus.a 

# Build stage for compiled artifacts
FROM alpine

RUN adduser -S taler
RUN apk --no-cache add \
  boost \
  boost-program_options \
  libevent \
  openssl \
  libzmq \
  su-exec

ENV TALER_DATA=/home/taler/.taler
ENV TALER_VERSION=0.16.3.1
ENV TALER_PREFIX=/opt/taler-${TALER_VERSION}
ENV PATH=${TALER_PREFIX}/bin:$PATH

COPY --from=taler-core ${TALER_PREFIX} ${TALER_PREFIX}
COPY docker-entrypoint.sh /entrypoint.sh

VOLUME ["/home/taler/.taler"]

EXPOSE 23153

ENTRYPOINT ["/entrypoint.sh"]

CMD ["talerd"]

