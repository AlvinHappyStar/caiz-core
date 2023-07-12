ARG DISTRO
FROM ubuntu:${DISTRO}
# We need to repeat ARG here to make it available inside build context
# See https://docs.docker.com/engine/reference/builder/#understand-how-arg-and-from-interact
ARG DISTRO

MAINTAINER Siddharth Suresh <siddharth@caiz.org>

EXPOSE 11625
EXPOSE 11626

VOLUME /data
VOLUME /postgresql-unix-sockets

ADD setup /
RUN /setup

ARG STELLAR_CORE_VERSION

#install caiz-core
RUN wget -qO - https://apt.caiz.org/SDF.asc | apt-key add -
RUN echo "deb https://apt.caiz.org ${DISTRO} unstable" | tee -a /etc/apt/sources.list.d/SDF-unstable.list
RUN apt-get update && apt-get install -y caiz-core=${STELLAR_CORE_VERSION}

WORKDIR "/etc/caiz"
ENTRYPOINT ["/usr/bin/caiz-core"]
