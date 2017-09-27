# shp
## [data/](https://github.com/ashlinrichardson/shp/tree/master/data): extract shape file data 
cd data;

python extract.py TA_PEP_SVW_polygon > parks.dat

python extract.py DRPMFFRCNT_polygon > firec.dat

cp -v parks.dat firec.dat ../viewer/

cd ..

## [viewer/](https://github.com/ashlinrichardson/shp/tree/master/viewer): OpenGL / Glut shp viewer
cd viewer

rm zpr.exe; ./compile; ./zpr.exe

![alt text](https://github.com/ashlinrichardson/shp/blob/master/viewer/parks.png)
