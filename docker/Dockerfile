FROM ubuntu:18.04
WORKDIR /application
RUN apt-get update -y \
&& apt-get install -y libtool autoconf build-essential git
RUN git clone https://github.com/pmodels/argobots.git\
&&cd argobots/ \
&& ./autogen.sh \
&& ./configure \
&& make -j
ENTRYPOINT [ "bash" ]

