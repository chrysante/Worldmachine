#! /bin/bash
parent_path=$( cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P )
cd "$parent_path"
premake5 xcode4
xcodebuild -scheme Worldmachine build -configuration Release