if(KDE_PLATFORM_FEATURE_BINARY_INCOMPATIBLE_FEATURE_REDUCTION)
   set(KDNSSD_SETTINGS_BASE_CLASS "KCoreConfigSkeleton")
else()
   set(KDNSSD_SETTINGS_BASE_CLASS "KConfigSkeleton")
endif()

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/settings.kcfgc
"ClassName=Configuration
File=kcm_kdnssd.kcfg
GlobalEnums=false
Inherits=${KDNSSD_SETTINGS_BASE_CLASS}
ItemAccessors=false
MemberVariables=private
Mutators=true
NameSpace=DNSSD
SetUserTexts=false
Singleton=true
Visibility=KDNSSD_EXPORT
IncludeFiles=dnssd/dnssd_export.h
")

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/kcm_kdnssd.kcfg
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>
<kcfg xmlns=\"http://www.kde.org/standards/kcfg/1.0\"
      xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"
      xsi:schemaLocation=\"http://www.kde.org/standards/kcfg/1.0
      http://www.kde.org/standards/kcfg/1.0/kcfg.xsd\" >
 <kcfgfile name=\"kdnssdrc\" />
 <group name=\"browsing\" >
  <entry key=\"DomainList\" type=\"StringList\" >
   <label>Additional domains for browsing</label>
   <whatsthis>List of 'wide-area' (non link-local) domains that should be browsed.</whatsthis>
  </entry>
 </group>
</kcfg>
")
