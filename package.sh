#!/bin/bash

set -euo pipefail

CONAN_USER="$1"
CONAN_PW="$2"
IMAGE_NAME="tentris-server-rs"

if podman --version > /dev/null; then
    echo "Using podman"
    BUILDER="podman"
elif docker --version > /dev/null; then
    echo "Using docker"
    BUILDER="docker"
else
    echo "Error: Neither podman nor docker found" 2>&1
    exit 1
fi

${BUILDER} build . --tag "${IMAGE_NAME}" --build-arg CONAN_USER=${CONAN_USER} --build-arg CONAN_PW=${CONAN_PW} --ssh=default
CONTAINER_ID=$(${BUILDER} container create "${IMAGE_NAME}")
${BUILDER} cp "${CONTAINER_ID}:/tentris-server-rs" .
${BUILDER} container rm "${CONTAINER_ID}"
