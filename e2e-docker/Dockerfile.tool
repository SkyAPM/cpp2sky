FROM openjdk:8

WORKDIR /tests

ARG COMMIT_HASH=24270f8f1ee1cb9186ede5202ff1c4ae3d2d482a

ADD https://github.com/apache/skywalking-agent-test-tool/archive/${COMMIT_HASH}.tar.gz .

RUN tar -xf ${COMMIT_HASH}.tar.gz --strip 1

RUN rm ${COMMIT_HASH}.tar.gz

RUN ./mvnw -B -DskipTests package

FROM openjdk:8

EXPOSE 19876 12800

WORKDIR /tests

COPY --from=0 /tests/dist/skywalking-mock-collector.tar.gz /tests

RUN tar -xf skywalking-mock-collector.tar.gz --strip 1

RUN chmod +x bin/collector-startup.sh

ENTRYPOINT bin/collector-startup.sh