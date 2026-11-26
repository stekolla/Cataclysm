#!/usr/bin/env bash
# Download a project-selected loading screen archive into data/.
set -euo pipefail
cd "$(dirname "$0")/.."
: "${CATA_LOADING_SCREENS_ARCHIVE_URL:?Set CATA_LOADING_SCREENS_ARCHIVE_URL to the archive URL}"
curl -fsSL --retry 3 "$CATA_LOADING_SCREENS_ARCHIVE_URL" | tar -xz
