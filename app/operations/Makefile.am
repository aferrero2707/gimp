## Process this file with automake to produce Makefile.in

SUBDIRS = \
	layer-modes		\
	tests

AM_CPPFLAGS = \
	-DG_LOG_DOMAIN=\"Gimp-Operations\"	\
	-I$(top_builddir)			\
	-I$(top_srcdir)				\
	-I$(top_builddir)/app			\
	-I$(top_srcdir)/app			\
	$(CAIRO_CFLAGS)				\
	$(GEGL_CFLAGS)				\
	$(GDK_PIXBUF_CFLAGS)			\
	-I$(includedir)

noinst_LIBRARIES = \
	libappoperations.a

libappoperations_a_sources = \
	operations-types.h			\
	operations-enums.h			\
	gimp-operations.c			\
	gimp-operations.h			\
	\
	gimp-operation-config.c			\
	gimp-operation-config.h			\
	gimpcageconfig.c			\
	gimpcageconfig.h			\
	gimpcurvesconfig.c			\
	gimpcurvesconfig.h			\
	gimplevelsconfig.c			\
	gimplevelsconfig.h			\
	\
	gimpoperationblend.c			\
	gimpoperationblend.h			\
	gimpoperationborder.c			\
	gimpoperationborder.h			\
	gimpoperationcagecoefcalc.c		\
	gimpoperationcagecoefcalc.h		\
	gimpoperationcagetransform.c		\
	gimpoperationcagetransform.h		\
	gimpoperationcomposecrop.c		\
	gimpoperationcomposecrop.h		\
	gimpoperationequalize.c			\
	gimpoperationequalize.h			\
	gimpoperationflood.c			\
	gimpoperationflood.h			\
	gimpoperationgrow.c			\
	gimpoperationgrow.h			\
	gimpoperationhistogramsink.c		\
	gimpoperationhistogramsink.h		\
	gimpoperationmaskcomponents.c		\
	gimpoperationmaskcomponents.h		\
	gimpoperationprofiletransform.c		\
	gimpoperationprofiletransform.h		\
	gimpoperationscalarmultiply.c		\
	gimpoperationscalarmultiply.h		\
	gimpoperationsemiflatten.c		\
	gimpoperationsemiflatten.h		\
	gimpoperationsetalpha.c			\
	gimpoperationsetalpha.h			\
	gimpoperationshapeburst.c		\
	gimpoperationshapeburst.h		\
	gimpoperationshrink.c			\
	gimpoperationshrink.h			\
	gimpoperationthresholdalpha.c		\
	gimpoperationthresholdalpha.h		\
	\
	gimpoperationpointfilter.c		\
	gimpoperationpointfilter.h		\
	gimpoperationcurves.c			\
	gimpoperationcurves.h			\
	gimpoperationdesaturate.c		\
	gimpoperationdesaturate.h		\
	gimpoperationlevels.c			\
	gimpoperationlevels.h			\
	gimpoperationposterize.c		\
	gimpoperationposterize.h		\
	gimpoperationthreshold.c		\
	gimpoperationthreshold.h

libappoperations_a_built_sources = operations-enums.c

libappoperations_a_SOURCES = \
	$(libappoperations_a_built_sources)	\
	$(libappoperations_a_sources)

#
# rules to generate built sources
#
# setup autogeneration dependencies
gen_sources = xgen-oec
CLEANFILES = $(gen_sources)

xgen-oec: $(srcdir)/operations-enums.h $(GIMP_MKENUMS) Makefile.am
	$(AM_V_GEN) $(GIMP_MKENUMS) \
		--fhead "#include \"config.h\"\n#include <gio/gio.h>\n#include \"libgimpbase/gimpbase.h\"\n#include \"operations-enums.h\"\n#include \"gimp-intl.h\"" \
		--fprod "\n/* enumerations from \"@basename@\" */" \
		--vhead "GType\n@enum_name@_get_type (void)\n{\n  static const G@Type@Value values[] =\n  {" \
		--vprod "    { @VALUENAME@, \"@VALUENAME@\", \"@valuenick@\" }," \
		--vtail "    { 0, NULL, NULL }\n  };\n" \
		--dhead "  static const Gimp@Type@Desc descs[] =\n  {" \
		--dprod "    { @VALUENAME@, @valuedesc@, @valuehelp@ }," \
		--dtail "    { 0, NULL, NULL }\n  };\n\n  static GType type = 0;\n\n  if (G_UNLIKELY (! type))\n    {\n      type = g_@type@_register_static (\"@EnumName@\", values);\n      gimp_type_set_translation_context (type, \"@enumnick@\");\n      gimp_@type@_set_value_descriptions (type, descs);\n    }\n\n  return type;\n}\n" \
		$< > $@

$(srcdir)/operations-enums.c: xgen-oec
	$(AM_V_GEN) if ! cmp -s $< $@; then \
		cp $< $@; \
	else \
		# if the file hasn't changed, only update its timestamp, so that \
		# the receipe doesn't get executed again next time, but allow this \
		# to fail (e.g., when using a read-only source directory). \
		touch $@ 2> /dev/null \
		|| true; \
	fi
