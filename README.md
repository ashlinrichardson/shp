# shp
cd data;
python extract.py TA_PEP_SVW_polygon > parks.dat
python extract.py DRPMFFRCNT_polygon > firec.dat
cp -v parks.dat firec.dat ../viewer/
cd ..
cd viewer
rm zpr.exe; ./compile; ./zpr.exe
