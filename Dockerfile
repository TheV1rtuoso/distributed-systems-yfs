# docker run -it -v ${PWD}:/proj --privileged distsys
FROM gcc:latest

RUN apt-get update && \
    apt-get install -y libfuse-dev libfuse2 psmisc && \
    rm -rf /var/lib/apt/lists/*

VOLUME .:/project
