#! /bin/bash
git clone --depth=1 --branch v3.1.1 https://github.com/assimp/assimp.git $HOME/assimp
mkdir -p $HOME/build_assimp
cd $HOME/build_assimp
cmake -DCMAKE_BUILD_TYPE=Release $HOME/assimp
make -j
sudo make install
