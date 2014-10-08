#! /bin/bash
sudo apt-get install -qq -y graphviz flex bison
cd $HOME
wget http://ftp.stack.nl/pub/users/dimitri/doxygen-1.8.8.src.tar.gz
tar zxvf doxygen-1.8.8.src.tar.gz
cd $HOME/doxygen-1.8.8
./configure
make
sudo make install

