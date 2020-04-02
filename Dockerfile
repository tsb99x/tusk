FROM alpine AS builder

RUN apk update \
    && apk add build-base cmake

WORKDIR /usr/src/tusk
COPY . .
RUN mkdir build                                        \
    && cd build                                        \
    && cmake -DCMAKE_EXE_LINKER_FLAGS="-static -Os" .. \
    && cmake --build . --target tusk                   \
    && ldd tusk                                        \
    && ls -lah

FROM scratch

WORKDIR /usr/local/tusk
COPY --from=builder /usr/src/tusk/build/tusk .
ENTRYPOINT [ "./tusk" ]
