# # FINAL STEP
FROM debian:bullseye-slim

RUN apt-get update && apt-get install -y build-essential \
    cmake \
    git \
    wget \
    tar \
    autoconf

#
# install libevent
#
RUN wget https://github.com/libevent/libevent/releases/download/release-2.1.9-beta/libevent-2.1.9-beta.tar.gz \
    && tar zxvf libevent-2.1.9-beta.tar.gz \
    && cd libevent-2.1.9-beta \
    && ./configure \
    && make \
    && make install

#
# install glog
#
RUN wget https://github.com/google/glog/archive/v0.3.5.tar.gz \
    && tar zxvf v0.3.5.tar.gz \
    && cd glog-0.3.5 \
    && ./configure \
    && make \
    && make install


WORKDIR /luxorproxy
COPY . .
WORKDIR /luxorproxy/build
RUN cmake -DCMAKE_BUILD_TYPE=Debug .. \ 
    && make
RUN cp /luxorproxy/build/btcagent /luxorproxy/build/luxorproxy

EXPOSE 3333 3334

ENTRYPOINT [ "/luxorproxy/build/luxorproxy" ]