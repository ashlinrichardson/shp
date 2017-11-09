# shp
## [data/](https://github.com/ashlinrichardson/shp/tree/master/data): extract shape file data 
cd data;

python extract.py TA_PEP_SVW_polygon > parks.dat

python extract.py DRPMFFRCNT_polygon > firec.dat

cp -v parks.dat firec.dat ../viewer/

cd ..

## [data/](https://github.com/ashlinrichardson/shp/tree/master/data): convert Parks data to GeoJSON
ogr2ogr -f GeoJSON TA_PEP_SVW_polygon.json TA_PEP_SVW_polygon.shp -lco RFC7946=YES
