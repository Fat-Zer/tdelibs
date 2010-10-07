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

cmake_minimum_required( VERSION 2.6 )

# FIXME KDE3_MODULE_DIR is not set; this is cmake bug?
set( KDE3_MODULE_DIR "/usr/share/cmake/Modules" )

# directory to our cmake modules
set( CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules" )

project( kdelibs )
set( VERSION "3.5.12" )

##### user requested options ####################

OPTION( KDE_MALLOC "Use own malloc implementation" OFF )
OPTION( KDE_MALLOC_DEBUG "Enable debugging in fast malloc" OFF )
OPTION( KDE_MALLOC_FULL "Make alloc as fast as possible" OFF )

OPTION( WITH_ARTS "Build with aRts" ON )
OPTION( WITH_ALSA "Enable ALSA support" ON )
OPTION( WITH_LIBART "Enable libart support (for SVG icons)" OFF )
OPTION( WITH_LIBIDN "Enable support for libidn" OFF )
OPTION( WITH_SSL "Enable support for SSL" ON )
OPTION( WITH_CUPS "Enable CUPS support" ON )
OPTION( WITH_LUA "Enable LUA support" OFF )
OPTION( WITH_TIFF "Enable tiff support" OFF )
OPTION( WITH_JASPER "Enable jasper (jpeg2k) support" OFF )
OPTION( WITH_OPENEXR "Enable openexr support" OFF )

OPTION( WITH_ASPELL "Enable aspell support" OFF )
OPTION( WITH_HSPELL "Enable hspell support" OFF )


##### paths setup ###############################

if( NOT BIN_INSTALL_DIR )
  set( BIN_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/bin )
endif ( NOT BIN_INSTALL_DIR )

if( NOT LIB_INSTALL_DIR )
  set( LIB_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/lib )
endif( NOT LIB_INSTALL_DIR )

if( NOT INCLUDE_INSTALL_DIR )
  set( INCLUDE_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/include )
endif( NOT INCLUDE_INSTALL_DIR )

if( NOT SHARE_INSTALL_DIR )
  set( SHARE_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/share )
endif( NOT SHARE_INSTALL_DIR )

if( NOT ICONS_INSTALL_DIR )
  set( ICONS_INSTALL_DIR ${SHARE_INSTALL_DIR}/icons )
endif( NOT ICONS_INSTALL_DIR )

if( NOT MIME_INSTALL_DIR )
  set( MIME_INSTALL_DIR ${SHARE_INSTALL_DIR}/mimelnk )
endif( NOT MIME_INSTALL_DIR )

if( NOT PLUGIN_INSTALL_DIR )
  set( PLUGIN_INSTALL_DIR ${LIB_INSTALL_DIR}/kde3 )
endif( NOT PLUGIN_INSTALL_DIR )

if( NOT SERVICES_INSTALL_DIR )
  set( SERVICES_INSTALL_DIR ${SHARE_INSTALL_DIR}/services )
endif( NOT SERVICES_INSTALL_DIR )

if( NOT DATA_INSTALL_DIR )
  set( DATA_INSTALL_DIR ${SHARE_INSTALL_DIR}/apps )
endif( NOT DATA_INSTALL_DIR )

if( NOT HTML_INSTALL_DIR )
  set( HTML_INSTALL_DIR ${SHARE_INSTALL_DIR}/doc/HTML )
endif( NOT HTML_INSTALL_DIR )

if( NOT CONFIG_INSTALL_DIR )
  set( CONFIG_INSTALL_DIR ${SHARE_INSTALL_DIR}/config )
endif( NOT CONFIG_INSTALL_DIR )

if( NOT LOCALE_INSTALL_DIR )
  set( LOCALE_INSTALL_DIR ${SHARE_INSTALL_DIR}/locale )
endif( NOT LOCALE_INSTALL_DIR )

if( NOT PKGCONFIG_INSTALL_DIR )
  set( PKGCONFIG_INSTALL_DIR ${LIB_INSTALL_DIR}/pkgconfig )
endif( NOT PKGCONFIG_INSTALL_DIR )

if( NOT DCOP_PATH )
  set( DCOP_PATH ${BIN_INSTALL_DIR} )
endif( NOT DCOP_PATH )

if( NOT KDEDIR )
  set( KDEDIR ${CMAKE_INSTALL_PREFIX} )
endif( NOT KDEDIR )


#################################################


if( NOT SYSCONFDIR )
  set( SYSCONFDIR ${CMAKE_INSTALL_PREFIX}/etc )
endif( NOT SYSCONFDIR )

if( NOT XDG_MENUDIR )
  set( XDG_MENUDIR ${SYSCONFDIR}/xdg/menus )
endif( NOT XDG_MENUDIR )


##### set PKG_CONFIG_PATH #######################

set( ENV{PKG_CONFIG_PATH} "${PKG_CONFIG_PATH}:${PKGCONFIG_INSTALL_DIR}:${LIB_INSTALL_DIR}/pkgconfig:$ENV{PKG_CONFIG_PATH}" )


##### include our cmake modules #################

include( TDEMacros )


##### check for CPU architecture ################

### FIXME fast malloc is also available on x86_64 architecture?
if( ${CMAKE_SYSTEM_PROCESSOR} MATCHES "i?86" OR ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64" )
  set( KDE_MALLOC_X86 1 )
  message( STATUS "Found x86 architecture" )
else( ${CMAKE_SYSTEM_PROCESSOR} MATCHES "i?86" OR ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64" )
  if( KDE_MALLOC )
    message( FATAL_ERROR "\nKDE fast malloc is available only on x86 architecture" )
  endif( KDE_MALLOC )
endif( ${CMAKE_SYSTEM_PROCESSOR} MATCHES "i?86" OR ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64" )


##### check for operating system ################

if( ${CMAKE_SYSTEM_NAME} STREQUAL "Linux" )
  set( KDEINIT_SETUID 1 )
  set( KDEINIT_OOM_PROTECT 1 )
endif( ${CMAKE_SYSTEM_NAME} STREQUAL "Linux" )


##### check for compiler capabilities ###########

include( CheckCXXCompilerFlag )

check_cxx_compiler_flag( -Woverloaded-virtual WOVERLOADED_VIRTUAL )
if( WOVERLOADED_VIRTUAL )
  set( WOVERLOADED_VIRTUAL -Woverloaded-virtual )
endif( WOVERLOADED_VIRTUAL )

check_cxx_compiler_flag( -fno-builtin NO_BUILTIN )
if( NO_BUILTIN )

endif( NO_BUILTIN )


##### system checks #############################

include( CheckIncludeFile )
include( CheckSymbolExists )
include( CheckFunctionExists )
include( FindPkgConfig )
include( CheckCSourceCompiles )
include( CheckTypeSize )
include( CheckCSourceRuns )

check_include_file( "alloca.h" HAVE_ALLOCA_H )
check_include_file( "arpa/nameser8_compat.h" HAVE_ARPA_NAMESER8_COMPAT_H )
check_include_file( "awe_voice.h" HAVE_AWE_VOICE_H )
check_include_file( "Carbon/Carbon.h" HAVE_CARBON_CARBON_H )
check_include_file( "crt_externs.h" HAVE_CRT_EXTERNS_H )
check_include_file( "ctype.h" HAVE_CTYPE_H )
check_include_file( "dirent.h" HAVE_DIRENT_H )
check_include_file( "dld.h" HAVE_DLD_H )
check_include_file( "dlfcn.h" HAVE_DLFCN_H )
check_include_file( "dl.h" HAVE_DL_H )
check_include_file( "float.h" HAVE_FLOAT_H )
check_include_file( "fstab.h" HAVE_FSTAB_H )
check_include_file( "ieeefp.h" HAVE_IEEEFP_H )
check_include_file( "inttypes.h" HAVE_INTTYPES_H )
check_include_file( "libutil.h" HAVE_LIBUTIL_H )
check_include_file( "limits.h" HAVE_LIMITS_H )
check_include_file( "linux/awe_voice.h" HAVE_LINUX_AWE_VOICE_H )
check_include_file( "locale.h" HAVE_LOCALE_H )
check_include_file( "machine/soundcard.h" HAVE_MACHINE_SOUNDCARD_H )
check_include_file( "malloc.h" HAVE_MALLOC_H )
check_include_file( "memory.h" HAVE_MEMORY_H )
check_include_file( "mntent.h" HAVE_MNTENT_H )
check_include_file( "ndir.h" HAVE_NDIR_H )
check_include_file( "netinet/in.h" HAVE_NETINET_IN_H )
check_include_file( "net/if.h" HAVE_NET_IF_H )
check_include_file( "paths.h" HAVE_PATHS_H )
check_include_file( "pty.h" HAVE_PTY_H )
check_include_file( "stdint.h" HAVE_STDINT_H )
check_include_file( "stdio.h" HAVE_STDIO_H )
check_include_file( "stdlib.h" HAVE_STDLIB_H )
check_include_file( "strings.h" HAVE_STRINGS_H )
check_include_file( "string.h" HAVE_STRING_H )
check_include_file( "sysent.h" HAVE_SYSENT_H )
check_include_file( "sys/bitypes.h" HAVE_SYS_BITYPES_H )
check_include_file( "sys/dir.h" HAVE_SYS_DIR_H )
check_include_file( "sys/filio.h" HAVE_SYS_FILIO_H )
check_include_file( "sys/mman.h" HAVE_SYS_MMAN_H )
check_include_file( "sys/mntent.h" HAVE_SYS_MNTENT_H )
check_include_file( "sys/mnttab.h" HAVE_SYS_MNTTAB_H )
check_include_file( "sys/mount.h" HAVE_SYS_MOUNT_H )
check_include_file( "sys/ndir.h" HAVE_SYS_NDIR_H )
check_include_file( "sys/param.h" HAVE_SYS_PARAM_H )
check_include_file( "sys/prctl.h" HAVE_SYS_PRCTL_H )
check_include_file( "sys/select.h" HAVE_SYS_SELECT_H )
check_include_file( "sys/soundcard.h" HAVE_SYS_SOUNDCARD_H )
check_include_file( "sys/stat.h" HAVE_SYS_STAT_H )
check_include_file( "sys/stropts.h" HAVE_SYS_STROPTS_H )
check_include_file( "sys/types.h" HAVE_SYS_TYPES_H )
check_include_file( "sys/ucred.h" HAVE_SYS_UCRED_H )
check_include_file( "sys/xattr.h" sys/xattr.h )
check_include_file( "termios.h" HAVE_TERMIOS_H )
check_include_file( "termio.h" HAVE_TERMIO_H )
check_include_file( "unistd.h" HAVE_UNISTD_H )
check_include_file( "util.h" HAVE_UTIL_H )

# FIXME I'm not sure if test TIME_WITH_SYS_TIME are correct
check_include_file( "sys/time.h" HAVE_SYS_TIME_H )
check_include_file( "time.h" HAVE_TIME_H )
if( HAVE_SYS_TIME_H AND HAVE_TIME_H )
  set( TIME_WITH_SYS_TIME 1 )
endif( HAVE_SYS_TIME_H AND HAVE_TIME_H )

check_include_file( "valgrind/memcheck.h" HAVE_VALGRIND_MEMCHECK_H )
check_include_file( "values.h" HAVE_VALUES_H )
check_include_file( "X11/extensions/shape.h" HAVE_X11_EXTENSIONS_SHAPE_H )
check_include_file( "X11/extensions/Xrandr.h" HAVE_X11_EXTENSIONS_XRANDR_H )
check_include_file( "X11/extensions/XShm.h" HAVE_X11_EXTENSIONS_XSHM_H )
check_include_file( "X11/ICE/ICElib.h" HAVE_X11_ICE_ICELIB_H )
check_include_file( "/usr/src/sys/gnu/i386/isa/sound/awe_voice.h" HAVE__USR_SRC_SYS_GNU_I386_ISA_SOUND_AWE_VOICE_H )
check_include_file( "/usr/src/sys/i386/isa/sound/awe_voice.h" HAVE__USR_SRC_SYS_I386_ISA_SOUND_AWE_VOICE_H )

check_function_exists( random HAVE_RANDOM )
check_symbol_exists( random "stdlib.h" HAVE_RANDOM_PROTO )

check_function_exists( srandom HAVE_SRANDOM )
check_symbol_exists( srandom "stdlib.h" HAVE_SRANDOM_PROTO )

check_function_exists( seteuid HAVE_SETEUID )

check_function_exists( gethostname HAVE_GETHOSTNAME )
check_symbol_exists( gethostname "unistd.h" HAVE_GETHOSTNAME_PROTO )

check_function_exists( vsnprintf HAVE_VSNPRINTF )
check_function_exists( snprintf HAVE_SNPRINTF )

check_function_exists( setenv HAVE_SETENV )
check_symbol_exists( setenv "stdlib.h" HAVE_SETENV_PROTO )

check_function_exists( unsetenv HAVE_UNSETENV )
check_symbol_exists( unsetenv "stdlib.h" HAVE_UNSETENV_PROTO )

check_function_exists( mkdtemp HAVE_MKDTEMP )
check_symbol_exists( mkdtemp "stdlib.h" HAVE_MKDTEMP_PROTO )

check_function_exists( mkstemp HAVE_MKSTEMP )
check_symbol_exists( mkstemp "stdlib.h" HAVE_MKSTEMP_PROTO )

check_function_exists( mkstemps HAVE_MKSTEMPS )
check_symbol_exists( mkstemps "stdlib.h" HAVE_MKSTEMPS_PROTO )

check_function_exists( initgroups HAVE_INITGROUPS )
check_symbol_exists( initgroups "grp.h" HAVE_INITGROUPS_PROTO )

check_symbol_exists( S_ISSOCK "sys/stat.h" HAVE_S_ISSOCK )

check_symbol_exists( gethostbyname2 "netdb.h" HAVE_GETHOSTBYNAME2 )
check_symbol_exists( gethostbyname2_r "netdb.h" HAVE_GETHOSTBYNAME2_R )
check_symbol_exists( gethostbyname_r "netdb.h" HAVE_GETHOSTBYNAME_R )
check_symbol_exists( gai_strerror "sys/types.h;sys/socket.h;netdb.h" HAVE_GAI_STRERROR )
check_symbol_exists( getaddrinfo "sys/types.h;sys/socket.h;netdb.h" HAVE_GETADDRINFO )

check_function_exists( usleep HAVE_USLEEP )
check_symbol_exists( usleep "unistd.h" HAVE_USLEEP_PROTO )

check_function_exists( getmntinfo HAVE_GETMNTINFO )
check_function_exists( getnameinfo HAVE_GETNAMEINFO )
check_function_exists( getpagesize HAVE_GETPAGESIZE )
check_function_exists( getpeereid HAVE_GETPEEREID )
check_function_exists( getpeername HAVE_GETPEERNAME )
check_function_exists( getprotobyname_r HAVE_GETPROTOBYNAME_R )
check_function_exists( getpt HAVE_GETPT )
check_function_exists( getservbyname_r HAVE_GETSERVBYNAME_R )
check_function_exists( getservbyport_r HAVE_GETSERVBYPORT_R )
check_function_exists( getsockname HAVE_GETSOCKNAME )
check_function_exists( getsockopt HAVE_GETSOCKOPT )
check_function_exists( gettimeofday HAVE_GETTIMEOFDAY )
check_function_exists( grantpt HAVE_GRANTPT )
check_function_exists( if_nametoindex HAVE_IF_NAMETOINDEX )
check_function_exists( index HAVE_INDEX)
check_function_exists( inet_ntop HAVE_INET_NTOP )
check_function_exists( inet_pton HAVE_INET_PTON )
check_function_exists( setfsent HAVE_SETFSENT )
check_function_exists( setgroups HAVE_SETGROUPS )
check_function_exists( setlocale HAVE_SETLOCALE )
check_function_exists( setmntent HAVE_SETMNTENT )
check_function_exists( setpriority HAVE_SETPRIORITY )
check_function_exists( isnan HAVE_FUNC_ISNAN )
check_function_exists( _finite HAVE_FUNC__FINITE )
check_function_exists( finite HAVE_FUNC_FINITE )
check_function_exists( isinf HAVE_FUNC_ISINF )
check_function_exists( freeaddrinfo HAVE_FREEADDRINFO )

check_type_size( "char*" SIZEOF_CHAR_P )
check_type_size( "int" SIZEOF_INT )
check_type_size( "long" SIZEOF_LONG )
check_type_size( "short" SIZEOF_SHORT )
check_type_size( "size_t" SIZEOF_SIZE_T )
check_type_size( "unsigned long" SIZEOF_UNSIGNED_LONG )

set( CMAKE_EXTRA_INCLUDE_FILES "netdb.h" )
check_type_size( "struct addrinfo" STRUCT_ADDRINFO )
set( CMAKE_EXTRA_INCLUDE_FILES "netinet/in.h" )
check_type_size( "struct sockaddr_in6" HAVE_STRUCT_SOCKADDR_IN6 )
unset( CMAKE_EXTRA_INCLUDE_FILES )

check_c_source_compiles("
  #include <features.h>
  #ifndef __GNU_LIBRARY__
  #error
  #endif
  int main(int argc, char *argv[]) { return 0; } "
  _GNU_SOURCE )

check_c_source_compiles("
  #include <unistd.h>
  int main(int argc, char *argv[]) { revoke(\"/dev/tty\"); return 0; } "
  HAVE_REVOKE )

check_c_source_runs("
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netdb.h>
  #include <string.h>
  int main()
  {
    struct addrinfo hint, *res;
    int err;
    memset(&hint, 0, sizeof(hint));
    hint.ai_family = AF_INET;
    hint.ai_protocol = 0;
    hint.ai_socktype = SOCK_STREAM;
    hint.ai_flags = AI_PASSIVE;
    err = getaddrinfo(0, \"18300\", &hint, &res); /* kxmlrpc tries this */
    if (err != 0 || res == 0 || res->ai_family != AF_INET)
      return 1;
    return 0;
  }"
  HAVE_GOOD_GETADDRINFO
)

if( NOT HAVE_GOOD_GETADDRINFO )
  set( HAVE_BROKEN_GETADDRINFO 1 )
endif( NOT HAVE_GOOD_GETADDRINFO )


# save CMAKE_REQUIRED_LIBRARIES
set( bak_CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES} )

set( CMAKE_REQUIRED_LIBRARIES resolv )
check_c_source_compiles("
  #include <sys/types.h>
  #include <netinet/in.h>
  #include <arpa/nameser.h>
  #include <resolv.h>
  int main(int argc, char *argv[]) { res_init(); return 0; } "
  HAVE_RES_INIT )

if( HAVE_RES_INIT )
  set( RESOLV_LIBRARIES resolv )
endif( HAVE_RES_INIT)

# restore CMAKE_REQUIRED_LIBRARIES
set( CMAKE_REQUIRED_LIBRARIES ${bak_CMAKE_REQUIRED_LIBRARIES} )
unset( bak_CMAKE_REQUIRED_LIBRARIES )


# FIXME for unknown reason cmake cannot find su
#find_program( __PATH_SU su )
set( __PATH_SU "/bin/su" )

if( __PATH_SU )
  set( DEFAULT_SUPER_USER_COMMAND ${__PATH_SU} )
else( __PATH_SU )
  message( STATUS "WARNING: su was not found" )
endif( __PATH_SU )

# FIXME for unknown reason cmake cannot find sudo
# find_program( __PATH_SUDO "sudo" )
set( __PATH_SUDO "/usr/bin/sudo" )
if( WITH_SUDO_KDESU_BACKEND )
  if( __PATH_SUDO )
    set( DEFAULT_SUPER_USER_COMMAND ${__PATH_SUDO} )
  else( __PATH_SUDO )
    message( FATAL_ERROR "sudo was chosen as kdesu backend, but was not found in path." )
  endif( __PATH_SUDO )
endif( WITH_SUDO_KDESU_BACKEND )


#### set some constants #########################

set( LTDL_OBJDIR \".\" )
set( KDELIBSUFF "" )
set( kde_socklen_t socklen_t )


##### check for libdl ###########################

find_library( HAVE_LIBDL dl )
if( NOT HAVE_LIBDL-NOTFOUND )
  set( DL_LIBRARIES dl )
endif( NOT HAVE_LIBDL-NOTFOUND )


##### check for zlib ############################

find_package( ZLIB )
if( NOT ZLIB_FOUND )
    message(FATAL_ERROR "\nzlib are required, but not found on your system" )
endif( NOT ZLIB_FOUND )

set( HAVE_LIBZ 1 )


#### check for bzip2 ############################

find_package( BZip2 )
if( NOT BZIP2_FOUND )
  # FIXME I'm not sure if bzip2 are required; maybe is optional?
  message(FATAL_ERROR "\nbzip2 are required, but not found on your system" )
endif( NOT BZIP2_FOUND )

set( HAVE_BZIP2_SUPPORT 1 )

if( BZIP2_NEED_PREFIX )
  set( NEED_BZ2_PREFIX 1 )
endif( BZIP2_NEED_PREFIX )


##### check for jpeg ############################

find_package( JPEG )
if( NOT JPEG_FOUND )
  # FIXME I think jpeg support must be required
  message(FATAL_ERROR "\njpeg are required, but not found on your system" )
endif( NOT JPEG_FOUND )

set( HAVE_LIBJPEG 1 )


##### check for png #############################

find_package( PNG )
if( NOT PNG_FOUND )
  # FIXME I think png support must be required
  message(FATAL_ERROR "\npng are required, but not found on your system" )
endif( NOT PNG_FOUND )

set( HAVE_LIBPNG 1 )


##### check for tiff ############################

if( WITH_TIFF )
  find_package( TIFF )
  if( NOT TIFF_FOUND )
    message(FATAL_ERROR "\ntiff are requested, but not found on your system" )
  endif( NOT TIFF_FOUND )
  set( HAVE_LIBTIFF 1)
endif( WITH_TIFF )


##### check for jasper ##########################

if( WITH_JASPER )
  find_package( Jasper )
  if( NOT JASPER_FOUND )
    message(FATAL_ERROR "\njasper are requested, but not found on your system" )
  endif( NOT JASPER_FOUND )
  set( HAVE_JASPER 1 )
endif( WITH_JASPER )


##### check for openexr #########################

if( WITH_OPENEXR )
  pkg_search_module( OPENEXR OpenEXR )
  if( NOT OPENEXR_FOUND )
    message(FATAL_ERROR "\nopenexr are requested, but not found on your system" )
  endif( NOT OPENEXR_FOUND )
  set( HAVE_EXR 1 )
endif( WITH_OPENEXR )


##### check for freetype2 #######################

pkg_search_module( FREETYPE freetype2 )
if( NOT FREETYPE_FOUND )
    message(FATAL_ERROR "\nfreetype2 are required, but not found on your system" )
endif( NOT FREETYPE_FOUND )

set( KDEINIT_USE_FONTCONFIG 1 )


##### check for fontconfig ######################

pkg_search_module( FONTCONFIG fontconfig )
if( NOT FONTCONFIG_FOUND )
    message(FATAL_ERROR "\nfontconfig are required, but not found on your system" )
endif( NOT FONTCONFIG_FOUND )


##### check for Xrandr ##########################

pkg_search_module( XRANDR xrandr )
if( XRANDR_FOUND )
  set( XRANDR_SUPPORT 1 )
endif( XRANDR_FOUND )


##### check for libxml-2.0 ######################

pkg_search_module( LIBXML2 libxml-2.0 )
if( NOT LIBXML2_FOUND )
    message(FATAL_ERROR "\nlibxml-2.0 are required, but not found on your system" )
endif( NOT LIBXML2_FOUND )


##### check for libxslt ######################

pkg_search_module( LIBXSLT libxslt )
if( NOT LIBXSLT_FOUND )
    message(FATAL_ERROR "\nlibxslt are required, but not found on your system" )
endif( NOT LIBXSLT_FOUND )


##### check for Qt3 #############################

find_package( Qt3 )
if( NOT QT_FOUND )
    message(FATAL_ERROR "\nQt3 are required, but not found on your system" )
endif( NOT QT_FOUND )


##### check for TQt #############################

pkg_search_module( TQT TQt )
if( NOT TQT_FOUND )
    message(FATAL_ERROR "\nTQt are required, but not found on your system" )
endif( NOT TQT_FOUND )

set( QT_MOC_EXECUTABLE "tmoc" )

# check if Qt is linked against Xft
set( bak_CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES} )
set( CMAKE_REQUIRED_LIBRARIES ${TQT_LDFLAGS} )
check_function_exists( XftInit KDEINIT_USE_XFT )
set( CMAKE_REQUIRED_LIBRARIES ${bak_CMAKE_REQUIRED_LIBRARIES} )
unset( bak_CMAKE_REQUIRED_LIBRARIES )


##### check for ALSA ############################
# FIXME I propose to drop support for very old ALSA versions (i.e 0.5)

if( WITH_ALSA )
  find_package( ALSA )
  if( NOT ALSA_FOUND )
    message(FATAL_ERROR "\nALSA support are requested, but not found on your system" )
  endif( NOT ALSA_FOUND )
  set( HAVE_LIBASOUND2 1 )

  # FIXME I'm not sure if order is correct
  check_include_file( "alsa/asoundlib.h" HAVE_ALSA_ASOUNDLIB_H )
  if( NOT HAVE_ALSA_ASOUNDLIB_H )
    check_include_file( "sys/asoundlib.h" HAVE_SYS_ASOUNDLIB_H )
  endif( NOT HAVE_ALSA_ASOUNDLIB_H )
endif( WITH_ALSA )


##### check for aRts ############################

if( WITH_ARTS )
  pkg_search_module( ARTS arts )
  if( ARTS_FOUND )
    set( ARTS_MCOPIDL_EXECUTABLE ${ARTS_PREFIX}/bin/mcopidl )
  else( ARTS_FOUND )
    message(FATAL_ERROR "\naRts are requested, but not found on your system" )
  endif( ARTS_FOUND )
else( WITH_ARTS )
  set( WITHOUT_ARTS 1 )
endif( WITH_ARTS )


##### check for Xft #############################

if( KDEINIT_USE_XFT )
  pkg_search_module( XFT xft )
  if( NOT XFT_FOUND )
      message(FATAL_ERROR "\nXft are required, but not found on your system" )
  endif( NOT XFT_FOUND )
endif( KDEINIT_USE_XFT )


##### check for libart ##########################

if( WITH_LIBART )
  pkg_search_module( LIBART libart-2.0 )
  if( NOT LIBART_FOUND )
    message(FATAL_ERROR "\nlibart-2.0 support are requested, but not found on your system" )
  endif( NOT LIBART_FOUND )
  set( HAVE_LIBART 1 )
endif( WITH_LIBART )


##### check for libidn ##########################

if( WITH_LIBIDN )
  pkg_search_module( LIBIDN libidn )
  if( NOT LIBIDN_FOUND )
    message(FATAL_ERROR "\nlibidn support are requested, but not found on your system" )
  endif( NOT LIBIDN_FOUND )
  set( HAVE_LIBIDN 1 )
  check_include_file( "idna.h" HAVE_IDNA_H )
  check_include_file( "punycode.h" HAVE_PUNYCODE_H )
  check_include_file( "stringprep.h" HAVE_STRINGPREP_H )
endif( WITH_LIBIDN )


##### check for openssl #########################

if( WITH_SSL )
  pkg_search_module( SSL openssl )
  if( NOT SSL_FOUND )
    message(FATAL_ERROR "\nSSL support are requested, but openssl is not found on your system" )
  endif( NOT SSL_FOUND )
  set( KSSL_HAVE_SSL 1 )
endif( WITH_SSL )


##### check for cups ############################
# FIXME CUPS must be at least 1.1.9

if( WITH_CUPS )
  find_package( Cups )
  if( NOT CUPS_FOUND )
    message(FATAL_ERROR "\nCUPS support are requested, but not found on your system" )
  endif( NOT CUPS_FOUND )
  set( HAVE_CUPS 1 )
  # FIXME if CUPS is at least 1.1.20, doesn't have password caching
  set( HAVE_CUPS_NO_PWD_CACHE 1 )
endif( WITH_CUPS )


##### check for LUA #############################

if( WITH_LUA )
  message(FATAL_ERROR "\nLUA support are not ready yet" )
  pkg_search_module( LUA lua )
  if( NOT LUA_FOUND )
    message(FATAL_ERROR "\nLUA support are requested, but not found on your system" )
  endif( NOT LUA_FOUND )
  set( HAVE_LUA 1 )
endif( WITH_LUA )


##### check for avahi ###########################

pkg_search_module( AVAHI avahi-qt3 )
if( NOT AVAHI_FOUND )
    message(FATAL_ERROR "\navahi support are requested, but not found on your system" )
endif( NOT AVAHI_FOUND )

set( AVAHI_API_0_6 1 )


##### check for aspell ##########################
# we need ASPELL_DATADIR too

if( WITH_ASPELL )
  find_package( ASPELL )
  if( NOT ASPELL_FOUND )
    message(FATAL_ERROR "\nASPELL support are requested, but not found on your system" )
  endif( NOT ASPELL_FOUND )
endif( WITH_ASPELL )


##### check for hspell ##########################

if( WITH_HSPELL )
  find_package( HSPELL )
  if( NOT HSPELL_FOUND )
    message(FATAL_ERROR "\nHSPELL support are requested, but not found on your system" )
  endif( NOT HSPELL_FOUND )
endif( WITH_HSPELL )


##### write configure files #####################

configure_file( config.h.cmake config.h )
configure_file( dcop/dcop-path.h.cmake dcop/dcop-path.h )
configure_file( kdecore/kdemacros.h.cmake kdecore/kdemacros.h )
configure_file( kjs/global.h.in kjs/global.h )
configure_file( kio/kssl/ksslconfig.h.cmake kio/kssl/ksslconfig.h )
configure_file( kdoctools/checkXML.cmake kdoctools/checkXML )


##### tools #####################################

set( KDE3_DCOPIDL_EXECUTABLE ${CMAKE_BINARY_DIR}/dcop/dcopidl/dcopidl )
set( KDE3_DCOPIDL2CPP_EXECUTABLE ${CMAKE_BINARY_DIR}/dcop/dcopidl2cpp/dcopidl2cpp )
set( KDE3_MEINPROC_EXECUTABLE ${CMAKE_BINARY_DIR}/kdoctools/meinproc )
set( KDE3_KCFGC_EXECUTABLE ${CMAKE_BINARY_DIR}/kdecore/kconfig_compiler/kconfig_compiler )


##### global compiler settings ##################

add_definitions(
  -DHAVE_CONFIG_H
  -DQT_NO_ASCII_CAST
  -DQT_CLEAN_NAMESPACE
  -DQT_NO_STL
  -DQT_NO_COMPAT
  -DQT_NO_TRANSLATION
)

set( CMAKE_SHARED_LINKER_FLAGS "-Wl,--no-undefined" )
set( CMAKE_MODULE_LINKER_FLAGS "-Wl,--no-undefined" )


##### headers ###################################

install( FILES
    kde.pot
  DESTINATION ${INCLUDE_INSTALL_DIR} )


##### subdirectories ############################

add_subdirectory( dcop )
add_subdirectory( libltdl )
add_subdirectory( kdefx )
add_subdirectory( kdecore )
add_subdirectory( kunittest )
add_subdirectory( kdeui )
add_subdirectory( kdesu )
add_subdirectory( kjs )
add_subdirectory( kwallet )
add_subdirectory( kio )
add_subdirectory( kded )

# FIXME this directory/target doesn't exists
# add_subdirectory( kded_post )

add_subdirectory( kdoctools )

# FIXME is not very important, I will back there later
# add_subdirectory( doc )

add_subdirectory( kparts )
add_subdirectory( kutils )
add_subdirectory( kspell2 )
add_subdirectory( kmdi )
add_subdirectory( kinit )
add_subdirectory( kdeprint )
add_subdirectory( kab )
add_subdirectory( kresources )
add_subdirectory( kabc )
add_subdirectory( arts )
add_subdirectory( interfaces )
add_subdirectory( kate )
add_subdirectory( kcert )
if( XRANDR_FOUND )
  add_subdirectory( krandr )
endif( XRANDR_FOUND )
add_subdirectory( khtml )
add_subdirectory( kcmshell )
add_subdirectory( kconf_update )
add_subdirectory( kdewidgets )
add_subdirectory( kimgio )
add_subdirectory( kioslave )
add_subdirectory( kstyles )
add_subdirectory( libkmid )
add_subdirectory( libkscreensaver )
add_subdirectory( networkstatus )
add_subdirectory( knewstuff )
add_subdirectory( licenses )
add_subdirectory( dnssd )
add_subdirectory( pics )
add_subdirectory( mimetypes )
