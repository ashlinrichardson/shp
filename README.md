## [viewer/](https://github.com/ashlinrichardson/shp/tree/master/viewer): OpenGL / Glut shp viewer and Intersection Program

Interactive display for large numbers of polygons with large numbers (millions) of points

Includes function for intersecting one set of polygons against another

E.g., if one set of polygons is jurisdictional boundaries, the program will assign a jurisdiction to a given polygon in the other set

For the sample data, provincial parks are assigned to regional fire management areas

## instructions
cd viewer

rm run.exe; make; ./run.exe

![alt text](https://github.com/ashlinrichardson/shp/blob/master/viewer/parks.png)
