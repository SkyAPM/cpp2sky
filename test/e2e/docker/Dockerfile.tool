FROM eclipse-temurin:8

RUN apt-get update \
 && apt-get install -y --no-install-recommends git \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /tests

ARG COMMIT_HASH=b6efe6af0a5499502b8cf8b76c7351e3f172a616

ADD https://github.com/apache/skywalking-agent-test-tool/archive/${COMMIT_HASH}.tar.gz .

RUN tar -xf ${COMMIT_HASH}.tar.gz --strip 1 \
  && rm ${COMMIT_HASH}.tar.gz \
  && ./mvnw -B -DskipTests package

FROM eclipse-temurin:8

EXPOSE 19876 12800

WORKDIR /tests

COPY --from=0 /tests/dist/skywalking-mock-collector.tar.gz /tests

RUN tar -xf skywalking-mock-collector.tar.gz --strip 1 \
  && chmod +x bin/collector-startup.sh

ENTRYPOINT bin/collector-startup.sh
