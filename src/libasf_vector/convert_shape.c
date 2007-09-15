#include "asf.h"
#include "dateUtil.h"
#include "shapefil.h"
#include "asf_vector.h"

// Convert metadata to shape
void meta2shape(char *line, DBFHandle dbase, SHPHandle shape, int n)
{
  meta_parameters *meta;
  double lat[5], lon[5];

  // Read metadata
  meta = meta_read(line);

  // Determine corner coordinates
  meta_get_latLon(meta, 0, 0, 0.0, &lat[0], &lon[0]);
  meta_get_latLon(meta, 0, meta->general->sample_count, 0.0, 
		  &lat[1], &lon[1]);
  meta_get_latLon(meta, meta->general->line_count, meta->general->sample_count,
                  0.0, &lat[2], &lon[2]);
  meta_get_latLon(meta, meta->general->line_count, 0, 0.0, 
		  &lat[3], &lon[3]);
  lat[4] = lat[0];
  lon[4] = lon[0];	

  // Write information into database file
  DBFWriteStringAttribute(dbase, n, 0, meta->general->sensor);
  DBFWriteStringAttribute(dbase, n, 1, meta->general->sensor_name);
  DBFWriteStringAttribute(dbase, n, 2, meta->general->mode);
  if (meta->sar)
    DBFWriteStringAttribute(dbase, n, 3, meta->sar->polarization);
  DBFWriteIntegerAttribute(dbase, n, 4, meta->general->orbit);
  DBFWriteIntegerAttribute(dbase, n, 5, meta->general->frame);
  DBFWriteStringAttribute(dbase, n, 6, meta->general->acquisition_date);
  if (meta->general->orbit_direction == 'D')
    DBFWriteStringAttribute(dbase, n, 7, "Descending");
  else if (meta->general->orbit_direction == 'A')
    DBFWriteStringAttribute(dbase, n, 7, "Ascending");
  DBFWriteDoubleAttribute(dbase, n, 8, meta->general->center_latitude);
  DBFWriteDoubleAttribute(dbase, n, 9, meta->general->center_longitude);
  DBFWriteDoubleAttribute(dbase, n, 10, lat[0]);
  DBFWriteDoubleAttribute(dbase, n, 11, lon[0]);
  DBFWriteDoubleAttribute(dbase, n, 12, lat[1]);
  DBFWriteDoubleAttribute(dbase, n, 13, lon[1]);
  DBFWriteDoubleAttribute(dbase, n, 14, lat[2]);
  DBFWriteDoubleAttribute(dbase, n, 15, lon[2]);
  DBFWriteDoubleAttribute(dbase, n, 16, lat[3]);
  DBFWriteDoubleAttribute(dbase, n, 17, lon[3]);
 
  // Write shape object
  SHPObject *shapeObject=NULL;
  shapeObject = SHPCreateSimpleObject(SHPT_POLYGON, 5, lon, lat, NULL);
  SHPWriteObject(shape, -1, shapeObject);
  SHPDestroyObject(shapeObject);

  meta_free(meta);

  return;
}

// Convert kml to shape

// Convert point to shape
void point2shape(char *line, DBFHandle dbase, SHPHandle shape, int n)
{
  double lat, lon;
  char id[255], *p;

  // Read ID and lat/lon;
  p = strchr(line, ',');
  if (p) {
    sscanf(p+1, "%lf,%lf", &lat, &lon);
    *p = '\0';
    sprintf(id, "%s", line);
    *p = ',';
  }

  // Write information into database file
  DBFWriteStringAttribute(dbase, n, 0, id);
  DBFWriteDoubleAttribute(dbase, n, 1, lat);
  DBFWriteDoubleAttribute(dbase, n, 2, lon);
  
  // Write shape object
  SHPObject *shapeObject=NULL;
  shapeObject = SHPCreateSimpleObject(SHPT_POINT, 1, &lon, &lat, NULL);
  SHPWriteObject(shape, -1, shapeObject);
  SHPDestroyObject(shapeObject);

  return;
}

// Convert polygon to shape
void polygon2shape(char *line, DBFHandle dbase, SHPHandle shape, int n)
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
    line = strchr(p+1, ',');
  }

  // Read coordinates of the vertices
  lat = (double *) MALLOC(sizeof(double)*(vertices+1));
  lon = (double *) MALLOC(sizeof(double)*(vertices+1));
  p_lat = line;
  for (ii=0; ii<vertices; ii++) {
    sscanf(p_lat+1, "%lf", &lat[ii]);
    p_lon = strchr(p_lat+1, ',');
    sscanf(p_lon+1, "%lf", &lon[ii]);
    p_lat = strchr(p_lon+1, ',');
  } 
  lat[vertices] = lat[0];
  lon[vertices] = lon[0];

  // Write information into database file
  DBFWriteStringAttribute(dbase, n, 0, id);
  DBFWriteIntegerAttribute(dbase, n, 1, vertices);
  
  // Write shape object
  SHPObject *shapeObject=NULL;
  shapeObject = SHPCreateSimpleObject(SHPT_POLYGON, vertices+1, lon, lat, NULL);
  SHPWriteObject(shape, -1, shapeObject);
  SHPDestroyObject(shapeObject);

  FREE(lat);
  FREE(lon);

  return;
}

// Convert RGPS cell to shape
void rgps2shape(cell_t cell, double *lat, double *lon, int vertices,
		DBFHandle dbase, SHPHandle shape, int n)
{
  // Write information into database file
  DBFWriteIntegerAttribute(dbase, n, 0, cell.cell_id);
  DBFWriteIntegerAttribute(dbase, n, 1, cell.nVertices);
  DBFWriteStringAttribute(dbase, n, 2, cell.date);
  DBFWriteStringAttribute(dbase, n, 3, cell.sourceImage);
  DBFWriteStringAttribute(dbase, n, 4, cell.targetImage);
  DBFWriteStringAttribute(dbase, n, 5, cell.stream);
  DBFWriteDoubleAttribute(dbase, n, 6, cell.area);
  DBFWriteDoubleAttribute(dbase, n, 7, cell.multi_year_ice);
  DBFWriteDoubleAttribute(dbase, n, 8, cell.open_water);
  DBFWriteDoubleAttribute(dbase, n, 9, cell.incidence_angle);
  DBFWriteDoubleAttribute(dbase, n, 10, cell.cell_x);
  DBFWriteDoubleAttribute(dbase, n, 11, cell.cell_y);
  DBFWriteDoubleAttribute(dbase, n, 12, cell.dudx);
  DBFWriteDoubleAttribute(dbase, n, 13, cell.dudy);
  DBFWriteDoubleAttribute(dbase, n, 14, cell.dvdx);
  DBFWriteDoubleAttribute(dbase, n, 15, cell.dvdy);
  DBFWriteDoubleAttribute(dbase, n, 16, cell.dtp);
  DBFWriteDoubleAttribute(dbase, n, 17, cell.temperature);
  DBFWriteDoubleAttribute(dbase, n, 18, cell.u_wind);
  DBFWriteDoubleAttribute(dbase, n, 19, cell.v_wind);
  
  // Write shape object
  SHPObject *shapeObject=NULL;
  shapeObject = SHPCreateSimpleObject(SHPT_POLYGON, vertices+1,
				      lon, lat, NULL);
  if (shapeObject == NULL)
    asfPrintError("Could not create shape object (%d)\n", n);
  SHPWriteObject(shape, -1, shapeObject);
  SHPDestroyObject(shapeObject);

  return;
}

// Convert RGPS grid points to shape
void rgps_grid2shape(grid_attr_t grid, DBFHandle dbase, SHPHandle shape, int n)
{
  // Write information into database file
  DBFWriteIntegerAttribute(dbase, n, 0, grid.grid_id);
  DBFWriteStringAttribute(dbase, n, 1, grid.date);
  DBFWriteDoubleAttribute(dbase, n, 2, grid.day);
  DBFWriteDoubleAttribute(dbase, n, 3, grid.grid_x);
  DBFWriteDoubleAttribute(dbase, n, 4, grid.grid_y);
  DBFWriteStringAttribute(dbase, n, 5, grid.sourceImage);
  DBFWriteStringAttribute(dbase, n, 6, grid.targetImage);
  DBFWriteStringAttribute(dbase, n, 7, grid.stream);
  DBFWriteIntegerAttribute(dbase, n, 8, grid.quality);

  // Write shape object
  SHPObject *shapeObject=NULL;
  shapeObject = SHPCreateSimpleObject(SHPT_POINT, 1, 
				      &grid.lon, &grid.lat, NULL);
  if (shapeObject == NULL)
    asfPrintError("Could not create shape object (%d)\n", n);
  SHPWriteObject(shape, -1, shapeObject);
  SHPDestroyObject(shapeObject);

  return;
}

// Convert RGPS weather data to shape
void rgps_weather2shape(char *line, DBFHandle dbase, SHPHandle shape, int n)
{
  double lat, lon, direction, speed, temperature, pressure;
  char date[15], *p;

  // Read weather information;
  p = strchr(line, ',');
  if (p) {
    sscanf(p+1, "%lf,%lf,%lf,%lf,%lf,%lf", 
	   &lat, &lon, &direction, &speed, &temperature, &pressure);
    *p = 0;
    sprintf(date, "%s", line);
  }

  // Write information into database file
  DBFWriteStringAttribute(dbase, n, 0, date);
  DBFWriteDoubleAttribute(dbase, n, 1, lat);
  DBFWriteDoubleAttribute(dbase, n, 2, lon);
  DBFWriteDoubleAttribute(dbase, n, 3, direction);
  DBFWriteDoubleAttribute(dbase, n, 4, speed);
  DBFWriteDoubleAttribute(dbase, n, 5, temperature);
  DBFWriteDoubleAttribute(dbase, n, 6, pressure);

  // Write shape object
  SHPObject *shapeObject=NULL;
  shapeObject = SHPCreateSimpleObject(SHPT_POINT, 1, &lon, &lat, NULL);
  SHPWriteObject(shape, -1, shapeObject);
  SHPDestroyObject(shapeObject);

  return;
}

// Convert multimatch to shape file
void multimatch2shape(char *line, DBFHandle dbase, SHPHandle shape, int n)
{
  double lat, lon, ref_x, ref_y, ref_z, search_x, search_y, search_z;
  double dx, dy, dz, direction, speed;

  // Read information from line
  sscanf(line, "%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf",
	 &lat, &lon, &ref_x, &ref_y, &ref_z, &search_x, &search_y, &search_z,
	 &dx, &dy, &dz, &direction, &speed);

  // Write information into database file
  DBFWriteDoubleAttribute(dbase, n, 0, ref_x);
  DBFWriteDoubleAttribute(dbase, n, 1, ref_y);
  DBFWriteDoubleAttribute(dbase, n, 2, ref_z);
  DBFWriteDoubleAttribute(dbase, n, 3, search_x);
  DBFWriteDoubleAttribute(dbase, n, 4, search_y);
  DBFWriteDoubleAttribute(dbase, n, 5, search_z);
  DBFWriteDoubleAttribute(dbase, n, 6, dx);
  DBFWriteDoubleAttribute(dbase, n, 7, dy);
  DBFWriteDoubleAttribute(dbase, n, 8, dz);
  DBFWriteDoubleAttribute(dbase, n, 9, direction);
  DBFWriteDoubleAttribute(dbase, n, 10, speed);

  // Write shape object
  SHPObject *shapeObject=NULL;
  shapeObject = SHPCreateSimpleObject(SHPT_POINT, 1, &lon, &lat, NULL);
  SHPWriteObject(shape, -1, shapeObject);
  SHPDestroyObject(shapeObject);

  return;
}

// Convert shapefile to text file - general dump function
void shape2text(char *inFile, FILE *fp)
{
  DBFHandle dbase;
  SHPHandle shape;
  DBFFieldType dbaseType;
  SHPObject *shapeObject;
  char fieldName[25], str[50];
  int ii, kk, nEntities, nVertices, nParts, iPart;
  int nFields, nWidth, nDecimals, nValue, pointType;
  double fValue;
  const char *sValue;

  // Open shapefile
  open_shape(inFile, &dbase, &shape);  
  fprintf(fp, "NAME OF SHAPEFILE: %s\n", inFile);

  // Extract the vital information out of the shapefile
  SHPGetInfo(shape, &nEntities, &pointType, NULL, NULL);
  fprintf(fp, "Number of structures: %d\n", nEntities);
  switch (pointType)
    {
    case SHPT_POLYGON:
      fprintf(fp, "Shape type: Polygon\n\n");
      break;
    case SHPT_POINT:
      fprintf(fp, "Shape type: Point\n\n");
      break;
    case SHPT_ARC:
      asfPrintWarning("Shape type 'Arc' currently not supported\n\n");
      break;
    case SHPT_MULTIPOINT:
      asfPrintWarning("Shape type 'Multipoint' currently not supported\n\n");
      break;
    }
  for (ii=0; ii<nEntities; ii++) {

    // Read object and report basic information
    shapeObject = SHPReadObject(shape, ii);
    nParts = shapeObject->nParts;
    nVertices = shapeObject->nVertices;
    fprintf(fp, "Structure: %d\n", ii+1);
    if (nParts > 1)
      fprintf(fp, "Number of parts: %d\n", nParts);
    for (iPart=0; iPart<nParts; iPart++) {
      nVertices = shapeObject->panPartStart[iPart+1] -
	shapeObject->panPartStart[iPart];
      for (kk=0; kk<nVertices; kk++) {
	if (nParts > 1 && kk == 0) {
	  fprintf(fp, "\nPart: %d\n", iPart+1);
	}
	if (iPart < nParts && shapeObject->panPartStart[iPart] == kk)
	  fprintf(fp, "Number of vertices: %d\n", nVertices);
	fprintf(fp, "%d - Lat: %.4lf, Lon: %.4lf\n", 
		kk, shapeObject->padfY[kk], shapeObject->padfX[kk]);
      }
    }
    SHPDestroyObject(shapeObject);
    fprintf(fp, "\n");

    // Extract the attributes out of the database file
    nFields = DBFGetFieldCount(dbase);
    fprintf(fp, "Number of fields: %d\n", nFields);
    sValue = (char *) MALLOC(sizeof(char)*255);
    for (kk=0; kk<nFields; kk++) {
      dbaseType = DBFGetFieldInfo(dbase, kk, fieldName, &nWidth, &nDecimals);
      switch (dbaseType)
	{
	case FTString:
	  sValue = DBFReadStringAttribute(dbase, ii, kk);
	  fprintf(fp, "%s: %s\n", fieldName, sValue);
	  break;
	case FTInteger:
	  nValue = DBFReadIntegerAttribute(dbase, ii, kk);
	  fprintf(fp, "%s: %d\n", fieldName, nValue);
	  break;
	case FTDouble:
	  fValue = DBFReadDoubleAttribute(dbase, ii, kk);
	  sprintf(str, "%%s: %%%d.%dlf\n", nWidth, nDecimals);
	  fprintf(fp, str, fieldName, fValue);
	  break;
	case FTLogical:
	case FTInvalid:
	  break;
	}
    }
    fprintf(fp, "\n");
  }

  // Close shapefile
  close_shape(dbase, shape);

  return;
}
