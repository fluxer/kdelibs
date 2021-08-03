## install the cmake files

set(module_install_dir ${KDE4_DATA_INSTALL_DIR}/cmake/modules)

# maybe FindHUNSPELL.cmake doesn't have to be installed ?
# This variable doesn't actually do anything, but it's probably a good idea
# to have it around, so there is a place where the modules which are not installed
# are listed explicitly:
set(cmakeFilesDontInstall
    FindMPV.cmake
    FindUDev.cmake
    FindAvahi.cmake
    FindDNSSD.cmake
    FindENCHANT.cmake
    FindACL.cmake
    FindLibCDIO.cmake
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
    FindFLAC++.cmake
    FindFontconfig.cmake
    FindGettextPO.cmake
    FindGLIB2.cmake
    FindGMP.cmake
    FindGpgme.cmake
    FindGphoto2.cmake
    FindGSSAPI.cmake
    FindHUNSPELL.cmake
    FindKDE4Internal.cmake
    FindKDE4Workspace.cmake
    FindKSane.cmake
    FindLibATASmart.cmake
    FindLibDRM.cmake
    FindLibGcrypt.cmake
    FindLibintl.cmake
    FindLibKonq.cmake
    FindLibLZMA.cmake
    FindLibMms.cmake
    FindLibNXCL.cmake
    FindLIBPARTED.cmake
    FindLibSpectre.cmake
    FindLibSSH.cmake
    FindLibVNCServer.cmake
    FindMPFR.cmake
    FindMtp.cmake
    FindNetworkManager.cmake
    FindOpenEXR.cmake
    FindOpenGLES.cmake
    FindPAM.cmake
    FindPCIUTILS.cmake
    FindPopplerQt4.cmake
    FindQalculate.cmake
    FindRAW1394.cmake
    FindSamba.cmake
    FindSane.cmake
    FindSensors.cmake
    FindSharedMimeInfo.cmake
    FindStrigi.cmake
    FindSpeechd.cmake
    FindSudo.cmake
    FindTaglib.cmake
    FindLibUSB.cmake
    FindWebP.cmake
    FindX11_XCB.cmake
    FindXCB.cmake
    kde4_cmake_uninstall.cmake.in
    KDE4Defaults.cmake
    KDE4Macros.cmake
    MacroBoolTo01.cmake
    MacroDBusAddActivationService.cmake
    MacroLibrary.cmake
    MacroOptionalAddSubdirectory.cmake
    MacroOptionalFindPackage.cmake
    UnixAuth.cmake
)

install(FILES ${cmakeFiles} DESTINATION ${module_install_dir})
