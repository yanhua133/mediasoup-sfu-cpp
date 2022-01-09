if [ -e xcode ]
then
echo "exist xcode"
/bin/rm -rf xcode
mkdir xcode
else
mkdir xcode
echo "mkdir xcode"
fi
#cd xcode
cd xcode
cmake -G "Xcode" ..

