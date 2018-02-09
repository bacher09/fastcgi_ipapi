FROM alpine:3.7 as builder

RUN apk update && apk add gcc g++ cmake make linux-headers musl-dev ninja \
    gmp-dev autoconf  automake libtool binutils \
    fcgi geoip fcgi-dev geoip-dev && \
    mkdir /src && mkdir /build

COPY . /src
WORKDIR /build
RUN cmake -DCMAKE_C_FLAGS="-U_FORTIFY_SOURCE" \
          -DCMAKE_BUILD_TYPE=Release -DWITH_CHROOT=OFF /src && \
    make -j$(nproc) VERBOSE=1


FROM alpine:3.7
COPY --from=builder /build/ip_api /usr/local/bin/ip_api.fastcgi
COPY install_geoip.sh /tmp/
RUN apk --no-cache add fcgi geoip lighttpd && \
    /tmp/install_geoip.sh && \
    rm -f /tmp/install_geoip.sh

COPY sample_conf/lighttpd.conf /etc/lighttpd.conf
RUN chmod o+r /etc/lighttpd.conf && chown 1000 /run
EXPOSE 8080
USER 1000:1000
ENTRYPOINT ["/usr/sbin/lighttpd", "-f", "/etc/lighttpd.conf", "-D"]
