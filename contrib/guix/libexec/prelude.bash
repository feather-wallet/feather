#!/usr/bin/env bash
export LC_ALL=C
set -e -o pipefail

# shellcheck source=contrib/shell/realpath.bash
source contrib/shell/realpath.bash

# shellcheck source=contrib/shell/git-utils.bash
source contrib/shell/git-utils.bash

################
# Required non-builtin commands should be invocable
################

check_tools() {
    for cmd in "$@"; do
        if ! command -v "$cmd" > /dev/null 2>&1; then
            echo "ERR: This script requires that '$cmd' is installed and available in your \$PATH"
            exit 1
        fi
    done
}

check_tools cat env readlink dirname basename git

################
# We should be at the top directory of the repository
################

same_dir() {
    local resolved1 resolved2
    resolved1="$(bash_realpath "${1}")"
    resolved2="$(bash_realpath "${2}")"
    [ "$resolved1" = "$resolved2" ]
}

if ! same_dir "${PWD}" "$(git_root)"; then
cat << EOF
ERR: This script must be invoked from the top level of the git repository

Hint: This may look something like:
    env FOO=BAR ./contrib/guix/guix-<blah>

EOF
exit 1
fi

################
# Execute "$@" in a pinned, possibly older version of Guix, for reproducibility
# across time.
time-machine() {
    # shellcheck disable=SC2086
    guix time-machine --url=https://codeberg.org/guix/guix.git \
                      --commit=4d8c55de60cf9a5cafc4b881035131921d07314d \
                      --cores="$JOBS" \
                      --keep-failed \
                      --fallback \
                      ${SUBSTITUTE_URLS:+--substitute-urls="$SUBSTITUTE_URLS"} \
                      ${ADDITIONAL_GUIX_COMMON_FLAGS} ${ADDITIONAL_GUIX_TIMEMACHINE_FLAGS} \
                      -- "$@"
}


################
# Set common variables
################

COMMIT="$(git_head_commit)"
TAG="$(git_head_tag)"

if [[ -n "${TAG}" ]]; then
    VERSION="${TAG}"
else
    VERSION="${COMMIT}"
fi

VERSION="${FORCE_VERSION:-${VERSION}}"
DISTNAME="${DISTNAME:-feather-${VERSION}}"

VERSION_BASE_DIR="${VERSION_BASE_DIR:-${PWD}}"
version_base_prefix="${VERSION_BASE_DIR}/guix/guix-build-"
VERSION_BASE="${version_base_prefix}${VERSION}"  # TOP

DISTSRC_BASE="${DISTSRC_BASE:-${VERSION_BASE}}"

OUTDIR_BASE="${OUTDIR_BASE:-${VERSION_BASE}/output}"

LOGDIR_BASE="${LOGDIR_BASE:-${VERSION_BASE}/logs}"

var_base_basename="var"
VAR_BASE="${VAR_BASE:-${VERSION_BASE}/${var_base_basename}}"

profiles_base_basename="profiles"
PROFILES_BASE="${PROFILES_BASE:-${VAR_BASE}/${profiles_base_basename}}"
