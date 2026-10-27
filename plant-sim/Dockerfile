FROM debian:bookworm

RUN apt-get update && apt-get install -y \
    build-essential \
    libmodbus-dev \
    pkg-config \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY . .

RUN make clean && make

EXPOSE 5020

CMD ["./main"]