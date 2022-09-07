#!/bin/bash

set -e
#set -x

[[ -z "${SRCDIR}" ]] && SRCDIR="${PWD}"

OUTPUT_DIR="./coverage_report/"
DATA_DIR="${SRCDIR}/bazel-testlogs/"
PROJECT=`basename "${SRCDIR}"`

# This is the target that will be run to generate coverage data. It can be overridden by consumer
# projects that want to run coverage on a different/combined target.
# Command-line arguments take precedence over ${COVERAGE_TARGET}.
if [[ $# -gt 0 ]]; then
  COVERAGE_TARGETS=$*
elif [[ -n "${COVERAGE_TARGET}" ]]; then
  COVERAGE_TARGETS=${COVERAGE_TARGET}
else
  COVERAGE_TARGETS=//test/...
fi


echo "Starting gen_coverage.sh..."
echo "    PWD=$(pwd)"
echo "    OUTPUT_DIR=${OUTPUT_DIR}"
echo "    DATA_DIR=${DATA_DIR}"
echo "    TARGETS=${COVERAGE_TARGETS}"

function gen_coverage_data() {
    echo "Generating coverage data..."
    bazel coverage "$1" --test_output=errors
}

function purge_files() {
    local src_files="$1"
    local purge_files="$2"

    for file in ${src_files}
    do
        if [ -s "${file}" ] ; then
            mv "${file}" "${file}".bak
            lcov --remove "${file}".bak "${purge_files}" -o "${file}"
            chmod 777 "${file}"
        fi
    done
}

function gen_report() {
    echo "Generating report..."

    local output_directory="$1"

    # The location where coverage.dat files were stored
    path="${DATA_DIR}"

    datfiles=$(find ${path} -name "coverage.dat")

    purge_files ${datfiles} "pb.h pb.cc"

    find ${path} -name "coverage.dat" -size 0 -delete

    datfiles=$(find ${path} -name "coverage.dat")

    # add --branch-coverage to show branch coverage
    genhtml --title ${PROJECT} --ignore-errors "source" ${datfiles} -o "${OUTPUT_DIR}"
    tar -zcf ${PROJECT}_coverage.tar.gz ${OUTPUT_DIR}
    mv ${PROJECT}_coverage.tar.gz ${OUTPUT_DIR}

    echo "HTML coverage report is in ${OUTPUT_DIR}/index.html"
    echo "All coverage report files are in ${OUTPUT_DIR}/${PROJECT}_coverage.tar.gz"
}

gen_coverage_data ${COVERAGE_TARGETS}
gen_report ${OUTPUT_DIR}
