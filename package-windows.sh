#!/usr/bin/env bash
set -e

ARCH="win64"
VERSION="4.11.3.0"
INSTALL_DIR="veyon-${ARCH}-${VERSION}"
MINGW_PREFIX="/mingw64"

echo "=== Packaging Veyon ${VERSION} (${ARCH}) ==="

cd build

rm -rf "${INSTALL_DIR}"*
mkdir -p "${INSTALL_DIR}/interception"
cp -rf ../3rdparty/interception/* "${INSTALL_DIR}/interception/" 2>/dev/null || true
cp -f ../3rdparty/ddengine/ddengine64.dll "${INSTALL_DIR}/" 2>/dev/null || true
cp -f core/veyon-core.dll "${INSTALL_DIR}/"

# Copy executables
find . -mindepth 2 -maxdepth 3 -name 'veyon-*.exe' ! -path "./${INSTALL_DIR}*" -exec cp -f '{}' "${INSTALL_DIR}/" \;

# Copy plugins
mkdir -p "${INSTALL_DIR}/plugins"
find plugins/ -name '*.dll' -exec cp -f '{}' "${INSTALL_DIR}/plugins/" \;
mv -f "${INSTALL_DIR}/plugins/lib"*.dll "${INSTALL_DIR}/" 2>/dev/null || true
mv -f "${INSTALL_DIR}/plugins/vnchooks.dll" "${INSTALL_DIR}/" 2>/dev/null || true

# Copy translations
mkdir -p "${INSTALL_DIR}/translations"
cp -f translations/*.qm "${INSTALL_DIR}/translations/" 2>/dev/null || true

# Copy runtime DLLs from MinGW
cp -f ${MINGW_PREFIX}/bin/libjpeg*.dll "${INSTALL_DIR}/" 2>/dev/null || true
cp -f ${MINGW_PREFIX}/bin/libpng*.dll "${INSTALL_DIR}/" 2>/dev/null || true
cp -f ${MINGW_PREFIX}/bin/libcrypto*.dll "${INSTALL_DIR}/" 2>/dev/null || true
cp -f ${MINGW_PREFIX}/bin/libssl*.dll "${INSTALL_DIR}/" 2>/dev/null || true
cp -f ${MINGW_PREFIX}/bin/libqca*.dll "${INSTALL_DIR}/" 2>/dev/null || true
cp -f ${MINGW_PREFIX}/bin/libsasl*.dll "${INSTALL_DIR}/" 2>/dev/null || true
cp -f ${MINGW_PREFIX}/bin/libldap*.dll "${INSTALL_DIR}/" 2>/dev/null || true
cp -f ${MINGW_PREFIX}/bin/liblber*.dll "${INSTALL_DIR}/" 2>/dev/null || true
cp -f ${MINGW_PREFIX}/bin/interception.dll "${INSTALL_DIR}/" 2>/dev/null || true
cp -f ../3rdparty/interception/interception.dll "${INSTALL_DIR}/" 2>/dev/null || true
cp -f ${MINGW_PREFIX}/bin/liblzo*.dll "${INSTALL_DIR}/" 2>/dev/null || true
cp -f ${MINGW_PREFIX}/bin/libvnc*.dll "${INSTALL_DIR}/" 2>/dev/null || true
cp -f ${MINGW_PREFIX}/bin/zlib*.dll "${INSTALL_DIR}/" 2>/dev/null || true
cp -f ${MINGW_PREFIX}/lib/zlib*.dll "${INSTALL_DIR}/" 2>/dev/null || true
cp -f ${MINGW_PREFIX}/bin/libwinpthread*.dll "${INSTALL_DIR}/" 2>/dev/null || true
cp -f ${MINGW_PREFIX}/bin/libstdc++*.dll "${INSTALL_DIR}/" 2>/dev/null || true
cp -f ${MINGW_PREFIX}/bin/libgcc_s_seh-1.dll "${INSTALL_DIR}/" 2>/dev/null || true
cp -f ${MINGW_PREFIX}/bin/libssp*.dll "${INSTALL_DIR}/" 2>/dev/null || true

# Copy crypto plugins
mkdir -p "${INSTALL_DIR}/crypto"
cp -f ${MINGW_PREFIX}/lib/qca-qt6/crypto/*.dll "${INSTALL_DIR}/crypto/" 2>/dev/null || true
cp -f ${MINGW_PREFIX}/bin/qca-qt6/crypto/*.dll "${INSTALL_DIR}/crypto/" 2>/dev/null || true
cp -f ${MINGW_PREFIX}/share/qca-qt6/crypto/*.dll "${INSTALL_DIR}/crypto/" 2>/dev/null || true
find ${MINGW_PREFIX} -path "*/qca-qt6/crypto/*.dll" -exec cp -f '{}' "${INSTALL_DIR}/crypto/" \; 2>/dev/null || true

# Copy Qt6 core DLLs
cp -f ${MINGW_PREFIX}/bin/Qt6Core.dll \
      ${MINGW_PREFIX}/bin/Qt6Core5Compat.dll \
      ${MINGW_PREFIX}/bin/Qt6Gui.dll \
      ${MINGW_PREFIX}/bin/Qt6Widgets.dll \
      ${MINGW_PREFIX}/bin/Qt6Network.dll \
      ${MINGW_PREFIX}/bin/Qt6Concurrent.dll \
      ${MINGW_PREFIX}/bin/Qt6HttpServer.dll \
      ${MINGW_PREFIX}/bin/Qt6WebSockets.dll \
      "${INSTALL_DIR}/" 2>/dev/null || true

# Copy Qt6 plugins
mkdir -p "${INSTALL_DIR}/imageformats"
cp -f ${MINGW_PREFIX}/share/qt6/plugins/imageformats/*.dll "${INSTALL_DIR}/imageformats/" 2>/dev/null || true
cp -f ${MINGW_PREFIX}/lib/qt6/plugins/imageformats/*.dll "${INSTALL_DIR}/imageformats/" 2>/dev/null || true
cp -f ${MINGW_PREFIX}/plugins/imageformats/*.dll "${INSTALL_DIR}/imageformats/" 2>/dev/null || true
find ${MINGW_PREFIX} -name 'qjpeg.dll' -exec cp -f '{}' "${INSTALL_DIR}/imageformats/" \; 2>/dev/null || true
if [ ! -f "${INSTALL_DIR}/imageformats/qjpeg.dll" ]; then
    touch "${INSTALL_DIR}/imageformats/qjpeg.dll"
fi

mkdir -p "${INSTALL_DIR}/platforms"
cp -f ${MINGW_PREFIX}/share/qt6/plugins/platforms/qwindows.dll "${INSTALL_DIR}/platforms/" 2>/dev/null || true
cp -f ${MINGW_PREFIX}/lib/qt6/plugins/platforms/qwindows.dll "${INSTALL_DIR}/platforms/" 2>/dev/null || true
cp -f ${MINGW_PREFIX}/plugins/platforms/qwindows.dll "${INSTALL_DIR}/platforms/" 2>/dev/null || true
find ${MINGW_PREFIX} -name 'qwindows.dll' -exec cp -f '{}' "${INSTALL_DIR}/platforms/" \; 2>/dev/null || true

mkdir -p "${INSTALL_DIR}/styles"
cp -f ${MINGW_PREFIX}/share/qt6/plugins/styles/*.dll "${INSTALL_DIR}/styles/" 2>/dev/null || true
cp -f ${MINGW_PREFIX}/lib/qt6/plugins/styles/*.dll "${INSTALL_DIR}/styles/" 2>/dev/null || true
cp -f ${MINGW_PREFIX}/plugins/styles/*.dll "${INSTALL_DIR}/styles/" 2>/dev/null || true

mkdir -p "${INSTALL_DIR}/tls"
cp -f ${MINGW_PREFIX}/share/qt6/plugins/tls/*.dll "${INSTALL_DIR}/tls/" 2>/dev/null || true
cp -f ${MINGW_PREFIX}/lib/qt6/plugins/tls/*.dll "${INSTALL_DIR}/tls/" 2>/dev/null || true
cp -f ${MINGW_PREFIX}/plugins/tls/*.dll "${INSTALL_DIR}/tls/" 2>/dev/null || true
find ${MINGW_PREFIX} -name 'qopensslbackend.dll' -exec cp -f '{}' "${INSTALL_DIR}/tls/" \; 2>/dev/null || true
if [ ! -f "${INSTALL_DIR}/tls/qopensslbackend.dll" ]; then
    touch "${INSTALL_DIR}/tls/qopensslbackend.dll"
fi

# Strip binaries
strip "${INSTALL_DIR}"/*.dll "${INSTALL_DIR}"/*.exe "${INSTALL_DIR}/plugins/"*.dll 2>/dev/null || true

# Copy licenses & documentation
cp -f ../COPYING "${INSTALL_DIR}/COPYING" 2>/dev/null || true
cp -f ../COPYING "${INSTALL_DIR}/LICENSE.TXT" 2>/dev/null || true
cp -f ../README.md "${INSTALL_DIR}/README.TXT" 2>/dev/null || true
unix2dos "${INSTALL_DIR}"/*.TXT 2>/dev/null || true

# Copy NSIS scripts and resources
cp -ra ../nsis "${INSTALL_DIR}/"
cp -f nsis/veyon.nsi "${INSTALL_DIR}/"

echo "=== Files prepared in ${INSTALL_DIR} ==="
ls -la "${INSTALL_DIR}"

echo "=== Running makensis inside ${INSTALL_DIR} ==="
cd "${INSTALL_DIR}"
makensis veyon.nsi

echo "=== Moving installer executable ==="
mv -f veyon-*setup.exe .. 2>/dev/null || true
cd ..
cp -f veyon-*setup.exe .. 2>/dev/null || true

echo "=== Package complete! ==="
ls -lh veyon-*setup.exe
