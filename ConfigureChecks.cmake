# NOTE: only add something here if it is really needed by all of kdelibs.
#     Otherwise please prefer adding to the relevant config-foo.h.cmake file,
#     and the CMakeLists.txt that generates it (or a separate ConfigureChecks.make file if you prefer)
#     to minimize recompilations and increase modularity.

include(CheckIncludeFile)
include(CheckIncludeFiles)
include(CheckSymbolExists)
include(CheckCXXSymbolExists)
include(CheckFunctionExists)
include(CheckLibraryExists)
include(CheckTypeSize)
include(CheckStructMember)
include(CheckCXXSourceCompiles)
include(CheckPrototypeDefinition)

# The FindKDE4.cmake module sets _KDE4_PLATFORM_DEFINITIONS with
# definitions like _GNU_SOURCE that are needed on each platform.
set(CMAKE_REQUIRED_DEFINITIONS ${_KDE4_PLATFORM_DEFINITIONS})

set( KDELIBSUFF ${LIB_SUFFIX} )

macro_bool_to_01(LIBINTL_FOUND ENABLE_NLS)              # kdecore

# now check for dlfcn.h using the cmake supplied CHECK_INCLUDE_FILES() macro
# If definitions like -D_GNU_SOURCE are needed for these checks they
# should be added to _KDE4_PLATFORM_DEFINITIONS when it is originally
# defined outside this file.  Here we include these definitions in
# CMAKE_REQUIRED_DEFINITIONS so they will be included in the build of
# checks below.
set(CMAKE_REQUIRED_DEFINITIONS ${_KDE4_PLATFORM_DEFINITIONS})

check_include_files(stdio.h       HAVE_STDIO_H)                        # various
check_include_files(stdlib.h      HAVE_STDLIB_H)                       # various
check_include_files(string.h      HAVE_STRING_H)                       # various
check_include_files(strings.h     HAVE_STRINGS_H)                      # various
check_include_files(sys/time.h    TIME_WITH_SYS_TIME)                  # kdecore, kioslave

check_include_files(fstab.h       HAVE_FSTAB_H)                        # kio, kdecore
check_include_files(limits.h      HAVE_LIMITS_H)                       # various
check_include_files(mntent.h      HAVE_MNTENT_H)                       # solid, kio, kdecore
check_include_files(sysent.h      HAVE_SYSENT_H)                       # kdecore
check_include_files(sys/stat.h    HAVE_SYS_STAT_H)                     # various
check_include_files(sys/types.h   HAVE_SYS_TYPES_H)                    # various
check_include_files(sys/select.h  HAVE_SYS_SELECT_H)                   # various
check_include_files(sys/param.h   HAVE_SYS_PARAM_H)                    # various
check_include_files("stdio.h;sys/mnttab.h"  HAVE_SYS_MNTTAB_H)         # kio, kdecore
check_include_files(sys/mntent.h  HAVE_SYS_MNTENT_H)                   # solid, kio, kdecore
check_include_files("sys/param.h;sys/mount.h"  HAVE_SYS_MOUNT_H)       # kio, kdecore
check_include_files(unistd.h      HAVE_UNISTD_H)                       # various
check_include_files(stdint.h      HAVE_STDINT_H)                       # various

check_include_files(errno.h       HAVE_ERRNO_H)                        # various
check_include_files(sys/time.h    HAVE_SYS_TIME_H)                     # various
check_include_files(langinfo.h    HAVE_LANGINFO_H)                     # kdecore

macro_bool_to_01(X11_XTest_FOUND HAVE_XTEST)                           # kdecore
macro_bool_to_01(X11_Xcursor_FOUND HAVE_XCURSOR)                       # kdeui
macro_bool_to_01(X11_Xfixes_FOUND HAVE_XFIXES)                         # kdeui
macro_bool_to_01(X11_Xrender_FOUND HAVE_XRENDER)                       # kio


# Use check_symbol_exists to check for symbols in a reliable
# cross-platform manner.  It accounts for different calling
# conventions and the possibility that the symbol is defined as a
# macro.  Note that some symbols require multiple includes in a
# specific order.  Refer to the man page for each symbol for which a
# check is to be added to get the proper set of headers.
check_symbol_exists(strtoll         "stdlib.h"                 HAVE_STRTOLL)     # kioslave
check_symbol_exists(S_ISSOCK        "sys/stat.h"               HAVE_S_ISSOCK)    # config.h
check_symbol_exists(vsnprintf       "stdio.h"                  HAVE_VSNPRINTF)   # config.h
check_symbol_exists(getgrouplist    "unistd.h;grp.h"           HAVE_GETGROUPLIST)# kdecore/fakes.c

check_function_exists(backtrace        HAVE_BACKTRACE)                # kdecore, kio
check_function_exists(fdatasync        HAVE_FDATASYNC)                # kdecore, kate

check_function_exists(sendfile        HAVE_SENDFILE)                  # kioslave
check_function_exists(gettimeofday    HAVE_GETTIMEOFDAY)              # kpty, kdeui, some tests
check_function_exists(getgrouplist    HAVE_GETGROUPLIST)              # kio

check_library_exists(volmgt volmgt_running "" HAVE_VOLMGT)            # various

# Check for libresolv
# e.g. on slackware 9.1 res_init() is only a define for __res_init, so we check both, Alex
set(HAVE_RESOLV_LIBRARY FALSE)                                        # kdecore, kdecore/network, kpac
check_library_exists(resolv res_init "" HAVE_RES_INIT_IN_RESOLV_LIBRARY)
check_library_exists(resolv __res_init "" HAVE___RES_INIT_IN_RESOLV_LIBRARY)
if (HAVE___RES_INIT_IN_RESOLV_LIBRARY OR HAVE_RES_INIT_IN_RESOLV_LIBRARY)
   set(HAVE_RESOLV_LIBRARY TRUE)
endif (HAVE___RES_INIT_IN_RESOLV_LIBRARY OR HAVE_RES_INIT_IN_RESOLV_LIBRARY)

check_library_exists(nsl gethostbyname "" HAVE_NSL_LIBRARY)
check_library_exists(socket connect "" HAVE_SOCKET_LIBRARY)

if (UNIX)

  # for kdecore (kpty) & kdesu

  check_include_files("sys/types.h;libutil.h" HAVE_LIBUTIL_H)
  check_include_files(util.h       HAVE_UTIL_H)
  check_include_files(termios.h    HAVE_TERMIOS_H)
  check_include_files(termio.h     HAVE_TERMIO_H)
  check_include_files(pty.h        HAVE_PTY_H)
  check_include_files(sys/stropts.h HAVE_SYS_STROPTS_H)
  check_include_files(sys/filio.h  HAVE_SYS_FILIO_H)

  set(UTIL_LIBRARY)

  check_library_exists(utempter addToUtmp "" HAVE_ADDTOUTEMP)
  check_include_files(utempter.h HAVE_UTEMPTER_H)
  if (HAVE_ADDTOUTEMP AND HAVE_UTEMPTER_H)
    set(HAVE_UTEMPTER 1)
    set(UTEMPTER_LIBRARY utempter)
  else (HAVE_ADDTOUTEMP AND HAVE_UTEMPTER_H)
    check_function_exists(login login_in_libc)
    if (NOT login_in_libc)
      check_library_exists(util login "" login_in_libutil)
      if (login_in_libutil)
        set(UTIL_LIBRARY util)
      endif (login_in_libutil)
    endif (NOT login_in_libc)
    if (CMAKE_SYSTEM_NAME MATCHES Linux OR CMAKE_SYSTEM_NAME MATCHES Darwin OR CMAKE_SYSTEM_NAME MATCHES GNU/FreeBSD OR CMAKE_SYSTEM_NAME STREQUAL GNU)
      set (HAVE_UTMPX)
    else (CMAKE_SYSTEM_NAME MATCHES Linux OR CMAKE_SYSTEM_NAME MATCHES Darwin OR CMAKE_SYSTEM_NAME MATCHES GNU/FreeBSD OR CMAKE_SYSTEM_NAME STREQUAL GNU)
      check_function_exists(getutxent HAVE_UTMPX)
    endif (CMAKE_SYSTEM_NAME MATCHES Linux OR CMAKE_SYSTEM_NAME MATCHES Darwin OR CMAKE_SYSTEM_NAME MATCHES GNU/FreeBSD OR CMAKE_SYSTEM_NAME STREQUAL GNU)
    if (HAVE_UTMPX)
      set(utmp utmpx)
      if (login_in_libutil)
        check_library_exists(util loginx "" HAVE_LOGINX)
      endif (login_in_libutil)
    else (HAVE_UTMPX)
      set(utmp utmp)
    endif (HAVE_UTMPX)
    if (login_in_libc OR login_in_libutil)
      set(HAVE_LOGIN 1)
    else (login_in_libc OR login_in_libutil)
      set(HAVE_LOGIN)
      check_struct_member("struct ${utmp}" "ut_type" "${utmp}.h" HAVE_STRUCT_UTMP_UT_TYPE)
      check_struct_member("struct ${utmp}" "ut_pid" "${utmp}.h" HAVE_STRUCT_UTMP_UT_PID)
      check_struct_member("struct ${utmp}" "ut_session" "${utmp}.h" HAVE_STRUCT_UTMP_UT_SESSION)
    endif (login_in_libc OR login_in_libutil)
    check_struct_member("struct ${utmp}" "ut_syslen" "${utmp}.h" HAVE_STRUCT_UTMP_UT_SYSLEN)
    check_struct_member("struct ${utmp}" "ut_id" "${utmp}.h" HAVE_STRUCT_UTMP_UT_ID)
  endif (HAVE_ADDTOUTEMP AND HAVE_UTEMPTER_H)

  check_function_exists(openpty openpty_in_libc)
  if (NOT openpty_in_libc)
    check_library_exists(util openpty "" openpty_in_libutil)
    if (openpty_in_libutil)
      set(UTIL_LIBRARY util)
    endif (openpty_in_libutil)
  endif (NOT openpty_in_libc)
  if (openpty_in_libc OR openpty_in_libutil)
    set(HAVE_OPENPTY 1)
  else (openpty_in_libc OR openpty_in_libutil)
    set(HAVE_OPENPTY)

    execute_process(
      COMMAND sh -c "
        for ptm in ptc ptmx ptm ptym/clone; do
          if test -c /dev/$ptm; then
            echo /dev/$ptm
            break
          fi
        done"
      OUTPUT_VARIABLE PTM_DEVICE
      OUTPUT_STRIP_TRAILING_WHITESPACE)
    message(STATUS "PTY multiplexer: ${PTM_DEVICE}")

    check_function_exists(revoke     HAVE_REVOKE)
    check_function_exists(_getpty    HAVE__GETPTY)
    check_function_exists(getpt      HAVE_GETPT)
    check_function_exists(grantpt    HAVE_GRANTPT)
    check_function_exists(unlockpt   HAVE_UNLOCKPT)
    check_function_exists(posix_openpt HAVE_POSIX_OPENPT)
  endif (openpty_in_libc OR openpty_in_libutil)

  check_function_exists(ptsname    HAVE_PTSNAME)
  check_function_exists(tcgetattr  HAVE_TCGETATTR)
  check_function_exists(tcsetattr  HAVE_TCSETATTR)
endif (UNIX)

check_function_exists(getmntinfo HAVE_GETMNTINFO)        # kdecore, kio
check_function_exists(initgroups HAVE_INITGROUPS)        # kde3support/k3process, kdesu
check_function_exists(mkstemps   HAVE_MKSTEMPS)          # dcop, kdecore/fakes.c
check_function_exists(mkstemp    HAVE_MKSTEMP)           # kdecore/fakes.c
check_function_exists(mkdtemp    HAVE_MKDTEMP)           # kdecore/fakes.c
check_function_exists(strlcpy    HAVE_STRLCPY)           # kdecore/fakes.c
check_function_exists(strlcat    HAVE_STRLCAT)           # kdecore/fakes.c
check_function_exists(strcasestr HAVE_STRCASESTR)        # kdecore/fakes.c
check_symbol_exists(strcasestr string.h          HAVE_STRCASESTR_PROTO)
check_function_exists(setenv     HAVE_SETENV)            # kdecore/fakes.c
check_function_exists(seteuid    HAVE_SETEUID)           # kdecore/fakes.c
check_function_exists(setmntent  HAVE_SETMNTENT)         # solid, kio, kdecore
check_function_exists(unsetenv   HAVE_UNSETENV)          # kdecore/fakes.c
check_function_exists(usleep     HAVE_USLEEP)            # kdecore/fakes.c, kdeui/qxembed

# check for prototypes [for functions provided by kdefakes when not available]

check_symbol_exists(mkstemps "stdlib.h;unistd.h" HAVE_MKSTEMPS_PROTO)
check_symbol_exists(mkdtemp "stdlib.h;unistd.h"  HAVE_MKDTEMP_PROTO)
check_symbol_exists(mkstemp "stdlib.h;unistd.h"  HAVE_MKSTEMP_PROTO)
check_symbol_exists(strlcat string.h             HAVE_STRLCAT_PROTO)
check_symbol_exists(strlcpy string.h             HAVE_STRLCPY_PROTO)
check_symbol_exists(res_init "sys/types.h;netinet/in.h;arpa/nameser.h;resolv.h" HAVE_RES_INIT_PROTO)
check_symbol_exists(setenv stdlib.h              HAVE_SETENV_PROTO)
check_symbol_exists(unsetenv stdlib.h            HAVE_UNSETENV_PROTO)
check_symbol_exists(usleep unistd.h              HAVE_USLEEP_PROTO)
check_symbol_exists(initgroups "unistd.h;sys/types.h;unistd.h;grp.h" HAVE_INITGROUPS_PROTO)
check_symbol_exists(setreuid unistd.h            HAVE_SETREUID_PROTO)
check_symbol_exists(trunc math.h                 HAVE_TRUNC)
if(NOT HAVE_TRUNC)
    # check_symbol_exists() may fail with either undefined refence (in C mode) or with wrong type (in C++ mode)
    check_prototype_definition(trunc "double trunc(double arg)" "0.0" "math.h" HAVE_TRUNC_PROTO)
    set(HAVE_TRUNC ${HAVE_TRUNC_PROTO})
endif()

# check for existing datatypes

check_cxx_source_compiles("
  #include <sys/types.h>
  #include <sys/statvfs.h>
  int main(){
    struct statvfs *mntbufp;
    int flags;
    return getmntinfo(&mntbufp, flags);
  }
" GETMNTINFO_USES_STATVFS )

check_struct_member(dirent d_type dirent.h HAVE_DIRENT_D_TYPE) # kdecore, kioslave/file

