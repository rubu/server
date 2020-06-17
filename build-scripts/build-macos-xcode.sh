#!/bin/sh

fail()
{
    echo "$1" 1>&2
    exit 1
}

# Fail early if environment not set
[ -z "$BUILD_ARCHIVE_NAME" ] && fail "BUILD_ARCHIVE_NAME has to be set"
[ -z "$BUILD_PARALLEL_THREADS" ] && fail "BUILD_PARALLEL_THREADS has to be set"

# Unpack archived dependencies
cd ../dependencies64 || fail "Could not enter ../dependencies64"
tar xvJf large_files_macos.tar.xz || fail "Could not unpack large_files_macos.tar.xz"

# Clean and enter shadow build folder
echo Cleaning...
if [ -e ../build ]; then
    rm -Rf ../build || fail "Could not delete ../build"
fi

mkdir ../build || fail "Could not create ../build"
cd ../build || fail "Could not enter ../build"

# Detect ffmpeg / sfml / freeimage from brew
ffmpeg_include_path=$(brew --prefix ffmpeg)/include

# Run cmake
echo Running cmake...
cmake -DFFMPEG_INCLUDE_PATH:string="${ffmpeg_include_path}" -G Xcode .. || fail "cmake failed"

# Run make using the number of hardware threads in BUILD_PARALLEL_THREADS
echo Building...
xcodebuild -project CasparCG\ Server.xcodeproj -configuration Debug || fail "Could not build Xcode project"

# Create client folder to later zip
export SERVER_FOLDER="$BUILD_ARCHIVE_NAME"
if [ -f "$SERVER_FOLDER" ]; then
    rm -Rf "$SERVER_FOLDER" || fail "Could not delete $SERVER_FOLDER"
fi
mkdir "$SERVER_FOLDER" || fail "Could not create $SERVER_FOLDER"
mkdir "$SERVER_FOLDER/bin" || fail "Could not create $SERVER_FOLDER/bin"
mkdir "$SERVER_FOLDER/lib" || fail "Could not create $SERVER_FOLDER/lib"

# Copy compiled binaries
echo Copying binaries...
cp -f  shell/lib* "$SERVER_FOLDER/lib/" || fail "Could not copy server libraries"
cp -f  shell/*.ttf "$SERVER_FOLDER/" || fail "Could not copy font(s)"
cp -f  shell/casparcg "$SERVER_FOLDER/bin/" || fail "Could not copy server executable"
cp -f  shell/casparcg.config "$SERVER_FOLDER/" || fail "Could not copy server config"
cp -Rf shell/locales "$SERVER_FOLDER/bin/" || fail "Could not copy server CEF locales"
cp -Rf shell/swiftshader "$SERVER_FOLDER/bin/" || fail "Could not copy server CEF swiftshader"
cp -f  shell/*.pak "$SERVER_FOLDER/bin/" || fail "Could not copy CEF resources"
cp -f  shell/*.bin "$SERVER_FOLDER/bin/" || fail "Could not copy V8 resources"
cp -f  shell/*.dat "$SERVER_FOLDER/bin/" || fail "Could not copy ICU resources"

# Copy binary dependencies
echo Copying binary dependencies...
cp -Rf ../deploy/linux/* "$SERVER_FOLDER/" || fail "Could not copy binary dependencies"
cp -f  ../deploy/general/*.pdf "$SERVER_FOLDER/" || fail "Could not copy pdf"
cp -Rf ../deploy/general/wallpapers "$SERVER_FOLDER/" || fail "Could not copy wallpapers"
cp -Rf ../deploy/general/logos "$SERVER_FOLDER/" || fail "Could not copy logos"
cp -Rf ../deploy/general/server/media "$SERVER_FOLDER/" || fail "Could not copy media"
cp -Rf ../deploy/general/server/template "$SERVER_FOLDER/" || fail "Could not copy template"
cp -Rf ../deploy/general/server/font "$SERVER_FOLDER/" || fail "Could not copy font"

# Copy documentation
echo Copying documentation...
cp -f ../CHANGELOG "$SERVER_FOLDER/" || fail "Could not copy CHANGELOG"
cp -f ../README "$SERVER_FOLDER/" || fail "Could not copy README"
cp -f ../LICENSE "$SERVER_FOLDER/" || fail "Could not copy LICENSE"

# Create tar.gz file
echo Creating tag.gz...
tar -cvzf "$BUILD_ARCHIVE_NAME.tar.gz" "$SERVER_FOLDER" || fail "Could not create archive"

