kde_services_DATA = katepart.desktop

rckatedir = $(kde_datadir)/katepart
rckate_DATA = katepartui.rc katepartreadonlyui.rc

syntaxkatedir = $(kde_datadir)/katepart/syntax
syntaxkate_DATA = language.dtd syntax.template \
    abap.xml abc.xml actionscript.xml ada.xml ahdl.xml ahk.xml alert_indent.xml \
    alert.xml ample.xml ansic89.xml ansys.xml apache.xml asm6502.xml asm-avr.xml \
    asm-dsp56k.xml asm-m68k.xml asn1.xml asp.xml asterisk.xml awk.xml bash.xml \
    bibtex.xml bmethod.xml ccss.xml cgis.xml cg.xml changelog.xml chicken.xml \
    cisco.xml clipper.xml clojure.xml cmake.xml coffee.xml coldfusion.xml \
    commonlisp.xml component-pascal.xml context.xml cpp11.xml cpp.xml crk.xml \
    css.xml css-php.xml cs.xml cue.xml curry.xml c.xml ddoc.xml debianchangelog.xml \
    debiancontrol.xml desktop.xml diff.xml djangotemplate.xml dosbat.xml dot.xml \
    doxygenlua.xml doxygen.xml dtd.xml d.xml eiffel.xml email.xml erlang.xml \
    euphoria.xml e.xml ferite.xml fgl-4gl.xml fgl-per.xml fortran.xml freebasic.xml \
    fsharp.xml fstab.xml gap.xml gcc.xml gdb.xml gdl.xml gettext.xml git-rebase.xml \
    glosstex.xml glsl.xml gnuassembler.xml gnuplot.xml go.xml grammar.xml haml.xml \
    haskell.xml html.xml html-php.xml idconsole.xml idl.xml ilerpg.xml inform.xml \
    ini.xml jam.xml javadoc.xml javascript.xml javascript-php.xml java.xml json.xml \
    jsp.xml julia.xml kbasic.xml latex.xml ldif.xml less.xml lex.xml lilypond.xml \
    literate-curry.xml literate-haskell.xml logtalk.xml lpc.xml lua.xml m3u.xml \
    m4.xml mab.xml makefile.xml mako.xml mason.xml matlab.xml maxima.xml \
    mediawiki.xml mel.xml mergetagtext.xml mips.xml modelica.xml modelines.xml \
    modula-2.xml monobasic.xml mup.xml nasm.xml nemerle.xml nesc.xml noweb.xml \
    objectivecpp.xml objectivec.xml ocaml.xml octave.xml oors.xml opal.xml opencl.xml \
    pango.xml pascal.xml perl.xml pgn.xml php.xml picsrc.xml pig.xml pike.xml \
    postscript.xml povray.xml ppd.xml progress.xml prolog.xml protobuf.xml \
    purebasic.xml python.xml qmake.xml qml.xml qt4.xml rapidq.xml relaxngcompact.xml \
    relaxng.xml restructuredtext.xml rest.xml rexx.xml rhtml.xml rib.xml rpmspec.xml \
    rsiidl.xml ruby.xml r.xml sather.xml scala.xml scheme.xml sci.xml scss.xml \
    sed.xml sgml.xml sieve.xml sisu.xml sml.xml spice.xml sql-mysql.xml \
    sql-postgresql.xml sql.xml stata.xml systemc.xml systemverilog.xml tads3.xml \
    tcl.xml tcsh.xml template-toolkit.xml texinfo.xml textile.xml tibasic.xml \
    txt2tags.xml update-files.xml uscript.xml valgrind-suppression.xml \
    varnishtest.xml varnish.xml vcard.xml velocity.xml vera.xml verilog.xml vhdl.xml \
    vrml.xml winehq.xml wml.xml xharbour.xml xmldebug.xml xml.xml xorg.xml xslt.xml \
    xul.xml yacas.xml yacc.xml yaml.xml zonnon.xml zsh.xml

partrcdir   = $(kde_confdir)
partrc_DATA = katesyntaxhighlightingrc katefiletyperc

html-php.xml: $(srcdir)/html.xml $(srcdir)/generate-php.pl
	$(PERL) $(srcdir)/generate-php.pl < $(srcdir)/html.xml > $@

css-php.xml: $(srcdir)/css.xml $(srcdir)/generate-php.pl
	$(PERL) $(srcdir)/generate-php.pl < $(srcdir)/css.xml > $@

javascript-php.xml: $(srcdir)/javascript.xml $(srcdir)/generate-php.pl
	$(PERL) $(srcdir)/generate-php.pl < $(srcdir)/javascript.xml > $@
