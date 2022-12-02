#!/bin/bash
make clean
rm -rf dist
mkdir dist
cp CMakeLists.txt dist
cp -r src dist
cp -r include dist
cp -r lib dist
cp -r assets dist
cp -r internal_assets dist
cp -r joc2d dist
cp -r joc_src dist
cp joc_final.desktop dist
cp joc_final.png dist
cp joc_final dist
rm dist.zip
zip -r dist.zip dist
