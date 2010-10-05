#################################################
#
#  (C) 2010 Serghei Amelian
#  serghei (DOT) amelian (AT) gmail.com
#
#  Improvements and feedback are welcome
#
#  This file is released under GPL >= 2
#
#################################################

add_definitions(
  ${TQT_CFLAGS_OTHER}
  -include tqt.h
)

include_directories(
  ${TQT_INCLUDE_DIRS}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/kdecore
  ${CMAKE_SOURCE_DIR}/kdeui
  ${CMAKE_SOURCE_DIR}/kdefx
  ${CMAKE_SOURCE_DIR}/dcop
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)

set( test_PROGS
  kaboutdialogtest kblendtest kbuttonboxtest kcharselecttest
  kcolortest kcolordlgtest kcomboboxtest kcompletiontest
  kdatepicktest kdatewidgettest kdialogbasetest
  kdockwidgetdemo kdockwidgettest
  kdualcolortest keditlistboxtest kfontdialogtest
  kgradienttest khashtest kdesattest
  kledtest klineedittest klineeditdlgtest
  kmessageboxtest knuminputtest
  kpalettetest
  kpopuptest kprogresstest krulertest kseparatortest
  kstatusbartest ktabctltest
  kunbalancedgrdtest kwindowtest kwizardtest
  itemcontainertest kpanelmenutest kedittest
  kxmlguitest kaccelgentest ksystemtraytest klistviewtest
  ktextedittest kmainwindowtest kmainwindowrestoretest
  kactiontest kdatetimewidgettest ktimewidgettest
  kspelltest kinputdialogtest ktabwidgettest kwordwraptest
  kdocktest qxembedtest ksqueezedtextlabeltest
  kpixmapregionselectordialogtest kjanuswidgettest ktoolbarlabelactiontest
)

foreach( PROG ${test_PROGS} )
  tde_add_executable( ${PROG} AUTOMOC SOURCES "${PROG}.cpp" LINK kdeui-shared )
endforeach( PROG ${test_PROGS} )
