macro_optional_find_package(ENCHANT)
set_package_properties(ENCHANT PROPERTIES
    DESCRIPTION "Spell checking support via Enchant"
    URL "http://www.abisource.com/projects/enchant/"
    TYPE RECOMMENDED
)

macro_bool_to_01(ENCHANT_FOUND HAVE_ENCHANT)
