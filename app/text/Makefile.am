## Process this file with automake to produce Makefile.in

AM_CPPFLAGS = \
	-DG_LOG_DOMAIN=\"Gimp-Text\"	\
	-I$(top_builddir)		\
	-I$(top_srcdir)			\
	-I$(top_builddir)/app		\
	-I$(top_srcdir)/app		\
	$(GEGL_CFLAGS)			\
	$(PANGOCAIRO_CFLAGS)		\
	$(HARFBUZZ_CFLAGS)		\
	$(GDK_PIXBUF_CFLAGS)		\
	-I$(includedir)

noinst_LIBRARIES = libapptext.a

libapptext_a_sources = \
	text-types.h			\
	text-enums.h			\
	gimp-fonts.c			\
	gimp-fonts.h			\
	gimpfont.c			\
	gimpfont.h			\
	gimpfontlist.c			\
	gimpfontlist.h			\
	gimptext.c			\
	gimptext.h			\
	gimptext-compat.c		\
	gimptext-compat.h		\
	gimptext-parasite.c		\
	gimptext-parasite.h		\
	gimptext-vectors.c		\
	gimptext-vectors.h		\
	gimptext-xlfd.c			\
	gimptext-xlfd.h			\
	gimptextlayer.c			\
	gimptextlayer.h			\
	gimptextlayer-transform.c	\
	gimptextlayer-transform.h	\
	gimptextlayer-xcf.c		\
	gimptextlayer-xcf.h		\
	gimptextlayout.c		\
	gimptextlayout.h		\
	gimptextlayout-render.c		\
	gimptextlayout-render.h		\
	gimptextundo.c			\
	gimptextundo.h

libapptext_a_built_sources = text-enums.c

libapptext_a_SOURCES = $(libapptext_a_built_sources) $(libapptext_a_sources)

#
# rules to generate built sources
#
# setup autogeneration dependencies
gen_sources = xgen-tec
CLEANFILES = $(gen_sources)

xgen-tec: $(srcdir)/text-enums.h $(GIMP_MKENUMS) Makefile.am
	$(AM_V_GEN) $(GIMP_MKENUMS) \
		--fhead "#include \"config.h\"\n#include <gio/gio.h>\n#include \"libgimpbase/gimpbase.h\"\n#include \"text-enums.h\"\n#include \"gimp-intl.h\"" \
		--fprod "\n/* enumerations from \"@basename@\" */" \
		--vhead "GType\n@enum_name@_get_type (void)\n{\n  static const G@Type@Value values[] =\n  {" \
		--vprod "    { @VALUENAME@, \"@VALUENAME@\", \"@valuenick@\" }," \
		--vtail "    { 0, NULL, NULL }\n  };\n" \
		--dhead "  static const Gimp@Type@Desc descs[] =\n  {" \
		--dprod "    { @VALUENAME@, @valuedesc@, @valuehelp@ }," \
		--dtail "    { 0, NULL, NULL }\n  };\n\n  static GType type = 0;\n\n  if (G_UNLIKELY (! type))\n    {\n      type = g_@type@_register_static (\"@EnumName@\", values);\n      gimp_type_set_translation_context (type, \"@enumnick@\");\n      gimp_@type@_set_value_descriptions (type, descs);\n    }\n\n  return type;\n}\n" \
		$< > $@

$(srcdir)/text-enums.c: xgen-tec
	$(AM_V_GEN) if ! cmp -s $< $@; then \
		cp $< $@; \
	else \
		# if the file hasn't changed, only update its timestamp, so that \
		# the receipe doesn't get executed again next time, but allow this \
		# to fail (e.g., when using a read-only source directory). \
		touch $@ 2> /dev/null \
		|| true; \
	fi
