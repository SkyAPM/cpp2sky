#!/bin/bash

set -e

[[ -z "${SRCDIR}" ]] && SRCDIR="${PWD}"

OUTPUT_DIR="${SRCDIR}/coverage_report"
DATA_DIR="${SRCDIR}/bazel-testlogs/"
PROJECT=$(basename "${SRCDIR}")

# This is the target that will be run to generate coverage data. It can be overridden
# by consumer projects that want to run coverage on a different/combined target.
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

echo "Generating coverage data..."
bazel coverage --config=ci ${COVERAGE_TARGETS} --test_output=errors

rm -rf ${OUTPUT_DIR}
mkdir -p ${OUTPUT_DIR}

BAZEL_OUTPUT_PATH=$(bazel info output_path 2>/dev/null)
COVERAGE_DATA="${BAZEL_OUTPUT_PATH}/_coverage/_coverage_report.dat"

echo "Generating report..."

genhtml --title ${PROJECT} --ignore-errors "source,unsupported" --output-directory "${OUTPUT_DIR}" "${COVERAGE_DATA}"
tar -zcf ${PROJECT}_coverage.tar.gz ${OUTPUT_DIR}
mv ${PROJECT}_coverage.tar.gz ${OUTPUT_DIR}

echo "HTML coverage report is in ${OUTPUT_DIR}/index.html"
echo "All coverage report files are in ${OUTPUT_DIR}/${PROJECT}_coverage.tar.gz"
