#!/bin/sh

# Fichier ZIP avec sources + binaires Windows (.exe et .dll)
rm -f bomberlua.zip
find . -not \( -type d -or -wholename './.hg*' -or -name '*.o' -or -name '*.zip' -or -name '*.tar.gz' -or -wholename './launcher/Makefile*' -or -wholename './launcher/release/*' \) | zip -q bomberlua.zip -@ 2>&1 > release.log

# Fichier TGZ avec sources et media uniquement
cd ..
rm -f bomberlua/bomberlua.tar.gz
find bomberlua -not \( -type d -or -wholename 'bomberlua/.hg*' -or -name '*.o' -or -name '*.zip' -or -name '*.tar.gz' -or -wholename 'bomberlua/launcher/Makefile*' -or -wholename 'bomberlua/launcher/release/*' -or -name '*.exe' -or -name '*.dll' \) | xargs tar zcf bomberlua.tar.gz 2>&1 >> release.log
mv bomberlua.tar.gz bomberlua/bomberlua.tar.gz
cd bomberlua
