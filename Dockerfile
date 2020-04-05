FROM alpine AS builder

RUN apk update \
    && apk add build-base cmake ninja

WORKDIR /usr/src/tusk
COPY . .
RUN mkdir build                                                \
    && cd build                                                \
    && cmake -GNinja -DCMAKE_EXE_LINKER_FLAGS="-static -Os" .. \
    && cmake --build .                                         \
    && ctest                                                   \
    && ldd tusk                                                \
    && ls -lah

FROM scratch

WORKDIR /usr/local/tusk
COPY --from=builder /usr/src/tusk/build/tusk .
ENTRYPOINT [ "./tusk" ]
