#!/bin/bash
set -e

VERSION=$1
if [ -z "$VERSION" ]; then
    echo "Usage: ./release.sh v1.1"
    exit 1
fi

echo "=== Releasing Monastery $VERSION ==="

# Stage everything (code, resources.qrc, icons folder, etc.)
git add -A

# Clean commit message
git commit -m "v$VERSION - final polish: embedded toolbar icons + centered bold title + custom window/taskbar icon"

# Create annotated tag
git tag -a "v$VERSION" -m "Monastery v$VERSION - minimalist UI with embedded icons and perfect title bar"

# Push code and tags
git push origin main
git push origin --tags

echo "✅ v$VERSION pushed to GitHub!"
echo "Go to https://github.com/noblebrown-69/monastery/releases"
echo "and create a new release from the v$VERSION tag."