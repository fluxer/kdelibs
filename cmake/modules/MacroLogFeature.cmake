# This file defines the Feature Logging macros.
#
# MACRO_LOG_FEATURE(VAR FEATURE DESCRIPTION URL [REQUIRED [MIN_VERSION [COMMENTS]]])
#   Logs the information so that it can be displayed at the end
#   of the configure run
#   VAR : TRUE or FALSE, indicating whether the feature is supported
#   FEATURE: name of the feature, e.g. "libjpeg"
#   DESCRIPTION: description what this feature provides
#   URL: home page
#   REQUIRED: TRUE or FALSE, indicating whether the feature is required
#   MIN_VERSION: minimum version number. empty string if unneeded
#   COMMENTS: More info you may want to provide.  empty string if unnecessary
#
# Example:
#
# INCLUDE(MacroLogFeature)
#
# FIND_PACKAGE(JPEG)
# MACRO_LOG_FEATURE(JPEG_FOUND "libjpeg" "Support JPEG images" "http://www.ijg.org" TRUE "3.2a" "")
#

# Copyright (c) 2006, Alexander Neundorf, <neundorf@kde.org>
# Copyright (c) 2006, Allen Winter, <winter@kde.org>
# Copyright (c) 2009, Sebastian Trueg, <trueg@kde.org>
# Copyright (c) 2017-2020, Ivailo Monev, <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.


# in FeatureSummary.cmake since CMake 2.8.3
INCLUDE(FeatureSummary)

MACRO(MACRO_LOG_FEATURE _var _package _description _url ) # _required _minvers _comments)
    STRING(TOUPPER "${ARGV4}" _required)
    SET(_minvers "${ARGV5}")
    SET(_comments "${ARGV6}")

    SET_PACKAGE_PROPERTIES("${_package}" PROPERTIES
            DESCRIPTION "${_description}"
            URL "${_url}"
            PURPOSE "${_comments}"
    )
ENDMACRO(MACRO_LOG_FEATURE)
