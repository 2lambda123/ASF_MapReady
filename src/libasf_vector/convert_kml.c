#include "asf_vector.h"
#include "asf.h"

// Convert meta to kml
void meta2kml(char *line, FILE *fp)
{
  meta_parameters *meta;

  meta = meta_read(line);
  kml_entry(fp, meta, meta->general->basename);
  meta_free(meta);

  return;
}

// Convert point to kml
void point2kml(char *line, FILE *fp)
{
  float lat, lon;
  char *p;

  // Extract information out of the line
  p = strchr(line, ',');
  if (p) {
    sscanf(p+1, "%f,%f", &lat, &lon);
    *p = '\0';

    // Write information in kml file
    fprintf(fp, "<Placemark>\n");
    fprintf(fp, "<description><![CDATA[\n");
    fprintf(fp, "<strong>Latitude</strong>: %9.4f<br>\n", lat);
    fprintf(fp, "<strong>Longitude</strong>: %9.4f<br>\n", lon);
    fprintf(fp, "]]></description>\n");
    fprintf(fp, "<name>%s</name>\n", line);
    fprintf(fp, "<LookAt>\n");
    fprintf(fp, "<longitude>%9.4f</longitude>\n", lon);
    fprintf(fp, "<latitude>%9.4f</latitude>\n", lat);
    fprintf(fp, "<range>400000</range>\n");
    fprintf(fp, "</LookAt>\n");
    fprintf(fp, "<Point>\n");
    fprintf(fp, "<coordinates>%f,%f,0</coordinates>\n", lon, lat);
    fprintf(fp, "</Point>\n");
    fprintf(fp, "</Placemark>\n");
  }

  return;
}

// Convert polygon to kml
void polygon2kml(char *line, FILE *fp, char *name)
{
  int ii, vertices;
  double *lat, *lon;
  char id[255];
  char *p, *p_lat, *p_lon;

  // Read ID and number of vertices;
  p = strchr(line, ',');
  if (p) {
    sscanf(p+1, "%d", &vertices);
    *p = '\0';
    sprintf(id, "%s", line);
    *p = ',';
  }

  // Read coordinates of the vertices
  lat = (double *) MALLOC(sizeof(double)*(vertices+1));
  lon = (double *) MALLOC(sizeof(double)*(vertices+1));
  p_lat = p;
  for (ii=0; ii<vertices; ii++) {
    sscanf(p_lat+1, "%lf", &lat[ii]);
    p_lon = strchr(p_lat+1, ',');
    sscanf(p_lon+1, "%lf", &lon[ii]);
    p_lat = strchr(p_lon+1, ',');
  }
  lat[vertices] = lat[0];
  lon[vertices] = lon[0];

  // Write information in kml file
  fprintf(fp, "<Placemark>\n");
  fprintf(fp, "<description><![CDATA[\n");
  fprintf(fp, "<strong>ID</strong>:%s<br>\n", id);
  fprintf(fp, "<strong>Vertices</strong>: %d<br>\n", vertices);
  for (ii=0; ii<vertices; ii++) {
    fprintf(fp, "<strong>%d</strong> - ", ii+1);
    fprintf(fp, "<strong>Lat</strong>: %9.4f, ", lat[ii]);
    fprintf(fp, "<strong>Lon</strong>: %9.4f<br>\n", lon[ii]);
  }
  fprintf(fp, "]]></description>\n");
  fprintf(fp, "<name>%s</name>\n", name);
  fprintf(fp, "<LookAt>\n");
  fprintf(fp, "<longitude>%9.4f</longitude>\n", lon[0]);
  fprintf(fp, "<latitude>%9.4f</latitude>\n", lat[0]);
  fprintf(fp, "<range>400000</range>\n");
  fprintf(fp, "</LookAt>\n");
  write_kml_style_keys(fp);
  fprintf(fp, "<Polygon>\n");
  fprintf(fp, "<outerBoundaryIs>\n");
  fprintf(fp, "<LineString>\n");
  fprintf(fp, "<coordinates>\n");
  
  for (ii=0; ii<=vertices; ii++)
    fprintf(fp, "%.12f,%.12f,4000\n", lon[ii], lat[ii]);
  
  fprintf(fp, "</coordinates>\n");
  fprintf(fp, "</LineString>\n");
  fprintf(fp, "</outerBoundaryIs>\n");
  fprintf(fp, "</Polygon>\n");
  fprintf(fp, "</Placemark>\n");

  return;
}

// Convert RGPS cell information to kml
void rgps2kml(cell_t cell, double *lat, double *lon, FILE *fp)
{
  int ii;

  // Write information in kml file
  fprintf(fp, "<Placemark>\n");
  fprintf(fp, "  <description><![CDATA[\n");
  fprintf(fp, "<strong>Cell</strong>: %ld<br>\n", cell.cell_id);
  fprintf(fp, "<strong>Vertices</strong>: %d<br>\n", cell.nVertices);
  for (ii=0; ii<cell.nVertices; ii++) {
    fprintf(fp, "<strong>%d</strong> - ", ii+1);
    fprintf(fp, "<strong>Lat</strong>: %9.4f, ", lat[ii]);
    fprintf(fp, "<strong>Lon</strong>: %9.4f<br>\n", lon[ii]);
  }
  fprintf(fp, "<strong>Date</strong>: %s<br>\n", cell.date);
  fprintf(fp, "<strong>Source image</strong>: %s<br>\n", cell.sourceImage);
  fprintf(fp, "<strong>Target image</strong>: %s<br>\n", cell.targetImage);
  fprintf(fp, "<strong>Stream</strong>: %s<br>\n", cell.stream);
  fprintf(fp, "<strong>Area</strong>: %.1lf<br>\n", cell.area);
  fprintf(fp, "<strong>Multi-year ice</strong>: %.3lf<br>\n", 
	  cell.multi_year_ice);
  fprintf(fp, "<strong>Open water</strong>: %.3lf<br>\n", cell.open_water);
  fprintf(fp, "<strong>Incidence angle</strong>: %.4lf<br>\n", 
	  cell.incidence_angle);
  fprintf(fp, "<strong>Cell x</strong>: %.3lf<br>\n", cell.cell_x);
  fprintf(fp, "<strong>Cell y</strong>: %.3lf<br>\n", cell.cell_y);
  fprintf(fp, "<strong>dudx</strong>: %.4lf<br>\n", cell.dudx);
  fprintf(fp, "<strong>dudy</strong>: %.4lf<br>\n", cell.dudy);
  fprintf(fp, "<strong>dvdx</strong>: %.4lf<br>\n", cell.dvdx);
  fprintf(fp, "<strong>dvdy</strong>: %.4lf<br>\n", cell.dvdy);
  fprintf(fp, "<strong>dtp</strong>: %.4lf<br>\n", cell.dtp);
  fprintf(fp, "<strong>temperature</strong>: %.1lf<br>\n", cell.temperature);
  fprintf(fp, "<strong>u wind</strong>: %.3lf<br>\n", cell.u_wind);
  fprintf(fp, "<strong>v wind</strong>: %.3lf<br>\n", cell.v_wind);
  fprintf(fp, "]]></description>\n");
  fprintf(fp, "<name>%ld</name>\n", cell.cell_id);
  fprintf(fp, "<LookAt>\n");
  fprintf(fp, "<longitude>%.4lf</longitude>\n", lon[0]);
  fprintf(fp, "<latitude>%.4lf</latitude>\n", lat[0]);
  fprintf(fp, "<range>400000</range>\n");
  fprintf(fp, "<heading>-90</heading>\n");
  fprintf(fp, "</LookAt>\n");
  fprintf(fp, "<visibility>1</visibility>\n");
  fprintf(fp, "<open>1</open>\n");
  write_kml_style_keys(fp);
  fprintf(fp, "<Polygon>\n");
  fprintf(fp, "<extrude>1</extrude>\n");
  fprintf(fp, "<altitudeMode>absolute</altitudeMode>\n");
  fprintf(fp, "<outerBoundaryIs>\n");
  fprintf(fp, "<LinearRing>\n");
  fprintf(fp, "<coordinates>\n");
  
  for (ii=0; ii<=cell.nVertices; ii++)
    fprintf(fp, "%.12f,%.12f,7000\n", lon[ii], lat[ii]);
  
  fprintf(fp, "</coordinates>\n");
  fprintf(fp, "</LinearRing>\n");
  fprintf(fp, "</outerBoundaryIs>\n");
  fprintf(fp, "</Polygon>\n");
  fprintf(fp, "</Placemark>\n");

  return;
}

void rgps_grid2kml(grid_attr_t grid, FILE *fp)
{
  // Write information in kml file
  fprintf(fp, "<Placemark>\n");
  fprintf(fp, "<description><![CDATA[\n");
  fprintf(fp, "<strong>Grid ID</strong>: %ld<br>\n", grid.grid_id);
  fprintf(fp, "<strong>Date</strong>: %s<br>\n", grid.date);
  fprintf(fp, "<strong>Day of the year</strong>: %.4lf<br>\n", grid.day);
  fprintf(fp, "<strong>Grid x</strong>: %.3lf<br>\n", grid.grid_x);
  fprintf(fp, "<strong>Grid y</strong>: %.3lf<br>\n", grid.grid_y);
  fprintf(fp, "<strong>Source image</strong>: %s<br>\n", grid.sourceImage);
  fprintf(fp, "<strong>Target image</strong>: %s<br>\n", grid.targetImage);
  fprintf(fp, "<strong>Stream</strong>: %s<br>\n", grid.stream);
  fprintf(fp, "<strong>Quality</strong>: %d<br>\n", grid.quality);
  fprintf(fp, "]]></description>\n");
  fprintf(fp, "<LookAt>\n");
  fprintf(fp, "<longitude>%9.4f</longitude>\n", grid.lon);
  fprintf(fp, "<latitude>%9.4f</latitude>\n", grid.lat);
  fprintf(fp, "<range>400000</range>\n");
  fprintf(fp, "</LookAt>\n");
  fprintf(fp, "<Style>\n");
  fprintf(fp, "<IconStyle>\n");
  fprintf(fp, "<scale>0.5</scale>\n");
  fprintf(fp, "<Icon>\n");
  fprintf(fp, "<href>grid_point.png</href>\n");
  fprintf(fp, "</Icon>\n");
  fprintf(fp, "</IconStyle>\n");
  fprintf(fp, "</Style>\n");
  fprintf(fp, "<Point>\n");
  fprintf(fp, "<coordinates>%f,%f,0</coordinates>\n", grid.lon, grid.lat);
  fprintf(fp, "</Point>\n");
  fprintf(fp, "</Placemark>\n");
}

void write_dbase_field_to_kml(DBFHandle dbase, int record, 
			      int field, FILE *fp)
{
  DBFFieldType dbaseType;
  char fieldName[25], str[50];
  int nWidth, nDecimals, nValue;
  double fValue;
  const char *sValue;

  dbaseType = DBFGetFieldInfo(dbase, field, fieldName, 
			      &nWidth, &nDecimals);
  switch (dbaseType)
    {
    case FTString:
      sValue = DBFReadStringAttribute(dbase, record, field);
      fprintf(fp, "<strong>%s</strong>: %s<br>\n", 
	      fieldName, sValue);
      break;
    case FTInteger:
      nValue = DBFReadIntegerAttribute(dbase, record, field);
      fprintf(fp, "<strong>%s</strong>: %d<br>\n", 
	      fieldName, nValue);
      break;
    case FTDouble:
      fValue = DBFReadDoubleAttribute(dbase, record, field);
      sprintf(str, "<strong>%%s</strong>: %%%d.%dlf<br>\n", 
	      nWidth, nDecimals);
      fprintf(fp, str, fieldName, fValue);
      break;
    case FTLogical:
    case FTInvalid:
      break;
    }
}

void write_name_field_to_kml(DBFHandle dbase, int record, FILE *fp)
{
  DBFFieldType dbaseType;
  char fieldName[25], str[50];
  int nWidth, nDecimals, nValue;
  double fValue;
  const char *sValue;

  dbaseType = DBFGetFieldInfo(dbase, 0, fieldName, &nWidth, &nDecimals);
  switch (dbaseType)
    {
    case FTString:
      sValue = DBFReadStringAttribute(dbase, record, 0);
      fprintf(fp, "<name>%s</name>\n", sValue);
      break;
    case FTInteger:
      nValue = DBFReadIntegerAttribute(dbase, record, 0);
      fprintf(fp, "<name>%d</name>\n", nValue);
      break;
    case FTDouble:
      fValue = DBFReadDoubleAttribute(dbase, record, 0);
      sprintf(str, "<name>%%%d.%dlf</name>\n", nWidth, nDecimals);
      fprintf(fp, str, fValue);
      break;
    case FTLogical:
    case FTInvalid:
      break;
    }
}

// Convert shape to kml
// In this conversion we only deal with point and polygon shapes 
// that don't have multiple parts. All other cases error out.
void shape2kml(char *line, FILE *fp, char *name)
{
  DBFHandle dbase;
  SHPHandle shape;
  SHPObject *shapeObject;
  int ii, kk, ll, nEntities, nVertices, nParts, *part;
  int nFields, pointType;
  double *lat, *lon;

  // Open shapefile
  open_shape(line, &dbase, &shape);  

  // Extract the vital information out of the shapefile
  SHPGetInfo(shape, &nEntities, &pointType, NULL, NULL);
  switch (pointType)
    {
    case SHPT_POLYGON:
    case SHPT_POINT:
      break;
    case SHPT_ARC: 
      asfPrintError
	("Conversion does not support shape type 'Arc'\n");
      break;
    case SHPT_MULTIPOINT:
      asfPrintError
	("Conversion does not support shape type 'Multipoint'\n");
      break;
    }

  for (ii=0; ii<nEntities; ii++) {

    // Read lat/lon from shape object
    shapeObject = SHPReadObject(shape, ii);
    nVertices = shapeObject->nVertices;
    lat = (double *) MALLOC(sizeof(double)*(nVertices+1));
    lon = (double *) MALLOC(sizeof(double)*(nVertices+1));
    for (kk=0; kk<nVertices; kk++) {
      lat[kk] = shapeObject->padfY[kk];
      lon[kk] = shapeObject->padfX[kk];
    }
    lat[nVertices] = lat[0];
    lon[nVertices] = lon[0];
    nParts = shapeObject->nParts;
    part = (int *) MALLOC(sizeof(int)*(nParts+1));
    for (kk=0; kk<nParts; kk++)
      part[kk] = shapeObject->panPartStart[kk];
    part[nParts] = nVertices;
    SHPDestroyObject(shapeObject);
    if (nParts == 0)
      nParts++;

    // Extract the attributes out of the database file
    nFields = DBFGetFieldCount(dbase);

    for (ll=0; ll<nParts; ll++) {

      // Write information in kml file
      fprintf(fp, "<Placemark>\n");
      fprintf(fp, "<description><![CDATA[\n");
      for (kk=1; kk<nFields; kk++)
	write_dbase_field_to_kml(dbase, ii, kk, fp);
      fprintf(fp, "]]></description>\n");
      write_name_field_to_kml(dbase, ii, fp);
      fprintf(fp, "<LookAt>\n");
      fprintf(fp, "<longitude>%9.4f</longitude>\n", lon[0]);
      fprintf(fp, "<latitude>%9.4f</latitude>\n", lat[0]);
      fprintf(fp, "<range>400000</range>\n");
      fprintf(fp, "</LookAt>\n");
      if (pointType == SHPT_POLYGON) {
          write_kml_style_keys(fp);
	fprintf(fp, "<Polygon>\n");
	fprintf(fp, "<outerBoundaryIs>\n");
	fprintf(fp, "<LinearRing>\n");
	fprintf(fp, "<coordinates>\n");
	for (kk=part[ll]; kk<part[ll+1]; kk++)
	  fprintf(fp, "%.12f,%.12f,4000\n", lon[kk], lat[kk]);
	fprintf(fp, "</coordinates>\n");
	fprintf(fp, "</LinearRing>\n");
	fprintf(fp, "</outerBoundaryIs>\n");
	fprintf(fp, "</Polygon>\n");
      }
      else if (pointType == SHPT_POINT) {
	fprintf(fp, "<Point>\n");
	fprintf(fp, "<coordinates>%.12lf,%.12lf,4000</coordinates>",
		lon[0], lat[0]);
	fprintf(fp, "</Point>\n");
      }
      fprintf(fp, "</Placemark>\n");
    }
  }

  // Close shapefile
  close_shape(dbase, shape);

  return;
}
