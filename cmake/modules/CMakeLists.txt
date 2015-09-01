## install the cmake files

set(module_install_dir ${DATA_INSTALL_DIR}/cmake/modules )

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
    FindCarbon.cmake
    FindDBusMenuQt.cmake
    FindDNSSD.cmake
    FindENCHANT.cmake
    FindEigen.cmake
    FindEigen2.cmake
    FindExiv2.cmake
    FindFAM.cmake
    FindFFmpeg.cmake
    FindFlac.cmake
    FindFlex.cmake
    FindFontconfig.cmake
    FindGLIB2.cmake
    FindGMP.cmake
    FindGObject.cmake
    FindGSSAPI.cmake
    FindGStreamer.cmake
    FindGettext.cmake
    FindGphoto2.cmake
    FindHUNSPELL.cmake
    FindKDE4Internal.cmake
    FindKDE4Workspace.cmake
    FindKDevPlatform.cmake
    FindKdcraw.cmake
    FindKdeMultimedia.cmake
    FindKdepim.cmake
    FindKdepimLibs.cmake
    FindKexiv2.cmake
    FindKipi.cmake
    FindKopete.cmake
    FindKorundum.cmake
    FindLCMS.cmake
    FindLibArt.cmake
    FindLibAttica.cmake
    FindLibKonq.cmake
    FindLibLZMA.cmake
    FindLibXslt.cmake
    FindLibintl.cmake
    FindLinuxWirelesstools.cmake
    FindMsgfmt.cmake
    FindMusicBrainz.cmake
    FindMySQL.cmake
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
    FindTaglib.cmake
    FindUDisks2.cmake
    FindUSB.cmake
    FindWcecompat.cmake
    FindX11.cmake
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
    MacroEnsureVersion.cmake
    MacroLibrary.cmake
    MacroLogFeature.cmake
    MacroOptionalAddSubdirectory.cmake
    MacroOptionalDependPackage.cmake
    MacroOptionalFindPackage.cmake
    MacroPushRequiredVars.cmake
    MacroWriteBasicCMakeVersionFile.cmake
    PythonMacros.cmake
    SIPMacros.cmake
    check_installed_exports_file.cmake
    config-alsa.h.cmake
    create_exe_symlink.cmake
    kde4uic.cmake
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
