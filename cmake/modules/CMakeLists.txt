## install the cmake files

set(module_install_dir ${KDE4_DATA_INSTALL_DIR}/cmake/modules)

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
    FindSpeechd.cmake
    FindLibRaw.cmake
    FindLibJPEG.cmake
    FindOpenJPEG.cmake
)

# Explicitly list all files which will be installed.
# We don't use a GLOB anymore so we can have also cmake files here
# which are used inside kdelibs, but which don't have to be installed (and
# so become part of the public interface of kdelibs which has to be kept
# compatible).
set(cmakeFiles
    cmake-modules-styleguide.txt
    FindDBusMenuQt.cmake
    FindDjVuLibre.cmake
    FindEPub.cmake
    FindExiv2.cmake
    FindFFmpegThumbnailer.cmake
    FindFontconfig.cmake
    FindGettextPO.cmake
    FindGLIB2.cmake
    FindGMP.cmake
    FindGphoto2.cmake
    FindKDE4Internal.cmake
    FindKDE4Workspace.cmake
    FindKmod.cmake
    FindLibATASmart.cmake
    FindLibDRM.cmake
    FindLibintl.cmake
    FindLibKonq.cmake
    FindLibLZMA.cmake
    FindLibMms.cmake
    FindLIBPARTED.cmake
    FindLibSpectre.cmake
    FindLibSSH.cmake
    FindLibTorrent.cmake
    FindLibVNCServer.cmake
    FindLightDM.cmake
    FindLibDeflate.cmake
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
    FindLibUSB.cmake
    FindWebP.cmake
    FindX11_XCB.cmake
    FindXCB.cmake
    kde4_cmake_uninstall.cmake.in
    kde4_exec.sh.in
    KDE4Defaults.cmake
    KDE4Macros.cmake
)

install(FILES ${cmakeFiles} DESTINATION ${module_install_dir})
