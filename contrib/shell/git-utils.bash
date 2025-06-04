#!/usr/bin/env bash

git_root() {
    git rev-parse --show-toplevel 2> /dev/null
}

git_head_commit() {
    git rev-parse --short=12 HEAD
}

git_head_tag() {
    echo -n $(git describe --exact-match HEAD 2> /dev/null)
}
