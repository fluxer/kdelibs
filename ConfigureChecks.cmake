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
include(CheckStructHasMember)
include(CheckCXXSourceCompiles)
include(CheckPrototypeDefinition)
include(CMakePushCheckState)

# The FindKDE4.cmake module sets _KDE4_PLATFORM_DEFINITIONS with
# definitions like _GNU_SOURCE that are needed on each platform.
set(CMAKE_REQUIRED_DEFINITIONS ${_KDE4_PLATFORM_DEFINITIONS})


macro_bool_to_01(LIBINTL_FOUND ENABLE_NLS)              # kdecore
macro_bool_to_01(ACL_FOUND HAVE_POSIX_ACL)              # kio

check_include_files(string.h      HAVE_STRING_H)                       # various
check_include_files(sys/time.h    TIME_WITH_SYS_TIME)                  # kdecore, kioslave

check_include_files(fstab.h       HAVE_FSTAB_H)                        # kio, kdecore
check_include_files(limits.h      HAVE_LIMITS_H)                       # various
check_include_files(mntent.h      HAVE_MNTENT_H)                       # solid, kio, kdecore
check_include_files(sys/stat.h    HAVE_SYS_STAT_H)                     # various
check_include_files(sys/types.h   HAVE_SYS_TYPES_H)                    # various
check_include_files(sys/select.h  HAVE_SYS_SELECT_H)                   # various
check_include_files(sys/param.h   HAVE_SYS_PARAM_H)                    # various
check_include_files("stdio.h;sys/mnttab.h"  HAVE_SYS_MNTTAB_H)         # kio, kdecore
check_include_files(sys/mntent.h  HAVE_SYS_MNTENT_H)                   # solid, kio, kdecore
check_include_files("sys/param.h;sys/mount.h"  HAVE_SYS_MOUNT_H)       # kio, kdecore
check_include_files(unistd.h      HAVE_UNISTD_H)                       # various
check_include_files(stdint.h      HAVE_STDINT_H)                       # various
check_include_files(paths.h       HAVE_PATHS_H)                        # kdecore

check_include_files(sys/time.h    HAVE_SYS_TIME_H)                     # various

# TODO: separate to config-x11.h
macro_bool_to_01(X11_XTest_FOUND HAVE_XTEST)                           # kdecore
macro_bool_to_01(X11_Xcursor_FOUND HAVE_XCURSOR)                       # kdeui
macro_bool_to_01(X11_Xfixes_FOUND HAVE_XFIXES)                         # kdeui
macro_bool_to_01(X11_Xscreensaver_FOUND HAVE_XSCREENSAVER)             # kidletime
macro_bool_to_01(X11_XSync_FOUND HAVE_XSYNC)                           # kidletime


# Use check_symbol_exists to check for symbols in a reliable
# cross-platform manner.  It accounts for different calling
# conventions and the possibility that the symbol is defined as a
# macro.  Note that some symbols require multiple includes in a
# specific order.  Refer to the man page for each symbol for which a
# check is to be added to get the proper set of headers.
check_symbol_exists(strtoll         "stdlib.h"           HAVE_STRTOLL)      # kioslave
check_symbol_exists(getgrouplist    "unistd.h;grp.h"     HAVE_GETGROUPLIST) # kio

check_function_exists(backtrace     HAVE_BACKTRACE)                         # kdecore, kio
check_function_exists(fdatasync     HAVE_FDATASYNC)                         # kdecore, kate

check_function_exists(sendfile      HAVE_SENDFILE)                          # kioslave

check_symbol_exists(ttyname_r        "unistd.h"          HAVE_TTYNAME_R)    # kinit, kpty

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
  # for kpty
  check_include_files("sys/types.h;libutil.h" HAVE_LIBUTIL_H)
  check_include_files(termio.h     HAVE_TERMIO_H)
  check_include_files(pty.h        HAVE_PTY_H)
  check_include_files(sys/stropts.h HAVE_SYS_STROPTS_H)
  check_include_files(sys/filio.h  HAVE_SYS_FILIO_H)

  set(UTIL_LIBRARY)

  check_symbol_exists(setutxent "utmpx.h" HAVE_UTMPX)
  if (HAVE_UTMPX)
    set(utmp utmpx)
  else ()
    set(utmp utmp)
  endif ()

  cmake_reset_check_state()
  set(CMAKE_REQUIRED_LIBRARIES "util")
  check_symbol_exists(loginx "util.h" HAVE_UTIL_LOGINX)
  cmake_reset_check_state()
  check_symbol_exists(login "${utmp}.h" HAVE_LOGIN)
  if (NOT HAVE_LOGIN)
    cmake_reset_check_state()
    set(CMAKE_REQUIRED_LIBRARIES "util")
    check_symbol_exists(login "util.h" HAVE_UTIL_LOGIN)
    cmake_reset_check_state()
  endif ()

  if (HAVE_UTIL_LOGINX OR HAVE_UTIL_LOGIN)
    set(UTIL_LIBRARY "util")
  endif ()

  check_struct_has_member("struct ${utmp}" "ut_user" "${utmp}.h" HAVE_STRUCT_UTMP_UT_USER)
  check_struct_has_member("struct ${utmp}" "ut_type" "${utmp}.h" HAVE_STRUCT_UTMP_UT_TYPE)
  check_struct_has_member("struct ${utmp}" "ut_pid" "${utmp}.h" HAVE_STRUCT_UTMP_UT_PID)
  check_struct_has_member("struct ${utmp}" "ut_session" "${utmp}.h" HAVE_STRUCT_UTMP_UT_SESSION)
  check_struct_has_member("struct ${utmp}" "ut_syslen" "${utmp}.h" HAVE_STRUCT_UTMP_UT_SYSLEN)
  check_struct_has_member("struct ${utmp}" "ut_id" "${utmp}.h" HAVE_STRUCT_UTMP_UT_ID)

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
    set(HAVE_OPENPTY 0)

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
    check_function_exists(grantpt    HAVE_GRANTPT)
    check_function_exists(unlockpt   HAVE_UNLOCKPT)
    check_function_exists(posix_openpt HAVE_POSIX_OPENPT)
  endif (openpty_in_libc OR openpty_in_libutil)

  check_function_exists(ptsname_r  HAVE_PTSNAME_R)
endif (UNIX)

check_function_exists(getmntinfo HAVE_GETMNTINFO)        # kdecore, kio
check_function_exists(setmntent  HAVE_SETMNTENT)         # solid, kio, kdecore

check_symbol_exists(res_init "sys/types.h;netinet/in.h;arpa/nameser.h;resolv.h" HAVE_RES_INIT_PROTO)

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

check_struct_has_member("struct dirent" d_type dirent.h HAVE_DIRENT_D_TYPE) # kdecore, kioslave/file

