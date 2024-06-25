FROM alpine:latest
RUN apk add --no-cache cmake make g++
COPY . /app
WORKDIR /app
RUN cmake . && make
CMD ["./cachex", "--server"]

