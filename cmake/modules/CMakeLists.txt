## install the cmake files

set(module_install_dir ${DATA_INSTALL_DIR}/cmake/modules)

# maybe FindHUNSPELL.cmake doesn't have to be installed ?
# This variable doesn't actually do anything, but it's probably a good idea
# to have it around, so there is a place where the modules which are not installed
# are listed explicitly:
set(cmakeFilesDontInstall
    FindHUpnp.cmake
    FindUDev.cmake
)

# Explicitly list all files which will be installed.
# We don't use a GLOB anymore so we can have also cmake files here
# which are used inside kdelibs, but which don't have to be installed (and
# so become part of the public interface of kdelibs which has to be kept
# compatible).
set(cmakeFiles
    CheckPointerMember.cmake
    CheckPrototypeExists.cmake
    CheckStructMember.cmake
    FindACL.cmake
    FindAGG.cmake
    FindAkode.cmake
    FindAlsa.cmake
    FindAvahi.cmake
    FindBerkeleyDB.cmake
    FindBlueZ.cmake
    FindCanberra.cmake
    FindCHM.cmake
    FindCUPS.cmake
    FindDBusMenuQt.cmake
    FindKDevelop.cmake
    FindDjVuLibre.cmake
    FindDNSSD.cmake
    FindENCHANT.cmake
    FindEPub.cmake
    FindEigen.cmake
    FindEigen2.cmake
    FindExiv2.cmake
    FindFAM.cmake
    FindFFmpeg.cmake
    FindFlac.cmake
    FindFlex.cmake
    FindFontconfig.cmake
    FindLibGcrypt.cmake
    FindGLIB2.cmake
    FindGMP.cmake
    FindGObject.cmake
    FindGSSAPI.cmake
    FindGStreamer.cmake
    FindGettext.cmake
    FindGettextPO.cmake
    FindSpeechd.cmake
    FindGpgme.cmake
    FindGphoto2.cmake
    FindHUNSPELL.cmake
    FindKDE4Internal.cmake
    FindKDE4Workspace.cmake
    FindKDevPlatform.cmake
    FindKdepim.cmake
    FindKdepimLibs.cmake
    FindKipi.cmake
    FindKopete.cmake
    FindKorundum.cmake
    FindKSane.cmake
    FindKTp.cmake
    FindLCMS.cmake
    FindLCMS2.cmake
    FindLibArt.cmake
    FindLibAttica.cmake
    FindLibKasten.cmake
    FindLibKonq.cmake
    FindLibLZMA.cmake
    FindLibOkteta.cmake
    FindLibOktetaKasten.cmake
    FindLibXslt.cmake
    FindLibintl.cmake
    FindLibNXCL.cmake
    FindLibSpectre.cmake
    FindLibVNCServer.cmake
    FindLIBPARTED.cmake
    FindLinuxWirelesstools.cmake
    FindLibMms.cmake
    FindMusicBrainz.cmake
    FindMusicBrainz5.cmake
    FindMySQL.cmake
    FindMPFR.cmake
    FindNetworkManager.cmake
    FindOggVorbis.cmake
    FindOpenEXR.cmake
    FindPCRE.cmake
    FindPhonon.cmake
    FindPopplerQt4.cmake
    FindPostgreSQL.cmake
    FindPulseAudio.cmake
    FindPyKDE4.cmake
    FindPyQt4.cmake
    Qt4Macros.cmake
    Qt4ConfigDependentSettings.cmake
    FindPoppler.cmake
    FindPythonLibrary.cmake
    FindQCA2.cmake
    FindQImageBlitz.cmake
    FindQt4.cmake
    FindRUBY.cmake
    FindSIP.cmake
    FindSamba.cmake
    FindSane.cmake
    FindSasl2.cmake
    FindSharedMimeInfo.cmake
    FindSqlite.cmake
    FindStrigi.cmake
    FindSubversionLibrary.cmake
    FindTaglib.cmake
    FindUDisks2.cmake
    FindUSB.cmake
    FindXCB.cmake
    FindX11.cmake
    FindX11_XCB.cmake
    FindXKB.cmake
    FindXine.cmake
    FindXmms.cmake
    HandleImportedTargetsInCMakeRequiredLibraries.cmake
    KDE4Defaults.cmake
    KDE4Macros.cmake
    MacroAddLinkFlags.cmake
    MacroAdditionalCleanFiles.cmake
    MacroAppendIf.cmake
    MacroBoolTo01.cmake
    MacroLibrary.cmake
    MacroLogFeature.cmake
    MacroOptionalAddSubdirectory.cmake
    MacroOptionalFindPackage.cmake
    MacroPushRequiredVars.cmake
    MacroWriteBasicCMakeVersionFile.cmake
    PythonMacros.cmake
    SIPMacros.cmake
    check_installed_exports_file.cmake
    config-alsa.h.cmake
    create_exe_symlink.cmake
    cmake-modules-styleguide.txt
    kde4_cmake_uninstall.cmake.in
    BasicFindPackageVersion.cmake.in
    FindLibPython.py
    FindPyKDE4.py
    FindPyQt.py
    FindSIP.py
    PythonCompile.py
)

install(FILES ${cmakeFiles} DESTINATION ${module_install_dir})
