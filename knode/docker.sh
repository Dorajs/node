#!/usr/bin/env bash

set -e
set -x

# supported arch:
# - arm
# - arm64
# - x86
# - x86_64

if [ $# -lt 2 ]; then
  echo "$0 should have at least 4 parameters: command, target_arch"
  exit 1
fi
set -e
set -x

COMMAND=$1
ARCH=$2

NODE_SOURCE=`dirname $PWD`
IMAGE_NAME=ndk20b

case $COMMAND in
configure)
  # --platform linux/386
  # buildx
  docker -D build -t "$IMAGE_NAME" -f "Dockerfile" .

  docker container run -it \
    --mount type=bind,source="$NODE_SOURCE",target=/node \
    $IMAGE_NAME /node/build.sh configure "$ARCH"
  ;;
make)
  mkdir -p "$OUTPUT"
  echo building "$ARCH"
  docker container run -it \
    --mount type=bind,source="$NODE_SOURCE",target=/node \
    $IMAGE_NAME /node/build.sh make "$ARCH"
  ;;
*)
  echo "Unsupported command provided: $COMMAND"
  exit -1
  ;;
esac
