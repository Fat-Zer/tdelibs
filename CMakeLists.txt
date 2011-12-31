#################################################
#
#  (C) 2010-2011 Serghei Amelian
#  serghei (DOT) amelian (AT) gmail.com
#
#  Improvements and feedback are welcome
#
#  This file is released under GPL >= 2
#
#################################################

cmake_minimum_required( VERSION 2.8 )


##### general package setup #####################

project( tdelibs )

set( PACKAGE tdelibs )
set( VERSION "3.5.13" )


##### include essential cmake modules ###########

include( FindPkgConfig )
include( CheckIncludeFile )
include( CheckSymbolExists )
include( CheckFunctionExists )
include( CheckCSourceCompiles )
include( CheckCXXSourceCompiles )
include( CheckTypeSize )
include( CheckCSourceRuns )
include( CheckLibraryExists )


##### include our cmake modules #################

set( CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules" )
include( TDEMacros )
include( TDESetupPaths )


##### setup and find essential stuff ############

tde_setup_paths( )
find_package( TQt )


##### add apidox targets ############

add_custom_target(apidox
  COMMAND "./generate_apidox" "${CMAKE_SOURCE_DIR}" "${CMAKE_BINARY_DIR}" "${HTML_INSTALL_DIR}" "/usr/share/qt3/doc/html"
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/cmake/")

add_custom_target(install-apidox
  COMMAND "./install_apidox" "${CMAKE_SOURCE_DIR}" "${CMAKE_BINARY_DIR}" "${HTML_INSTALL_DIR}" "${CMAKE_INSTALL_PREFIX}"
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/cmake/")


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
OPTION( WITH_UTEMPTER "Use utempter for utmp management" OFF )
OPTION( WITH_AVAHI "Enable AVAHI support" OFF )
OPTION( WITH_ELFICON "Enable ELF embedded icon support" OFF )

OPTION( WITH_ASPELL "Enable aspell support" OFF )
OPTION( WITH_HSPELL "Enable hspell support" OFF )


##### set PKG_CONFIG_PATH #######################

set( ENV{PKG_CONFIG_PATH} "${PKG_CONFIG_PATH}:${PKGCONFIG_INSTALL_DIR}:${LIB_INSTALL_DIR}/pkgconfig:$ENV{PKG_CONFIG_PATH}" )


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
  set( TDEINIT_SETUID 1 )
  set( TDEINIT_OOM_PROTECT 1 )
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

check_include_file( "ltdl.h" HAVE_LTDL_H )
if( NOT HAVE_LTDL_H )
  tde_message_fatal( "ltdl.h are required, but not found on your system" )
endif( )

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
check_include_file( "values.h" HAVE_VALUES_H )

# FIXME I'm not sure if test TIME_WITH_SYS_TIME are correct
check_include_file( "sys/time.h" HAVE_SYS_TIME_H )
check_include_file( "time.h" HAVE_TIME_H )
if( HAVE_SYS_TIME_H AND HAVE_TIME_H )
  set( TIME_WITH_SYS_TIME 1 )
endif( HAVE_SYS_TIME_H AND HAVE_TIME_H )

check_include_file( "valgrind/memcheck.h" HAVE_VALGRIND_MEMCHECK_H )
check_include_file( "values.h" HAVE_VALUES_H )
check_include_file( "X11/extensions/shape.h" HAVE_X11_EXTENSIONS_SHAPE_H )
check_include_file( "/usr/src/sys/gnu/i386/isa/sound/awe_voice.h" HAVE__USR_SRC_SYS_GNU_I386_ISA_SOUND_AWE_VOICE_H )
check_include_file( "/usr/src/sys/i386/isa/sound/awe_voice.h" HAVE__USR_SRC_SYS_I386_ISA_SOUND_AWE_VOICE_H )

check_function_exists( random HAVE_RANDOM )
check_symbol_exists( random "stdlib.h" HAVE_RANDOM_PROTO )

check_function_exists( srandom HAVE_SRANDOM )
check_symbol_exists( srandom "stdlib.h" HAVE_SRANDOM_PROTO )

check_function_exists( seteuid HAVE_SETEUID )
check_function_exists( setegid HAVE_SETEGID )

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

check_function_exists( strlcat HAVE_STRLCAT )
check_symbol_exists( strlcat "string.h" HAVE_STRLCAT_PROTO )

check_function_exists( strlcpy HAVE_STRLCPY )
check_symbol_exists( strlcpy "string.h" HAVE_STRLCPY_PROTO )

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
check_function_exists( strtoll HAVE_STRTOLL )
check_function_exists( socket HAVE_SOCKET )
check_function_exists( strfmon HAVE_STRFMON )
check_function_exists( stpcpy HAVE_STPCPY )
check_function_exists( readdir_r HAVE_READDIR_R )
check_function_exists( tcgetattr HAVE_TCGETATTR )
check_function_exists( tcsetattr HAVE_TCSETATTR )
check_function_exists( strcasecmp HAVE_STRCASECMP )
check_function_exists( strchr HAVE_STRCHR )
check_function_exists( strcmp HAVE_STRCMP )
check_function_exists( strrchr HAVE_STRRCHR )
check_function_exists( ptsname HAVE_PTSNAME )
check_function_exists( unlockpt HAVE_UNLOCKPT )
check_function_exists( _getpty HAVE__GETPTY )
check_function_exists( __argz_count HAVE___ARGZ_COUNT )
check_function_exists( __argz_next HAVE___ARGZ_NEXT )
check_function_exists( __argz_stringify HAVE___ARGZ_STRINGIFY )
check_function_exists( sendfile HAVE_SENDFILE )
check_function_exists( rindex HAVE_RINDEX )
check_function_exists( putenv HAVE_PUTENV )
check_function_exists( poll HAVE_POLL )
check_function_exists( memcpy HAVE_MEMCPY )
check_function_exists( madvise HAVE_MADVISE )
check_function_exists( getgroups HAVE_GETGROUPS )
check_function_exists( getcwd HAVE_GETCWD )
check_function_exists( dlerror HAVE_DLERROR )
check_function_exists( crypt HAVE_CRYPT )
check_function_exists( bcopy HAVE_BCOPY )
check_function_exists( alloca HAVE_ALLOCA )
check_function_exists( mmap HAVE_MMAP )
check_function_exists( munmap HAVE_MUNMAP )

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

set( CMAKE_REQUIRED_LIBRARIES util )
check_c_source_runs("
    #include <pty.h>
    int main(int argc, char* argv) {
      int master_fd, slave_fd;
      int result;
      result = openpty(&master_fd, &slave_fd, 0, 0, 0);
      return 0;
  }"
  HAVE_OPENPTY
)
if( HAVE_OPENPTY )
  set( LIB_UTIL util )
endif( )

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
if( WITH_SUDO_TDESU_BACKEND )
  if( __PATH_SUDO )
    set( DEFAULT_SUPER_USER_COMMAND ${__PATH_SUDO} )
  else( __PATH_SUDO )
    message( FATAL_ERROR "sudo was chosen as tdesu backend, but was not found in path." )
  endif( __PATH_SUDO )
endif( WITH_SUDO_TDESU_BACKEND )


#### set some constants #########################

set( LTDL_OBJDIR \".\" )
set( KDELIBSUFF "${LIB_SUFFIX}" )
set( kde_socklen_t socklen_t )


##### check for libdl ###########################

find_library( HAVE_LIBDL dl )
if( NOT HAVE_LIBDL-NOTFOUND )
  set( DL_LIBRARIES dl )
endif( NOT HAVE_LIBDL-NOTFOUND )


##### check for utempter ########################

if( WITH_UTEMPTER )
  check_include_file( utempter.h HAVE_UTEMPTER_H )
  check_library_exists( utempter addToUtmp "" HAVE_ADDTOUTEMP )
  if( HAVE_UTEMPTER_H AND HAVE_ADDTOUTEMP )
    set( HAVE_UTEMPTER 1 )
    set( UTEMPTER_LIBRARY utempter )
  else( )
    message(FATAL_ERROR "\nutempter are requested, but not found on your system" )
  endif( )
endif( )


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

set( TDEINIT_USE_FONTCONFIG 1 )


##### check for fontconfig ######################

pkg_search_module( FONTCONFIG fontconfig )
if( NOT FONTCONFIG_FOUND )
    message(FATAL_ERROR "\nfontconfig are required, but not found on your system" )
endif( NOT FONTCONFIG_FOUND )


##### check for Xrandr ##########################

pkg_search_module( XRANDR xrandr>=1.2 )
if( XRANDR_FOUND )
  set( XRANDR_SUPPORT 1 )
endif( XRANDR_FOUND )


##### check for xrender #########################

pkg_search_module( XRENDER xrender )
if( XRENDER_FOUND )
  set( HAVE_XRENDER 1 )
endif( XRENDER_FOUND )

##### check for xcomposite #########################

pkg_search_module( XCOMPOSITE xcomposite )
if( XCOMPOSITE_FOUND )
  set( HAVE_XCOMPOSITE 1 )
endif( XCOMPOSITE_FOUND )

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


##### check for glib-2.0 & friends ##############

pkg_search_module( GLIB2 glib-2.0 )
if( NOT GLIB2_FOUND )
  tde_message_fatal( "glib-2.0 are required, but not found on your system" )
endif( )

pkg_search_module( GOBJECT2 gobject-2.0 )
if( NOT GLIB2_FOUND )
  tde_message_fatal( "gobject-2.0 are required, but not found on your system" )
endif( )


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

if( TDEINIT_USE_XFT )
  pkg_search_module( XFT xft )
  if( NOT XFT_FOUND )
      message(FATAL_ERROR "\nXft are required, but not found on your system" )
  endif( NOT XFT_FOUND )
endif( TDEINIT_USE_XFT )


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
  set( HAVE_SSL 1 )
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

if( WITH_AVAHI )
  if( WITH_QT4 )
  endif( WITH_QT4 )

  if( WITH_QT3 )
    pkg_search_module( AVAHI avahi-tqt )
    if( NOT AVAHI_FOUND )
        message(FATAL_ERROR "\navahi support are requested, but not found on your system" )
    endif( NOT AVAHI_FOUND )
    set( AVAHI_API_0_6 1 )
  endif( WITH_QT3 )
endif( )


##### check for libr ###########################
# WARNING
# before altering the LIBR_VERSION check,
# please ensure that nothing changed in the
# duplicated data definitions present in
# kio/kio/kfileitem.cpp

if( WITH_ELFICON )
  pkg_search_module( LIBR libr )
  if( NOT LIBR_FOUND )
      message(FATAL_ERROR "\nelficon support was requested, but libr was not found on your system" )
  endif( NOT LIBR_FOUND )
  if( LIBR_VERSION != 0.6.0 )
      message(FATAL_ERROR "\nelficon support was requested, but the libr version on your system may not be compatible with TDE" )
  endif( LIBR_VERSION != 0.6.0 )
  set( HAVE_ELFICON 1 )
endif( )


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
configure_file( tdecore/kdemacros.h.cmake tdecore/kdemacros.h )
configure_file( kjs/global.h.in kjs/global.h )
configure_file( kio/kssl/ksslconfig.h.cmake kio/kssl/ksslconfig.h )
configure_file( kdoctools/checkXML.cmake kdoctools/checkXML )


##### tools #####################################

set( KDE3_DCOPIDL_EXECUTABLE ${CMAKE_BINARY_DIR}/dcop/dcopidl/dcopidl )
set( KDE3_DCOPIDL2CPP_EXECUTABLE ${CMAKE_BINARY_DIR}/dcop/dcopidl2cpp/dcopidl2cpp )
set( KDE3_MEINPROC_EXECUTABLE ${CMAKE_BINARY_DIR}/kdoctools/meinproc )
set( KDE3_KCFGC_EXECUTABLE ${CMAKE_BINARY_DIR}/tdecore/kconfig_compiler/kconfig_compiler )


##### global compiler settings ##################

add_definitions(
  -DHAVE_CONFIG_H
)

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TQT_CXX_FLAGS}" )
set( CMAKE_SHARED_LINKER_FLAGS "-Wl,--no-undefined" )
set( CMAKE_MODULE_LINKER_FLAGS "-Wl,--no-undefined" )


##### headers ###################################

install( FILES
    kde.pot
  DESTINATION ${INCLUDE_INSTALL_DIR} )


##### subdirectories ############################

add_subdirectory( dcop )
add_subdirectory( libltdl )
add_subdirectory( kglib )
add_subdirectory( tdefx )
add_subdirectory( tdecore )
add_subdirectory( kunittest )
add_subdirectory( tdeui )
add_subdirectory( tdesu )
add_subdirectory( kjs )
add_subdirectory( kwallet )
add_subdirectory( kio )
add_subdirectory( kded )

# FIXME this directory/target doesn't exists
# add_subdirectory( kded_post )

add_subdirectory( kdoctools )
add_subdirectory( doc )
add_subdirectory( kparts )
add_subdirectory( kutils )
add_subdirectory( kspell2 )
add_subdirectory( kmdi )
add_subdirectory( kinit )
add_subdirectory( tdeprint )
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
add_subdirectory( krsync )
add_subdirectory( licenses )
add_subdirectory( dnssd )
add_subdirectory( pics )
add_subdirectory( mimetypes )


##### install import cmake modules ###############

tde_install_export( )
