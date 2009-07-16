#!/bin/sh

# ZIP file with sources + Windows binaries (.exe and .dll)
find . -not \( -type d -or -wholename './.hg*' -or -name '*.o' -or -name '*.zip' -or -name '*.tar.gz' -or -wholename './launcher/Makefile*' \) | zip -q bomberlua.zip -@ 2>&1 > release.log

# TGZ file with sources and media only
cd ..
find bomberlua -not \( -type d -or -wholename 'bomberlua/.hg*' -or -name '*.o' -or -name '*.zip' -or -name '*.tar.gz' -or -wholename 'bomberlua/launcher/Makefile*' -or -name '*.exe' -or -name '*.dll' \) | xargs tar zcf bomberlua.tar.gz 2>&1 >> release.log
mv bomberlua.tar.gz bomberlua/bomberlua.tar.gz
cd bomberlua
