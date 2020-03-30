FROM alpine AS builder

RUN apk update \
    && apk add build-base cmake

WORKDIR /usr/src/tusk
COPY . .
RUN mkdir build \
    && cd build \
    && cmake -DCMAKE_EXE_LINKER_FLAGS="-static -Os" .. \
    && cmake --build . \
    && ldd main

FROM scratch

WORKDIR /usr/local/tusk
COPY --from=builder /usr/src/tusk/build/main .
ENTRYPOINT [ "./main" ]
