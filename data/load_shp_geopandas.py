import geopandas as gp
glaciers = gp.GeoDataFrame.from_file(
    'ne_10m_glaciated_areas/ne_10m_glaciated_areas.shp')
glaciers.head()

