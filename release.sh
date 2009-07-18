#!/bin/sh

# Identifieur de la release
REL=$(date +"%Y%m%d")
echo "Packaging release $REL..."
echo "Packaging release $REL..." > release.log

# Fichier ZIP avec sources + binaires Windows (.exe et .dll)
rm -f bomberlua-$REL.zip
find . -not \( -type d -or -wholename './.hg*' -or -name '*.o' -or -name '*.zip' -or -name '*.tar.gz' -or -name '*.log' -or -wholename './launcher/Makefile*' -or -wholename './launcher/release/*' \) | zip -q bomberlua-$REL.zip -@ 2>&1 >> release.log

# Fichier TGZ avec sources et media uniquement
cd ..
rm -f bomberlua/bomberlua-$REL.tar.gz
find bomberlua -not \( -type d -or -wholename 'bomberlua/.hg*' -or -name '*.o' -or -name '*.zip' -or -name '*.tar.gz' -or -name '*.log' -or -wholename 'bomberlua/launcher/Makefile*' -or -wholename 'bomberlua/launcher/release/*' -or -name '*.exe' -or -name '*.dll' \) | xargs tar zcf bomberlua-$REL.tar.gz 2>&1 >> bomberlua/release.log
mv bomberlua-$REL.tar.gz bomberlua/bomberlua-$REL.tar.gz
cd bomberlua
