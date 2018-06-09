git clone https://github.com/sisoputnfrba/so-commons-library.git
git clone https://github.com/sisoputnfrba/tp-2018-1c-losCmuladores.git
git clone https://github.com/mumuki/cspec.git
cd cspec
make
sudo make install
cd ..
cd so-commons-library
sudo make install
cd ..
cd tp-2018-1c-losCmuladores
./clean_and_make_all
