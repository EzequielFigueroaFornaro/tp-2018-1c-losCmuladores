curl -u $1 -L -o "commons.tar" https://api.github.com/repos/sisoputnfrba/so-commons-library/tarball/master
curl -u $1 -L -o "tpSO.tar" https://api.github.com/repos/sisoputnfrba/tp-2018-1c-losCmuladores/tarball/master
git clone https://github.com/mumuki/cspec.git
cd cspec
make
sudo make install
cd ..
sudo apt-get install unzip
sudo utnso
tar -xvf "tpSO.tar"
mkdir tp
tar -xvf "tpSO.tar" -C "tp" --strip-components=1
mkdir "commons-library"
tar -xvf "commons.tar" -C "commons-library" --strip-components=1
cd commons-library
sudo make install
cd ..
cd tp
sudo utnso
./clean_and_make_all
