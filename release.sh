#!/bin/sh

# Identifier of the release
REL=$(date +"%Y%m%d")
echo "Packaging release $REL..."
echo "Packaging release $REL..." > release.log

# ZIP file with sources + Windows binaries (.exe and .dll)
rm -f bomberlua-$REL.zip
find . -not \( -type d -or -wholename './.hg*' -or -name '*.o' -or -name '*.zip' -or -name '*.tar.gz' -or -name '*.log' -or -wholename './launcher/Makefile*' -or -wholename './launcher/release/*' \) | zip -q bomberlua-$REL.zip -@ 2>&1 >> release.log

# TGZ file with sources and media only
cd ..
rm -f bomberlua/bomberlua-$REL.tar.gz
find bomberlua -not \( -type d -or -wholename 'bomberlua/.hg*' -or -name '*.o' -or -name '*.zip' -or -name '*.tar.gz' -or -name '*.log' -or -wholename 'bomberlua/launcher/Makefile*' -or -wholename 'bomberlua/launcher/release/*' -or -name '*.exe' -or -name '*.dll' \) | xargs tar zcf bomberlua-$REL.tar.gz 2>&1 >> bomberlua/release.log
mv bomberlua-$REL.tar.gz bomberlua/bomberlua-$REL.tar.gz
cd bomberlua
