#!/bin/sh

set -eu

cd "$(dirname "$0")"

LD_LIBRARY_PATH=.. DYLD_LIBRARY_PATH=.. exec ./example
