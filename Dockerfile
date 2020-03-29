FROM alpine AS builder

RUN apk update \
    && apk add build-base cmake

WORKDIR /usr/src/tusk
COPY . .
RUN mkdir build \
    && cd build \
    && cmake .. \
    && cmake --build .

FROM alpine

WORKDIR /usr/local/tusk
COPY --from=builder /usr/src/tusk/build/main .
ENTRYPOINT [ "./main" ]
