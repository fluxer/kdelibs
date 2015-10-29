
include_directories(
 ${CMAKE_SOURCE_DIR}/kdecore/sonnet ${KDE4_KDECORE_INCLUDES}
 )

# you can find macro_optional_find_package() in kdelibs/cmake/modules/OptionalFindPackage.cmake
# it is the same as FIND_PACKAGE(<name>) but additionally creates an OPTION(WITH_<name>)
# so the checking for the software can be disabled via ccmake or -DWITH_<name>=OFF
macro_optional_find_package(HSPELL)
set_package_properties(HSPELL PROPERTIES
    URL "http://ivrix.org.il/projects/spell-checker/"
    TYPE OPTIONAL
    PURPOSE "Spell checking support for Hebrew"
)

if (HSPELL_FOUND)
   add_subdirectory( hspell )
endif (HSPELL_FOUND)

macro_optional_find_package(ENCHANT)
set_package_properties(ENCHANT PROPERTIES
    DESCRIPTION "Spell checking support via Enchant"
    URL "http://www.abisource.com/projects/enchant/"
    TYPE OPTIONAL
)

if(ENCHANT_FOUND)
    add_subdirectory(enchant)
endif()

# This is not needed for spell checking if Enchant is provided or only Hebrew
# spell checking is required
#
# macro_optional_find_package(HUNSPELL)
# set_package_properties(HUNSPELL PROPERTIES
#     PURPOSE "Spell checking support via Hunspell"
#     URL "http://hunspell.sourceforge.net/"
#     TYPE OPTIONAL
# )

if (HUNSPELL_FOUND)
   add_subdirectory( hunspell )
endif (HUNSPELL_FOUND)

