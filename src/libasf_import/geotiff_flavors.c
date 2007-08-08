// Implementation of the interface described in geotiff_flavors.h.

#include <assert.h>
#include <sys/types.h>
#include <regex.h>

#include <geokeys.h>
#include <geo_tiffp.h>
#include <geo_keyp.h>
#include <geotiff.h>
#include <geotiffio.h>
#include <tiff.h>
#include <tiffio.h>
#include <xtiffio.h>

#include <asf.h>
#include <asf_meta.h>
#include <proj.h>
#include "asf_import.h"
#include "geotiff_flavors.h"
#include "arcgis_geotiff_support.h"
#include "import_generic_geotiff.h"
#include "find_arcgis_geotiff_aux_name.h"

geotiff_importer
detect_geotiff_flavor (const char *file)
{
  GString *inGeotiffAuxName;

  // Open the Geotiff.
  TIFF *tiff = XTIFFOpen (file, "r");
  asfRequire (tiff != NULL, "Error opening input TIFF file.\n");
  GTIF *gtif = GTIFNew (tiff);
  asfRequire (gtif != NULL,
	      "Error reading GeoTIFF keys from input TIFF file.\n");

  // Grab the citation.
  size_t max_citation_length = 10000;
  char *citation = MALLOC ((max_citation_length + 1) * sizeof (char));
  GTIFKeyGet (gtif, GTCitationGeoKey, citation, 0, max_citation_length);
  // Ensure the citation is at least eventually terminated somewhere.
  citation[max_citation_length] = '\0';

  // Test for a particular flavor.
  const char *tmp = "IMAGINE GeoTIFF Support";
  if ( strncmp (citation, tmp, strlen (tmp)) == 0 ) {
    short model_type;
    short proj_type;
    asfPrintStatus("\nFound IMAGINE GeoTIFF (ArcGIS, USGS Seamless etc) type of GeoTIFF.\n");

    int read_count
      = GTIFKeyGet (gtif, GTModelTypeGeoKey, &model_type, 0, 1);
    asfRequire (read_count == 1, "GTIFKeyGet failed.\n");
    if ( model_type == ModelTypeGeographic ) {
      FREE(citation);
      GTIFFree(gtif);
      XTIFFClose(tiff);
      return import_usgs_seamless;
    }
    // If the TIFF claims that a projection exists OR if the projection type
    // is unknown, then check for an ArcGIS metadata (.aux) file since it may
    // contain the needed info.
    //
    // If the tiff turns out to be an ArcGIS GeoTIFF and the file(s) contain
    // a supported projection type, then return the arcgis importer...
    else if ( model_type == ModelTypeProjected ||
              (model_type != ModelTypeGeographic && model_type != ModelTypeGeocentric)
            ) {
      char inBaseName[256];
      strcpy(inBaseName, file);
      char *ext = findExt(inBaseName);
      if (ext) {
        *ext = '\0';
      }
      inGeotiffAuxName = find_arcgis_geotiff_aux_name(inBaseName);
      if ( inGeotiffAuxName != NULL ) {
        proj_type = getArcgisProjType (inGeotiffAuxName->str);
        asfPrintStatus("Found ERDAS IMAGINE / ESRI ArcGIS type of GeoTIFF\n");
        switch (proj_type) {
          case UTM:     // Universal Transverse Mercator (UTM)
          case ALBERS:  // Albers Equal Area Conic (aka Albers Conical Equal Area)
          case LAMCC:   // Lambert Conformal Conic
          case PS:      // Polar Stereographic
          case LAMAZ:   // Lambert Azimuthal Equal Area
            g_string_free(inGeotiffAuxName, TRUE);
            FREE(citation);
            GTIFFree(gtif);
            XTIFFClose(tiff);
            return import_generic_geotiff;
            break;
          case DHFA_UNKNOWN_PROJECTION:
          default:      // Else cont...
            asfPrintWarning("Unable to determine projection type from\n"
                "ArcGIS metadata (.aux) file.  Attempting the ingest without it...\n");
            g_string_free(inGeotiffAuxName, TRUE);
            FREE(citation);
            GTIFFree(gtif);
            XTIFFClose(tiff);
            return import_generic_geotiff;
            break;
        }
      }
      else {
        asfPrintWarning("Found ERDAS IMAGINE / ESRI ArcGIS type GeoTIFF but the\n"
            "associated metadata (.aux) file appears to be missing.  The TIFF file\n"
            "will be examined for this data instead...\n");
        return import_generic_geotiff;
      }
    }
  } // strncmp on citation, tmp looking for IMAGINE GeoTIFF type
  else {
    return import_generic_geotiff;
  }

  // Test for a particular flavor.
  //GTIFKeyGet (gtif, PCSCitationGeoKey, citation, 0, max_citation_length);
  // Ensure the citation is at least eventually terminated somewhere.
  //citation[max_citation_length] = '\0';
  //regex_t asf_utm_citation_regex;
  /*
  int return_code
    = regcomp (&asf_utm_citation_regex,
	       "UTM zone ? projected GeoTIFF on WGS84 ellipsoid datum written "
	       "by Alaska Satellite Facility tools.", REG_NOSUB);
  */
  //int return_code
    //= regcomp (&asf_utm_citation_regex,
	//       "UTM zone [[:digit:]]+ [NS]", REG_EXTENDED | REG_NOSUB);
  //[[:digit:]]+ [NS]
  //assert (return_code == 0);
  //return_code = regexec (&asf_utm_citation_regex, citation, 0, NULL, 0);
  //if ( return_code == 0 ) {
    //GTIFFree(gtif);
    //XTIFFClose(tiff);
    //return import_asf_utm_geotiff;
  //}

  // Couldn't determine any flavor we know.
  FREE(citation);
  GTIFFree(gtif);
  XTIFFClose(tiff);
  return NULL;
}
