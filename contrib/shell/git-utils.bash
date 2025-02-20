#!/usr/bin/env bash

git_root() {
    git rev-parse --show-toplevel 2> /dev/null
}

git_head_version() {
    local recent_tag
    if recent_tag="$(git describe --exact-match HEAD 2> /dev/null)"; then
        echo "${recent_tag%-rc}"
    else
        git rev-parse --short=12 HEAD
    fi
}

is_release() {
    local recent_tag
    if recent_tag="$(git describe --exact-match HEAD 2> /dev/null)"; then
        if [[ "$recent_tag" == *"-rc" ]]; then
            echo -n "0"
        fi
        echo -n "1"
    else
        echo -n "0"
    fi
}
