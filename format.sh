#!/bin/bash

set -e

git ls-tree --full-tree --name-only -r HEAD\
  | grep -E -i "\.(cpp|c|h|hpp)$" \
  | xargs clang-format -i
