#!/usr/bin/env bash

set -e
set -x

# supported arch:
# - arm
# - arm64
# - x86
# - x86_64

if [ $# -lt 3 ]; then
  echo "$0 should have at least 3 parameters: command, target_arch, output"
  exit 1
fi
set -e
set -x

COMMAND=$1
ARCH=$2
OUTPUT=$3

NODE_SOURCE=`dirname $PWD`
IMAGE_NAME=ndk20b


mkdir -p "$OUTPUT"

case $COMMAND in
configure)
  # --platform linux/386
  # buildx
  docker -D build -t "$IMAGE_NAME" -f "Dockerfile" .

  docker container run -it \
    --mount type=bind,source="$NODE_SOURCE",target=/node \
    --mount type=bind,source="$OUTPUT",target=/output \
    $IMAGE_NAME ./build.sh configure "$ARCH" /output
  ;;
make)
  echo building "$ARCH"
  docker container run -it \
    --mount type=bind,source="$NODE_SOURCE",target=/node \
    --mount type=bind,source="$OUTPUT",target=/output \
    $IMAGE_NAME ./build.sh make "$ARCH" /output
  ;;
*)
  echo "Unsupported command provided: $COMMAND"
  exit -1
  ;;
esac
