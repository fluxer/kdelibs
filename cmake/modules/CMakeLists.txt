## install the cmake files

# This variable doesn't actually do anything, but it's probably a good idea
# to have it around, so there is a place where the modules which are not installed
# are listed explicitly:
set(cmakeFilesDontInstall
    FindMPV.cmake
    FindUDev.cmake
    FindAvahi.cmake
    FindENCHANT.cmake
    FindACL.cmake
    FindLibCDIO.cmake
    FindDevinfo.cmake
    FindFFmpeg.cmake
    FindLibRaw.cmake
    FindLibJPEG.cmake
    FindOpenJPEG.cmake
    FindWebP.cmake
    FindExiv2.cmake
)

# Explicitly list all files which will be installed.
# We don't use a GLOB anymore so we can have also cmake files here
# which are used inside kdelibs, but which don't have to be installed (and
# so become part of the public interface of kdelibs which has to be kept
# compatible).
set(cmakeFiles
    cmake-modules-styleguide.txt
    FindDjVuLibre.cmake
    FindEPub.cmake
    FindFFmpegThumbnailer.cmake
    FindFontconfig.cmake
    FindGettextPO.cmake
    FindGLIB2.cmake
    FindGMP.cmake
    FindGphoto2.cmake
    FindKmod.cmake
    FindLibATASmart.cmake
    FindLibDeflate.cmake
    FindLibDRM.cmake
    FindLibLZMA.cmake
    FindLIBPARTED.cmake
    FindLibSpectre.cmake
    FindLibSSH.cmake
    FindLibTorrent.cmake
    FindLibUSB.cmake
    FindLibVNCServer.cmake
    FindLightDM.cmake
    FindLibGit2.cmake
    FindMPFR.cmake
    FindMtp.cmake
    FindPCIUTILS.cmake
    FindPoppler.cmake
    FindQalculate.cmake
    FindRAW1394.cmake
    FindSensors.cmake
    FindSharedMimeInfo.cmake
    FindSudo.cmake
    FindTaglib.cmake
    FindX11_XCB.cmake
    FindXCB.cmake
    kde4_exec.sh.in
)

install(
    FILES ${cmakeFiles}
    DESTINATION ${KDE4_DATA_INSTALL_DIR}/cmake/modules
)

install(
    FILES
    KDE4Defaults.cmake
    KDE4Macros.cmake
    DESTINATION ${KDE4_DATA_INSTALL_DIR}/cmake/KDELibs4
)
