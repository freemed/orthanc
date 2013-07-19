#!/bin/bash

mkdir -p Build
LAAW_ROOT=~/Subversion/Jomago/Src/Labo/Laaw

${LAAW_ROOT}/Parser/Build/LaawParser.exe Build/CodeModelRaw.json ../OrthancConnection.h -I`pwd`/../../s/jsoncpp-src-0.6.0-rc2/include -fms-extensions && \
    python ${LAAW_ROOT}/Generators/CodeModelPostProcessing.py Build/CodeModel.json Build/CodeModelRaw.json Product.json && \
    python ${LAAW_ROOT}/Generators/GenerateWrapperCpp.py Build/OrthancClient.h Build/CodeModel.json Product.json ConfigurationCpp.json && \
    python ${LAAW_ROOT}/Generators/GenerateExternC.py Build/ExternC.cpp Build/CodeModel.json Product.json && \
    python ${LAAW_ROOT}/Generators/GenerateWindows32Def.py Build/Windows32.def Build/CodeModel.json && \
    python ${LAAW_ROOT}/Generators/GenerateWindows64Def.py Build/Windows64.def Build/CodeModel.json
