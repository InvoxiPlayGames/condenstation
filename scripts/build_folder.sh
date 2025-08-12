#!/bin/bash
rm -rf release
mkdir -p release/portal2_dlc3/addons/condenstation
cp zip1/addons/condenstation.vdf release/portal2_dlc3/addons/condenstation.vdf
cp out/condenstation_ps3.sprx release/portal2_dlc3/addons/condenstation/condenstation_ps3.sprx
# should really be building zip1 here
cp data/zip1.ps3.zip release/portal2_dlc3/zip1.ps3.zip
cp README_release.txt release/README.txt
cd release
rm ../out/condenstation.zip
zip -r ../out/condenstation.zip .
cd ..
