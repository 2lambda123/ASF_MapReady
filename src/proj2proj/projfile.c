#include "proj2proj.h"
#include <ctype.h>
#include "asf_nan.h"
#include "cla.h"
#include "asf.h"
#include "asf_meta.h"
#include "libasf_proj.h"

static GtkWidget * source_utm_menu = NULL;
static GtkWidget * source_ps_menu = NULL;
static GtkWidget * source_albers_menu = NULL;
static GtkWidget * source_lamcc_menu = NULL;
static GtkWidget * source_lamaz_menu = NULL;
static GtkWidget * source_latlon_menu = NULL;

static GtkWidget * target_utm_menu = NULL;
static GtkWidget * target_ps_menu = NULL;
static GtkWidget * target_albers_menu = NULL;
static GtkWidget * target_lamcc_menu = NULL;
static GtkWidget * target_lamaz_menu = NULL;
static GtkWidget * target_latlon_menu = NULL;

static datum_type_t get_datum(FILE *fp);
static spheroid_type_t get_spheroid(FILE *fp);

static char * projection_directory(int projection)
{
    char * location = NULL, * ret;

    switch (projection)
    {
    case PROJ_UTM:
        location = "utm";
        break;

    case PROJ_PS:
        location = "polar_stereographic";
        break;

    case PROJ_LAMCC:
        location = "lambert_conformal_conic";
        break;

    case PROJ_LAMAZ:
        location = "lambert_azimuthal_equal_area";
        break;

    case PROJ_ALBERS:
        location = "albers_equal_area_conic";
        break;
    }

    ret = (char *) malloc(sizeof(char) *
        (strlen(location) + strlen(get_asf_share_dir()) + 25));

    sprintf(ret, "%s%cprojections%c%s", get_asf_share_dir(),  DIR_SEPARATOR,
        DIR_SEPARATOR, location);

    return ret;
}

static int my_strcmp(const void *s1, const void *s2)
{
    char * string1 = * (char**)s1;
    char * string2 = * (char**)s2;

    if (strncmp(string1, "utm", 3) == 0 && strncmp(string2, "utm", 3) == 0)
    {
        int utm_zone1, utm_zone2;
        sscanf(string1, "utm_%d", &utm_zone1);
        sscanf(string2, "utm_%d", &utm_zone2);

        if (utm_zone1 == utm_zone2)
        {
            return *(string1 + strlen(string1) - 1) == 'N';
        }
        else
        {
            return utm_zone1 > utm_zone2;
        }
    }
    else
    {
        return g_ascii_strcasecmp(string1, string2);
    }
}

static const char * projection_file_prefix(int projection)
{
    switch (projection)
    {
    default:
    case PROJ_UTM:
        return "utm_";

    case PROJ_PS:
        return "polar_stereographic_";

    case PROJ_LAMCC:
        return "lambert_conformal_conic_";

    case PROJ_LAMAZ:
        return "lambert_azimuthal_equal_area_";

    case PROJ_ALBERS:
        return "albers_equal_area_conic_";
    }
}

static char * fudge_the_name(int projection, const char * name)
{
    static char buf[256];

    if (projection == PROJ_UTM)
    {
        strcpy(buf, name);
    }
    else
    {
        const char * prefix;
        prefix = projection_file_prefix(projection);

        if (strncmp(name, prefix, strlen(prefix)) == 0)
        {
            const char * p = name + strlen(prefix);
            strcpy(buf, p);
        }
        else
        {
            strcpy(buf, name);
        }
    }

    return buf;
}

static GtkWidget * populate_predefined_projections(int projection)
{
    gchar * proj_dir;
    GDir * dir;
    GtkWidget * m;
    GtkWidget * item;

    m = gtk_menu_new();

    item = gtk_menu_item_new_with_label("User Defined");
    gtk_menu_append(GTK_MENU(m), item);
    gtk_widget_show(item);

    item = gtk_separator_menu_item_new();
    gtk_menu_append(GTK_MENU(m), item);
    gtk_widget_show(item);

    // do not populate the predefined projections for UTM -- too many,
    // the large dropdownlist causes crashes on windows. Also, no
    // predefined projections for the lat/lon psuedoprojection
    if (projection != PROJ_UTM && projection != PROJ_LATLON) {
      proj_dir = projection_directory(projection);
      if (proj_dir)
      {
        dir = g_dir_open(proj_dir, 0, NULL);
        
        if (dir)
        {
          int i, n;
          char *names[512];
          
          n = 0;
          while (TRUE)
          {
            const char * name;
            
            name = (char*) g_dir_read_name(dir);
            
            if (name)
            {
              char * name_dup;
              char * p;
              
              name_dup = STRDUP(name);
              p = strrchr(name_dup, '.');
              
              if (p && strcmp(p, ".proj") == 0)
              {
                *p = '\0';
                
                names[n] = name_dup;
                ++n;
                
                if (n >= sizeof(names)/sizeof(names[0]))
                  break;
              }
            }
            else
            {
              break;
            }
          }
          
          g_dir_close(dir);
          
          qsort(names, n, sizeof(char *), my_strcmp);
          for (i = 0; i < n; ++i)
          {
            item = gtk_menu_item_new_with_label(
              fudge_the_name(projection, names[i]));
            
            g_object_set_data(G_OBJECT(item),
                              "file", (gpointer)names[i]);
            
            gtk_menu_append(GTK_MENU(m), item);
            gtk_widget_show(item);
          }
        }
      }
    }

    if (proj_dir)
      g_free(proj_dir);

    return m;
}

static const char * bracketed_projection_name(projection_type_t proj_type)
{
    switch (proj_type)
    {
    case PROJ_UTM:
        return "[Universal Transverse Mercator]";

    case PROJ_PS:
        return "[Polar Stereographic]";

    case PROJ_ALBERS:
        return "[Albers Conical Equal Area]";

    case PROJ_LAMAZ:
        return "[Lambert Azimuthal Equal Area]";

    case PROJ_LAMCC:
        return "[Lambert Conformal Conic]";

    default:
        return MAGIC_UNSET_STRING;
    }
}

static int previous_projection = -1;

void release_predefined_projections()
{
    if (previous_projection != -1)
    {
        g_object_unref(source_utm_menu);
        g_object_unref(source_latlon_menu);
        g_object_unref(source_ps_menu);
        g_object_unref(source_lamcc_menu);
        g_object_unref(source_lamaz_menu);
        g_object_unref(source_albers_menu);

        g_object_unref(target_utm_menu);
        g_object_unref(target_latlon_menu);
        g_object_unref(target_ps_menu);
        g_object_unref(target_lamcc_menu);
        g_object_unref(target_lamaz_menu);
        g_object_unref(target_albers_menu);
    }
}

void set_predefined_projections(int is_source, int projection)
{
    GtkWidget * predefined_projection_option_menu;
    GtkWidget * menu = NULL;

    /* looking through all the files can be slow, skip it if we can */
    if (projection == previous_projection)
      return;

    if (previous_projection == -1)
    {
        /* populate all the predefined projection menus */
        source_utm_menu =
            populate_predefined_projections(PROJ_UTM);
        target_utm_menu =
            populate_predefined_projections(PROJ_UTM);

        source_latlon_menu =
            populate_predefined_projections(PROJ_LATLON);
        target_latlon_menu =
            populate_predefined_projections(PROJ_LATLON);

        source_ps_menu =
            populate_predefined_projections(PROJ_PS);
        target_ps_menu =
            populate_predefined_projections(PROJ_PS);

        source_lamcc_menu =
            populate_predefined_projections(PROJ_LAMCC);
        target_lamcc_menu =
            populate_predefined_projections(PROJ_LAMCC);

        source_lamaz_menu =
            populate_predefined_projections(PROJ_LAMAZ);
        target_lamaz_menu =
            populate_predefined_projections(PROJ_LAMAZ);

        source_albers_menu =
            populate_predefined_projections(PROJ_ALBERS);
        target_albers_menu =
            populate_predefined_projections(PROJ_ALBERS);

        g_object_ref(source_latlon_menu);
        g_object_ref(source_utm_menu);
        g_object_ref(source_ps_menu);
        g_object_ref(source_lamcc_menu);
        g_object_ref(source_lamaz_menu);
        g_object_ref(source_albers_menu);

        g_object_ref(target_latlon_menu);
        g_object_ref(target_utm_menu);
        g_object_ref(target_ps_menu);
        g_object_ref(target_lamcc_menu);
        g_object_ref(target_lamaz_menu);
        g_object_ref(target_albers_menu);
    }

    predefined_projection_option_menu = is_source ?
      get_widget_checked("source_predefined_projection_option_menu") :
      get_widget_checked("target_predefined_projection_option_menu");

    switch (projection)
    {
    case PROJ_UTM:
        menu = is_source ? source_utm_menu : target_utm_menu;
        break;

    case PROJ_LATLON:
        menu = is_source ? source_latlon_menu : target_latlon_menu;
        break;

    case PROJ_PS:
        menu = is_source ? source_ps_menu : target_ps_menu;
        break;

    case PROJ_LAMCC:
        menu = is_source ? source_lamcc_menu : target_lamcc_menu;
        break;

    case PROJ_LAMAZ:
        menu = is_source ? source_lamaz_menu : target_lamaz_menu;
        break;

    case PROJ_ALBERS:
        menu = is_source ? source_albers_menu : target_albers_menu;
        break;
    }

    g_assert(menu);

    previous_projection = projection;
    gtk_option_menu_set_menu(
        GTK_OPTION_MENU(predefined_projection_option_menu), menu);

    gtk_option_menu_set_history(
        GTK_OPTION_MENU(predefined_projection_option_menu), 0);

    gtk_widget_show(menu);
    gtk_widget_show(predefined_projection_option_menu);
}

static void readline(FILE * f, char * buffer, size_t n)
{
    char * p;
    char * newline;

    p = fgets(buffer, n, f);

    if (!p)
    {
        strcpy(buffer, "");
    }
    else
    {
        newline = strrchr(buffer, '\n');
        if (newline)
            *newline = '\0';
    }
}

static int parse_val(char * inbuf, char * key, double * val)
{
    char * p, * eq, * buf;
    int match = FALSE;

    buf = g_strdup(inbuf);

    p = eq = strchr(buf, '=');
    if (!eq)
        return FALSE;

    *eq = '\0';
    --p;

    while (isspace((int)(*p)))
        *p-- = '\0';

    if (g_ascii_strcasecmp(buf, key) == 0)
    {
        p = eq + 1;
        while (isspace((int)(*p)))
            ++p;

        if (*p)
        {
            double d;
            if (parse_double(p, &d))
            {
                *val = d;
                match = TRUE;
            }
            else
            {
                *val = MAGIC_UNSET_DOUBLE;
            }
        }
    }

    g_free(buf);
    return match;
}

static void get_fields(FILE * fp, ...)
{
    va_list ap;
    char * keys[32];
    double * vals[32];
    char buf[256];
    unsigned int nkeys = 0;

    va_start(ap, fp);
    while (nkeys < sizeof(keys))
    {
        keys[nkeys] = va_arg(ap, char *);
        if (!keys[nkeys])
            break;

        vals[nkeys] = va_arg(ap, double *);
        ++nkeys;
    }
    va_end(ap);

    while (!feof(fp))
    {
        unsigned int i;
        int found = FALSE;

        readline(fp, buf, sizeof(buf));

        if (strlen(buf) > 0)
        {
            for (i = 0; i < nkeys; ++i)
            {
                if (parse_val(buf, keys[i], vals[i]))
                {
                    found = TRUE;
                    break;
                }
            }
        }
    }
}

static int parse_proj_args_file(char * file, project_parameters_t * pps,
                                projection_type_t * proj_type,
                                datum_type_t *datum)
{
    FILE * fp;
    char buf[256];
    int ret;

    fp = fopen(file, "rt");
    if (!fp)
    {
        return FALSE;
    }

    readline(fp, buf, sizeof(buf));
    ret = TRUE;

    *datum = WGS84_DATUM;
    if (strcmp(buf, bracketed_projection_name(PROJ_ALBERS)) == 0 ||
        strcmp(buf, "[Albers Equal Area Conic]") == 0)
    {
        *proj_type = PROJ_ALBERS;
        get_fields(fp,
            "First standard parallel", &pps->albers.std_parallel1,
            "Second standard parallel", &pps->albers.std_parallel2,
            "Central Meridian", &pps->albers.center_meridian,
            "Latitude of Origin", &pps->albers.orig_latitude,
            "False Easting", &pps->albers.false_easting,
            "False Northing", &pps->albers.false_northing,
            NULL);
        *datum = get_datum(fp);
    }
    else if (strcmp(buf, bracketed_projection_name(PROJ_LAMAZ)) == 0)
    {
        *proj_type = PROJ_LAMAZ;
        get_fields(fp,
            "Central Meridian", &pps->lamaz.center_lon,
            "Latitude of Origin", &pps->lamaz.center_lat,
            "False Easting", &pps->lamaz.false_easting,
            "False Northing", &pps->lamaz.false_northing,
            NULL);
        *datum = get_datum(fp);
    }
    else if (strcmp(buf, bracketed_projection_name(PROJ_LAMCC)) == 0)
    {
        *proj_type = PROJ_LAMCC;
        get_fields(fp,
            "First standard parallel", &pps->lamcc.plat1,
            "Second standard parallel", &pps->lamcc.plat2,
            "Central Meridian", &pps->lamcc.lon0,
            "Latitude of Origin", &pps->lamcc.lat0,
            "False Easting", &pps->lamcc.false_easting,
            "False Northing", &pps->lamcc.false_northing,
            /* "Scale Factor", &pps->lamcc.scale_factor, */
            NULL);
        *datum = get_datum(fp);
    }
    else if (strcmp(buf, bracketed_projection_name(PROJ_PS)) == 0)
    {
        double is_north_pole;
        spheroid_type_t spheroid;
        *proj_type = PROJ_PS;
        get_fields(fp,
            "First standard parallel", &pps->ps.slat,
            "Standard Parallel", &pps->ps.slat,
            "Central Meridian", &pps->ps.slon,
            "False Easting", &pps->ps.false_easting,
            "False Northing", &pps->ps.false_northing,
            "Northern Projection", &is_north_pole,
            NULL);
        pps->ps.is_north_pole = (int) is_north_pole;
        spheroid = get_spheroid(fp);
        switch(spheroid) {
          case WGS84_SPHEROID:
            *datum = WGS84_DATUM;
            break;
          case HUGHES_SPHEROID:
            *datum = HUGHES_DATUM;
            break;
          case BESSEL_SPHEROID:
          case CLARKE1866_SPHEROID:
          case CLARKE1880_SPHEROID:
          case GEM6_SPHEROID:
          case GEM10C_SPHEROID:
          case GRS1980_SPHEROID:
          case INTERNATIONAL1924_SPHEROID:
          case INTERNATIONAL1967_SPHEROID:
          case WGS72_SPHEROID:
          default:
            asfPrintWarning("Unknown, unrecognized, or unsupported reference ellipsoid found for.\n"
                "a polar stereographic projection.\n");
            *datum = UNKNOWN_DATUM;
        }
    }
    else if (strcmp(buf, bracketed_projection_name(PROJ_UTM)) == 0)
    {
        double zone;
        *proj_type = PROJ_UTM;
        get_fields(fp,
            "Scale Factor", &pps->utm.scale_factor,
            "Central Meridian", &pps->utm.lon0,
            "Latitude of Origin", &pps->utm.lat0,
            "False Easting", &pps->utm.false_easting,
            "False Northing", &pps->utm.false_northing,
            "Zone", &zone,
            NULL);
        pps->utm.zone = (int) zone;
        *datum = WGS84_DATUM;
    }
    else
    {
        ret = FALSE;
    }

    fclose(fp);
    return ret;
}

static int out_of_sync(const char * filename, int projection)
{
    /* kludge: sometimes attempt to load a predefined projection for the
    projection type that is no longer selected */
    const char * prefix = projection_file_prefix(projection);
    return strncmp(prefix, filename, strlen(prefix)) != 0;
}

project_parameters_t *
load_selected_predefined_projection_parameters(int is_source, int projection,
                                               datum_type_t *datum)
{
    GtkWidget * predefined_projection_option_menu;
    GtkWidget * menu;
    GtkWidget * selected_item;
    gchar filename[256];
    gchar * path;
    gchar * path_and_filename;
    project_parameters_t * ret;
    projection_type_t type;

    predefined_projection_option_menu = is_source ?
      get_widget_checked("source_predefined_projection_option_menu") :
      get_widget_checked("target_predefined_projection_option_menu");

    menu =
        gtk_option_menu_get_menu(
        GTK_OPTION_MENU(predefined_projection_option_menu));

    selected_item =
        gtk_menu_get_active(GTK_MENU(menu));

    sprintf(filename, "%s.proj", (char *)g_object_get_data(
        G_OBJECT(selected_item), "file"));

    if (out_of_sync(filename, projection))
        return NULL;

    path = projection_directory(projection);

    path_and_filename = (gchar *)
        g_malloc(sizeof(gchar *) * (strlen(path) + strlen(filename) + 5));

    sprintf(path_and_filename, "%s/%s", path, filename);

    ret = (project_parameters_t *) g_malloc(sizeof(project_parameters_t));
    if (!parse_proj_args_file(path_and_filename, ret, &type, datum))
    {
        gchar * tmp = (gchar *)
            g_malloc(sizeof(gchar *) * (strlen(path_and_filename) + 100));

        sprintf(tmp, "Error opening .proj file: %s\n", path_and_filename);
        message_box(tmp);
        g_free(tmp);

        return NULL;
    }

    g_free(path);
    g_free(path_and_filename);

    return ret;
}

static datum_type_t get_datum(FILE *fp)
{
  datum_type_t datum = UNKNOWN_DATUM; // Unknown until found
  char *eq, *s;
  char buf[512];
  if (!fp) asfPrintError("Projection parameter file not open.\n");
  FSEEK(fp, 0, SEEK_SET);
  readline(fp, buf, sizeof(buf));
  while (!feof(fp)) {
    if (strlen(buf)) {
      s = buf;
      while(isspace((int)*s))s++;
      if (strncmp(uc(s), "DATUM", 5) == 0) {
        eq = strchr(buf, '=');
        if (eq) {
          s = eq;
          s++;
          while (isspace((int)*s))s++;
          if (strncmp(uc(s), "EGM96", 5) == 0) {
            datum = EGM96_DATUM;
            break;
          }
          else if (strncmp(uc(s), "ED50", 4)   == 0) {
            datum = ED50_DATUM;
            break;
          }
          else if (strncmp(uc(s), "ETRF89", 6) == 0) {
            datum = ETRF89_DATUM;
            break;
          }
          else if (strncmp(uc(s), "ETRS89", 6) == 0) {
            datum = ETRS89_DATUM;
            break;
          }
          else if (strncmp(uc(s), "ITRF97", 6) == 0) {
            datum = ITRF97_DATUM;
            break;
          }
          else if (strncmp(uc(s), "NAD27", 5)  == 0) {
            datum = NAD27_DATUM;
            break;
          }
          else if (strncmp(uc(s), "NAD83", 5)  == 0) {
            datum = NAD83_DATUM;
            break;
          }
          else if (strncmp(uc(s), "WGS72", 5)  == 0) {
            datum = WGS72_DATUM;
            break;
          }
          else if (strncmp(uc(s), "WGS84", 5)  == 0) {
            datum = WGS84_DATUM;
            break;
          }
          else if (strncmp(uc(s), "HUGHES", 6) == 0) {
            datum = HUGHES_DATUM;
            break;
          }
        }
      }
    }
    readline(fp, buf, sizeof(buf));
  }

  return datum;
}

static spheroid_type_t get_spheroid(FILE *fp)
{
  spheroid_type_t spheroid = UNKNOWN_SPHEROID; // Unknown until found
  char *eq, *s;
  char buf[512];
  if (!fp) asfPrintError("Projection parameter file not open.\n");
  FSEEK(fp, 0, SEEK_SET);
  readline(fp, buf, sizeof(buf));

  while (!feof(fp)) {
    if (strlen(buf)) {
      s = buf;
      while(isspace((int)*s))s++;
      if (strncmp(uc(s), "SPHEROID", 5) == 0) {
        eq = strchr(buf, '=');
        if (eq) {
          s = eq;
          s++;
          while (isspace((int)*s))s++;
          if (strncmp(uc(s), "BESSEL", 5) == 0) {
            spheroid = BESSEL_SPHEROID;
            break;
          }
          else if (strncmp(uc(s), "CLARKE1866", 4)   == 0) {
            spheroid = CLARKE1866_SPHEROID;
            break;
          }
          else if (strncmp(uc(s), "CLARKE1880", 6) == 0) {
            spheroid = CLARKE1880_SPHEROID;
            break;
          }
          else if (strncmp(uc(s), "GEM6", 6) == 0) {
            spheroid = GEM6_SPHEROID;
            break;
          }
          else if (strncmp(uc(s), "GEM10C", 6) == 0) {
            spheroid = GEM10C_SPHEROID;
            break;
          }
          else if (strncmp(uc(s), "GRS1980", 5)  == 0) {
            spheroid = GRS1980_SPHEROID;
            break;
          }
          else if (strncmp(uc(s), "INTERNATIONAL1924", 5)  == 0) {
            spheroid = INTERNATIONAL1924_SPHEROID;
            break;
          }
          else if (strncmp(uc(s), "INTERNATIONAL1967", 5)  == 0) {
            spheroid = INTERNATIONAL1967_SPHEROID;
            break;
          }
          else if (strncmp(uc(s), "WGS72", 5)  == 0) {
            spheroid = WGS72_SPHEROID;
            break;
          }
          else if (strncmp(uc(s), "WGS84", 6) == 0) {
            spheroid = WGS84_SPHEROID;
            break;
          }
          else if (strncmp(uc(s), "HUGHES", 6) == 0) {
            spheroid = HUGHES_SPHEROID;
            break;
          }
        }
      }
    }
    readline(fp, buf, sizeof(buf));
  }

  return spheroid;
}
