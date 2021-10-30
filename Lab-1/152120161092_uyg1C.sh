echo "2.1 tempFiles yaratiliyor.\n"
mkdir -p tempFiles
echo "2.2 icerik listeleniyor\n"
ls -sr
echo "2.3 .c ve .sh tasiniyor\n"
mv *.cpp tempFiles ; mv *.sh tempFiles
echo "2.4 Tüm dosyalar siliniyor\n"
rm *.*
echo "2.5 Tüm dosyalar calisma yoluna kopyalaniyor\n"
sleep 3
cp tempFiles/*.* .
echo "2.6 tempFiles Klasörü recursive siliniyor.\n"
sleep 3
rm -r tempFiles
echo "2.7 kaynak kod derleniyor.\n"
g++ 152120161092_uyg1C.cpp -o executableFile
./executableFile
make &> 152120161092_uyg1C_output.txt
#./executableFile >> 152120161092_uyg1C_output.txt
