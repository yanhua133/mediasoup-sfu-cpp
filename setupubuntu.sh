#apt-cache search PostgreSQL
#apt-cache show PostgreSQL
#sudo apt-get -y update
#sudo apt-get -y install clang
#ubuntu 18.04 cmake=3.10.0  
#ubuntu 20.04 cmake=3.16.0
#for boringssl 
#sudo apt-get install golang
#sudo apt-get -y install git python cmake net-tools
#https://apt.llvm.org/
#wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key|sudo apt-key add -
#sudo apt-get install clang-12 lldb-12 lld-12
wget https://apt.llvm.org/llvm.sh
chmod +x llvm.sh
sudo ./llvm.sh 11

sudo update-alternatives --install /usr/bin/clang clang /usr/bin/clang-11 10000
sudo update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-11 10000
sudo update-alternatives --install /usr/bin/llvm-config llvm-config /usr/bin/llvm-config-11 10000

#http://gcc.gnu.org/projects/cxx-status.html
#https://stackoverflow.com/questions/7031126/switching-between-gcc-and-clang-llvm-using-cmake
sudo update-alternatives --config cc
sudo update-alternatives --config c++


