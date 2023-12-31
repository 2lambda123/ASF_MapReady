CFLAGS += $(HDF5_CFLAGS)
CFLAGS += $(GEOTIFF_CFLAGS)
# Makefile for libasf_import library

include ../../make_support/system_rules

LIBS  = \
	$(LIBDIR)/libasf_proj.a \
	$(LIBDIR)/libasf_raster.a \
	$(LIBDIR)/asf_meta.a \
	$(LIBDIR)/libasf_sar.a \
	$(LIBDIR)/asf.a \
	$(GEOTIFF_LIBS) \
	$(TIFF_LIBS) \
	$(JPEG_LIBS) \
	$(XML_LIBS) \
	$(PROJ_LIBS) \
	$(GLIB_LIBS) \
	$(NETCDF_LIBS) \
	$(HDF5_LIBS) \
	$(GSL_LIBS) \
	-lz

CFLAGS += $(W_ERROR) \
	$(GEOTIFF_CFLAGS) \
	$(TIFF_CFLAGS) \
	$(JPEG_CFLAGS) \
	$(XML_CFLAGS) \
	$(PROJ_CFLAGS) \
	$(GLIB_CFLAGS) \
	-D_REENTRANT \
	$(GSL_CFLAGS) \
	$(HDF5_CFLAGS) \
	$(GDAL_CFLAGS)

# These defines enable various debugging behavior and output.  Leaving
# debugging code in and available is all the rage these days, and is
# probably even a good idea.  The following stuff constitute a sort of
# catalog of the available debugging features for this program, which
# acronymish things you might choose to enable from the command line
# that invokes make (e.g. make DISSJO=nonemptystring) so you don't
# forget to remove them before release.
#
DISSJO =
ifdef DIUSJO
  DEBUG_FEATURES += -DDEBUG_IMPORT_USGS_SEAMLESS_JPEG_OUTPUT
endif
DIAUG =
ifdef DIAUG
  DEBUG_FEATURES += -DDEBUG_IMPORT_ASF_UTM_GEOTIFF_JPEG_OUTPUT
endif
CFLAGS += $(DEBUG_FEATURES)

OBJS  = \
	ardop_write.o \
	asf_import.o \
	aux_ERS.o \
	aux_JRS.o \
	aux_JRS_PCM.o \
	aux_RSAT.o \
	bin_state.o \
	createSubset.o \
	decoder_ERS.o \
	decoder_JRS.o \
	decoder_RSAT.o \
	decoder_utils.o \
	fetch_prc_stvec.o \
	find_arcgis_geotiff_aux_name.o \
	find_geotiff_name.o \
	frame.o \
	arcgis_geotiff_support.o \
	import_generic_geotiff.o \
	import_ceos.o \
	import_stf.o \
	import_bil.o \
	import_gridfloat.o \
	import_airsar.o \
	import_uavsar.o \
	import_gamma.o \
	import_vexcel_plain.o \
	import_jaxa_L0.o \
	import_alos_mosaic.o \
	import_radarsat2.o \
	import_terrasar.o \
	import_polsarpro.o \
	import_fgdc_meta.o \
	import_roipac.o \
	import_smap.o \
	import_seasat_h5.o \
	import_netcdf.o \
	import_sentinel.o \
	config_fgdc.o \
	missing.o \
	projected_image_import.o \
        tiff_to_byte_image.o \
        tiff_to_float_image.o \
	unpack.o \
	utilities_ceos.o \
	utilities_stf.o \
	lut.o \
	write_meta_and_img.o

all: build_only
	mv libasf_import.a $(LIBDIR)
	cp asf_import.h fgdc_meta.h dem_meta.h prc_stvecs.h \
	$(ASF_INCLUDE_DIR)
	rm -rf $(SHAREDIR)/sentinel
	cp -R sentinel $(SHAREDIR)

build_only: $(OBJS)
	rm -f libasf_import.a
	ar r libasf_import.a $(OBJS)
	$(RANLIB) libasf_import.a

$(OBJS): Makefile $(wildcard *.h) $(wildcard ../../include/*h)

clean:
	rm -f core $(OBJS) *.o
