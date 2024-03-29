FROM openjdk:8

WORKDIR /tests

ARG COMMIT_HASH=7dbdf726bdd12b67f09b3898bb95790eacbedf84

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
