#! /usr/bin/env bash
$EXTRACTRC `find . ../kioslave ../kfile -name \*.rc -o -name \*.ui` >> rc.cpp || exit 11
$XGETTEXT `find . ../kioslave ../kfile -name "*.cpp" -o -name "*.cc" -o -name "*.h" | grep -v "/tests"` -o $podir/kio4.pot
