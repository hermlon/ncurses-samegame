FROM alpine:3 AS builder
WORKDIR /build/
RUN apk add --no-cache build-base cmake ncurses-dev
COPY CMakeLists.txt ./
COPY main.cpp ./
RUN cmake . && make

FROM alpine:3
RUN rm /etc/motd
WORKDIR /root/
RUN apk add --no-cache libstdc++ ncurses-dev openssh foot-extra-terminfo
RUN adduser -D -s /home/samegame/main samegame
RUN passwd -d samegame
WORKDIR /home/samegame/
COPY sshd_config /sshd_config
COPY --from=builder /build/main ./
CMD /usr/sbin/sshd -D -f /sshd_config
