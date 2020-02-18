## install the cmake files

set(module_install_dir ${DATA_INSTALL_DIR}/cmake/modules)

# maybe FindHUNSPELL.cmake doesn't have to be installed ?
# This variable doesn't actually do anything, but it's probably a good idea
# to have it around, so there is a place where the modules which are not installed
# are listed explicitly:
set(cmakeFilesDontInstall
    FindHUpnp.cmake
)

# Explicitly list all files which will be installed.
# We don't use a GLOB anymore so we can have also cmake files here
# which are used inside kdelibs, but which don't have to be installed (and
# so become part of the public interface of kdelibs which has to be kept
# compatible).
set(cmakeFiles
    BasicFindPackageVersion.cmake.in
    check_installed_exports_file.cmake
    CheckPointerMember.cmake
    CheckStructMember.cmake
    cmake-modules-styleguide.txt
    FindACL.cmake
    FindAlsa.cmake
    FindAvahi.cmake
    FindBlueZ.cmake
    FindCanberra.cmake
    FindDBusMenuQt.cmake
    FindDjVuLibre.cmake
    FindDNSSD.cmake
    FindEigen3.cmake
    FindENCHANT.cmake
    FindEPub.cmake
    FindExiv2.cmake
    FindFFmpeg.cmake
    FindFLAC++.cmake
    FindFontconfig.cmake
    FindGettextPO.cmake
    FindGIO.cmake
    FindGLIB2.cmake
    FindGMP.cmake
    FindGObject.cmake
    FindGpgme.cmake
    FindGSSAPI.cmake
    FindHUNSPELL.cmake
    FindIBus.cmake
    FindKDE4Internal.cmake
    FindKDE4Workspace.cmake
    FindKipi.cmake
    FindKSane.cmake
    FindLibAttica.cmake
    FindLibGcrypt.cmake
    FindLibGPS.cmake
    FindLibintl.cmake
    FindLibKonq.cmake
    FindLibLZMA.cmake
    FindLibMms.cmake
    FindLibNXCL.cmake
    FindLIBPARTED.cmake
    FindLibSpectre.cmake
    FindLibSSH.cmake
    FindLibVNCServer.cmake
    FindLibXKlavier.cmake
    FindMPFR.cmake
    FindMtp.cmake
    FindMusicBrainz5.cmake
    FindNetworkManager.cmake
    FindOpenEXR.cmake
    FindOpenGLES.cmake
    FindPAM.cmake
    FindPCIUTILS.cmake
    FindPolkitQt-1.cmake
    FindPopplerQt4.cmake
    FindPulseAudio.cmake
    FindQalculate.cmake
    FindQCA2.cmake
    FindRAW1394.cmake
    FindSamba.cmake
    FindSane.cmake
    FindSensors.cmake
    FindSharedMimeInfo.cmake
    FindSCIM.cmake
    FindSpeechd.cmake
    FindSqlite.cmake
    FindSubversionLibrary.cmake
    FindSudo.cmake
    FindTaglib.cmake
    FindUDev.cmake
    FindUDisks2.cmake
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
    MacroLogFeature.cmake
    MacroOptionalAddSubdirectory.cmake
    MacroOptionalFindPackage.cmake
    MacroPushRequiredVars.cmake
    MacroWriteBasicCMakeVersionFile.cmake
    UnixAuth.cmake
)

install(FILES ${cmakeFiles} DESTINATION ${module_install_dir})
