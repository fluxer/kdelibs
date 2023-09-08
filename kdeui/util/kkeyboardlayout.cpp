/*  This file is part of the KDE libraries
    Copyright (C) 2023 Ivailo Monev <xakepa10@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2, as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "config-kkeyserver.h"
#include "kkeyboardlayout.h"
#include "kstandarddirs.h"
#include "klocale.h"
#include "kdebug.h"

#include <QX11Info>
#include <QMap>
#include <QTimer>
#include <QProcess>

#if defined(HAVE_XKB)
#  include <X11/Xatom.h>
#  include <X11/Xlib.h>
#  include <X11/XKBlib.h>
#  include <X11/extensions/XKBrules.h>
#endif

static const int s_layouttimeout = 500;
static const char s_xkbseparator = ',';

// NOTE: scooped via scoop-rules.py script, make sure to pipe it to `sort -u -k 2 -t ',' -f`
static const struct modelDescriptionData {
    const char* name;
    const char* description;
} modelDescriptionTbl[] = {
    { "a4techKB21", I18N_NOOP("A4Tech KB-21") },
    { "a4techKBS8", I18N_NOOP("A4Tech KBS-8") },
    { "a4_rfkb23", I18N_NOOP("A4Tech Wireless Desktop RFKB-23") },
    { "airkey", I18N_NOOP("Acer AirKey V") },
    { "acer_c300", I18N_NOOP("Acer C300") },
    { "acer_ferrari4k", I18N_NOOP("Acer Ferrari 4000") },
    { "acer_laptop", I18N_NOOP("Acer laptop") },
    { "scorpius", I18N_NOOP("Advance Scorpius KI") },
    { "applealu_ansi", I18N_NOOP("Apple Aluminium (ANSI)") },
    { "applealu_iso", I18N_NOOP("Apple Aluminium (ISO)") },
    { "applealu_jis", I18N_NOOP("Apple Aluminium (JIS)") },
    { "apple_laptop", I18N_NOOP("Apple laptop") },
    { "apple", I18N_NOOP("Apple") },
    { "asus_laptop", I18N_NOOP("Asus laptop") },
    { "azonaRF2300", I18N_NOOP("Azona RF2300 Wireless Internet") },
    { "benqx730", I18N_NOOP("BenQ X-Touch 730") },
    { "benqx800", I18N_NOOP("BenQ X-Touch 800") },
    { "benqx", I18N_NOOP("BenQ X-Touch") },
    { "brother", I18N_NOOP("Brother Internet") },
    { "btc5090", I18N_NOOP("BTC 5090") },
    { "btc5113rf", I18N_NOOP("BTC 5113RF Multimedia") },
    { "btc5126t", I18N_NOOP("BTC 5126T") },
    { "btc6301urf", I18N_NOOP("BTC 6301URF") },
    { "btc9000", I18N_NOOP("BTC 9000") },
    { "btc9000a", I18N_NOOP("BTC 9000A") },
    { "btc9001ah", I18N_NOOP("BTC 9001AH") },
    { "btc9019u", I18N_NOOP("BTC 9019U") },
    { "btc9116u", I18N_NOOP("BTC 9116U Mini Wireless Internet and Gaming") },
    { "cherrybunlim", I18N_NOOP("Cherry B.UNLIMITED") },
    { "cherrybluea", I18N_NOOP("Cherry Blue Line CyBo@rd (alt.)") },
    { "cherryblue", I18N_NOOP("Cherry Blue Line CyBo@rd") },
    { "cherrycyboard", I18N_NOOP("Cherry CyBo@rd USB-Hub") },
    { "cherrycmexpert", I18N_NOOP("Cherry CyMotion Expert") },
    { "cymotionlinux", I18N_NOOP("Cherry CyMotion Master Linux") },
    { "cherryblueb", I18N_NOOP("Cherry CyMotion Master XPress") },
    { "chicony", I18N_NOOP("Chicony Internet") },
    { "chicony9885", I18N_NOOP("Chicony KB-9885") },
    { "chicony0108", I18N_NOOP("Chicony KU-0108") },
    { "chicony0420", I18N_NOOP("Chicony KU-0420") },
    { "chromebook", I18N_NOOP("Chromebook") },
    { "classmate", I18N_NOOP("Classmate PC") },
    { "armada", I18N_NOOP("Compaq Armada laptop") },
    { "compaqeak8", I18N_NOOP("Compaq Easy Access") },
    { "compaqik13", I18N_NOOP("Compaq Internet (13 keys)") },
    { "compaqik18", I18N_NOOP("Compaq Internet (18 keys)") },
    { "compaqik7", I18N_NOOP("Compaq Internet (7 keys)") },
    { "ipaq", I18N_NOOP("Compaq iPaq") },
    { "presario", I18N_NOOP("Compaq Presario laptop") },
    { "creativedw7000", I18N_NOOP("Creative Desktop Wireless 7000") },
    { "dell101", I18N_NOOP("Dell 101-key PC") },
    { "inspiron", I18N_NOOP("Dell Inspiron 6000/8000 laptop") },
    { "latitude", I18N_NOOP("Dell Latitude laptop") },
    { "precision_m", I18N_NOOP("Dell Precision M laptop") },
    { "dellm65", I18N_NOOP("Dell Precision M65 laptop") },
    { "dellsk8125", I18N_NOOP("Dell SK-8125") },
    { "dellsk8135", I18N_NOOP("Dell SK-8135") },
    { "dellusbmm", I18N_NOOP("Dell USB Multimedia") },
    { "dell", I18N_NOOP("Dell") },
    { "dexxa", I18N_NOOP("Dexxa Wireless Desktop") },
    { "diamond", I18N_NOOP("Diamond 9801/9802") },
    { "dtk2000", I18N_NOOP("DTK2000") },
    { "emachines", I18N_NOOP("eMachines m6800 laptop") },
    { "ennyah_dkb1008", I18N_NOOP("Ennyah DKB-1008") },
    { "everex", I18N_NOOP("Everex STEPnote") },
    { "compalfl90", I18N_NOOP("FL90") },
    { "fscaa1667g", I18N_NOOP("Fujitsu-Siemens Amilo laptop") },
    { "pc101", I18N_NOOP("Generic 101-key PC") },
    { "pc102", I18N_NOOP("Generic 102-key PC") },
    { "pc104alt", I18N_NOOP("Generic 104-key PC with L-shaped Enter key") },
    { "pc104", I18N_NOOP("Generic 104-key PC") },
    { "pc105", I18N_NOOP("Generic 105-key PC") },
    { "pc86", I18N_NOOP("Generic 86-key PC") },
    { "geniuscomfy", I18N_NOOP("Genius Comfy KB-12e") },
    { "genius", I18N_NOOP("Genius Comfy KB-16M/Multimedia KWD-910") },
    { "geniuscomfy2", I18N_NOOP("Genius Comfy KB-21e-Scroll") },
    { "geniuskb19e", I18N_NOOP("Genius KB-19e NB") },
    { "geniuskkb2050hs", I18N_NOOP("Genius KKB-2050HS") },
    { "gyration", I18N_NOOP("Gyration") },
    { "macintosh_hhk", I18N_NOOP("Happy Hacking for Mac") },
    { "hhk", I18N_NOOP("Happy Hacking") },
    { "hpi6", I18N_NOOP("Hewlett-Packard Internet") },
    { "hpmini110", I18N_NOOP("Hewlett-Packard Mini 110 laptop") },
    { "hp250x", I18N_NOOP("Hewlett-Packard NEC SK-2500 Multimedia") },
    { "hpnx9020", I18N_NOOP("Hewlett-Packard nx9020") },
    { "hp500fa", I18N_NOOP("Hewlett-Packard Omnibook 500 FA") },
    { "hp5xx", I18N_NOOP("Hewlett-Packard Omnibook 500") },
    { "hp6000", I18N_NOOP("Hewlett-Packard Omnibook 6000/6100") },
    { "hpxe3gc", I18N_NOOP("Hewlett-Packard Omnibook XE3 GC") },
    { "hpxe3gf", I18N_NOOP("Hewlett-Packard Omnibook XE3 GF") },
    { "hpxt1000", I18N_NOOP("Hewlett-Packard Omnibook XT1000") },
    { "hpdv5", I18N_NOOP("Hewlett-Packard Pavilion dv5") },
    { "hpzt11xx", I18N_NOOP("Hewlett-Packard Pavilion ZT1100") },
    { "honeywell_euroboard", I18N_NOOP("Honeywell Euroboard") },
    { "rapidaccess2", I18N_NOOP("IBM Rapid Access II") },
    { "rapidaccess", I18N_NOOP("IBM Rapid Access") },
    { "ibm_spacesaver", I18N_NOOP("IBM Space Saver") },
    { "thinkpad", I18N_NOOP("IBM ThinkPad 560Z/600/600E/A22E") },
    { "thinkpad60", I18N_NOOP("IBM ThinkPad R60/T60/R61/T61") },
    { "thinkpadz60", I18N_NOOP("IBM ThinkPad Z60m/Z60t/Z61m/Z61t") },
    { "flexpro", I18N_NOOP("Keytronic FlexPro") },
    { "kinesis", I18N_NOOP("Kinesis") },
    { "logiaccess", I18N_NOOP("Logitech Access") },
    { "logicda", I18N_NOOP("Logitech Cordless Desktop (alt.)") },
    { "logiex110", I18N_NOOP("Logitech Cordless Desktop EX110") },
    { "logicd_it", I18N_NOOP("Logitech Cordless Desktop iTouch") },
    { "logiclx300", I18N_NOOP("Logitech Cordless Desktop LX-300") },
    { "logicd_nav", I18N_NOOP("Logitech Cordless Desktop Navigator") },
    { "logicd_opt", I18N_NOOP("Logitech Cordless Desktop Optical") },
    { "logicdpa2", I18N_NOOP("Logitech Cordless Desktop Pro (2nd alt.)") },
    { "logicd", I18N_NOOP("Logitech Cordless Desktop") },
    { "logicfn", I18N_NOOP("Logitech Cordless Freedom/Desktop Navigator") },
    { "logidinovoedge", I18N_NOOP("Logitech diNovo Edge") },
    { "logidinovo", I18N_NOOP("Logitech diNovo") },
    { "logitech_g15", I18N_NOOP("Logitech G15 extra keys via G15daemon") },
    { "logii350", I18N_NOOP("Logitech Internet 350") },
    { "logicink", I18N_NOOP("Logitech Internet Navigator") },
    { "logiik", I18N_NOOP("Logitech Internet") },
    { "logiitc", I18N_NOOP("Logitech iTouch Cordless Y-RB6") },
    { "logiinkseusb", I18N_NOOP("Logitech iTouch Internet Navigator SE USB") },
    { "logiinkse", I18N_NOOP("Logitech iTouch Internet Navigator SE") },
    { "itouch", I18N_NOOP("Logitech iTouch") },
    { "logiultraxc", I18N_NOOP("Logitech Ultra-X Cordless Media Desktop") },
    { "logiultrax", I18N_NOOP("Logitech Ultra-X") },
    { "logitech_base", I18N_NOOP("Logitech") },
    { "macbook79", I18N_NOOP("MacBook/MacBook Pro (intl.)") },
    { "macbook78", I18N_NOOP("MacBook/MacBook Pro") },
    { "macintosh_old", I18N_NOOP("Macintosh Old") },
    { "macintosh", I18N_NOOP("Macintosh") },
    { "mx1998", I18N_NOOP("Memorex MX1998") },
    { "mx2500", I18N_NOOP("Memorex MX2500 EZ-Access") },
    { "mx2750", I18N_NOOP("Memorex MX2750") },
    { "microsoftccurve2k", I18N_NOOP("Microsoft Comfort Curve 2000") },
    { "microsoftprose", I18N_NOOP("Microsoft Internet Pro (Swedish)") },
    { "microsoftinet", I18N_NOOP("Microsoft Internet") },
    { "microsoftelite", I18N_NOOP("Microsoft Natural Elite") },
    { "microsoft4000", I18N_NOOP("Microsoft Natural Ergonomic 4000") },
    { "microsoftprooem", I18N_NOOP("Microsoft Natural Pro OEM") },
    { "microsoftprousb", I18N_NOOP("Microsoft Natural Pro USB/Internet Pro") },
    { "microsoftpro", I18N_NOOP("Microsoft Natural Pro/Internet Pro") },
    { "microsoft7000", I18N_NOOP("Microsoft Natural Wireless Ergonomic 7000") },
    { "microsoft", I18N_NOOP("Microsoft Natural") },
    { "microsoftoffice", I18N_NOOP("Microsoft Office Keyboard") },
    { "microsoftsurface", I18N_NOOP("Microsoft Surface") },
    { "microsoftmult", I18N_NOOP("Microsoft Wireless Multimedia 1.0A") },
    { "sk1300", I18N_NOOP("NEC SK-1300") },
    { "sk2500", I18N_NOOP("NEC SK-2500") },
    { "sk6200", I18N_NOOP("NEC SK-6200") },
    { "sk7100", I18N_NOOP("NEC SK-7100") },
    { "omnikey101", I18N_NOOP("Northgate OmniKey 101") },
    { "olpc", I18N_NOOP("OLPC") },
    { "oretec", I18N_NOOP("Ortek Multimedia/Internet MCK-800") },
    { "pc98", I18N_NOOP("PC-98") },
    { "propeller", I18N_NOOP("Propeller Voyager KTEZ-1000") },
    { "qtronix", I18N_NOOP("QTronix Scorpius 98N+") },
    { "samsung4500", I18N_NOOP("Samsung SDM 4500P") },
    { "samsung4510", I18N_NOOP("Samsung SDM 4510P") },
    { "sanwaskbkg3", I18N_NOOP("Sanwa Supply SKB-KG3") },
    { "silvercrest", I18N_NOOP("Silvercrest Multimedia Wireless") },
    { "apex300", I18N_NOOP("SteelSeries Apex 300 (Apex RAW)") },
    { "sun_type6_jp", I18N_NOOP("Sun Type 6 (Japanese)") },
    { "sun_type6_jp_usb", I18N_NOOP("Sun Type 6 USB (Japanese)") },
    { "sun_type6_unix_usb", I18N_NOOP("Sun Type 6 USB (Unix)") },
    { "sun_type6_euro_usb", I18N_NOOP("Sun Type 6/7 USB (European)") },
    { "sun_type6_usb", I18N_NOOP("Sun Type 6/7 USB") },
    { "sun_type7_euro_usb", I18N_NOOP("Sun Type 7 USB (European)") },
    { "sun_type7_jp_usb", I18N_NOOP("Sun Type 7 USB (Japanese)/Japanese 106-key") },
    { "sun_type7_unix_usb", I18N_NOOP("Sun Type 7 USB (Unix)") },
    { "sun_type7_usb", I18N_NOOP("Sun Type 7 USB") },
    { "sp_inet", I18N_NOOP("Super Power Multimedia") },
    { "sven", I18N_NOOP("SVEN Ergonomic 2500") },
    { "sven303", I18N_NOOP("SVEN Slim 303") },
    { "symplon", I18N_NOOP("Symplon PaceBook tablet") },
    { "targa_v811", I18N_NOOP("Targa Visionary 811") },
    { "toshiba_s3000", I18N_NOOP("Toshiba Satellite S3000") },
    { "teck227", I18N_NOOP("Truly Ergonomic 227") },
    { "teck229", I18N_NOOP("Truly Ergonomic 229") },
    { "trustda", I18N_NOOP("Trust Direct Access") },
    { "trust_slimline", I18N_NOOP("Trust Slimline") },
    { "trust", I18N_NOOP("Trust Wireless Classic") },
    { "tm2020", I18N_NOOP("TypeMatrix EZ-Reach 2020") },
    { "tm2030PS2", I18N_NOOP("TypeMatrix EZ-Reach 2030 PS2") },
    { "tm2030USB-102", I18N_NOOP("TypeMatrix EZ-Reach 2030 USB (102/105:EU mode)") },
    { "tm2030USB-106", I18N_NOOP("TypeMatrix EZ-Reach 2030 USB (106:JP mode)") },
    { "tm2030USB", I18N_NOOP("TypeMatrix EZ-Reach 2030 USB") },
    { "unitekkb1925", I18N_NOOP("Unitek KB-1925") },
    { "vsonku306", I18N_NOOP("ViewSonic KU-306 Internet") },
    { "winbook", I18N_NOOP("Winbook Model XP5") },
    { "yahoo", I18N_NOOP("Yahoo! Internet") }
};
static const qint16 modelDescriptionTblSize = sizeof(modelDescriptionTbl) / sizeof(modelDescriptionData);

static const struct layoutDescriptionData {
    const char* name;
    const char* description;
} layoutDescriptionTbl[] = {
    { "custom", I18N_NOOP("A user-defined custom Layout") },
    { "al", I18N_NOOP("Albanian") },
    { "et", I18N_NOOP("Amharic") },
    { "ma", I18N_NOOP("Arabic (Morocco)") },
    { "sy", I18N_NOOP("Arabic (Syria)") },
    { "ara", I18N_NOOP("Arabic") },
    { "am", I18N_NOOP("Armenian") },
    { "az", I18N_NOOP("Azerbaijani") },
    { "ml", I18N_NOOP("Bambara") },
    { "bd", I18N_NOOP("Bangla") },
    { "by", I18N_NOOP("Belarusian") },
    { "be", I18N_NOOP("Belgian") },
    { "dz", I18N_NOOP("Berber (Algeria, Latin)") },
    { "ba", I18N_NOOP("Bosnian") },
    { "brai", I18N_NOOP("Braille") },
    { "bg", I18N_NOOP("Bulgarian") },
    { "mm", I18N_NOOP("Burmese") },
    { "cn", I18N_NOOP("Chinese") },
    { "hr", I18N_NOOP("Croatian") },
    { "cz", I18N_NOOP("Czech") },
    { "dk", I18N_NOOP("Danish") },
    { "af", I18N_NOOP("Dari") },
    { "mv", I18N_NOOP("Dhivehi") },
    { "nl", I18N_NOOP("Dutch") },
    { "bt", I18N_NOOP("Dzongkha") },
    { "au", I18N_NOOP("English (Australian)") },
    { "cm", I18N_NOOP("English (Cameroon)") },
    { "gh", I18N_NOOP("English (Ghana)") },
    { "ng", I18N_NOOP("English (Nigeria)") },
    { "za", I18N_NOOP("English (South Africa)") },
    { "gb", I18N_NOOP("English (UK)") },
    { "us", I18N_NOOP("English (US)") },
    { "epo", I18N_NOOP("Esperanto") },
    { "ee", I18N_NOOP("Estonian") },
    { "fo", I18N_NOOP("Faroese") },
    { "ph", I18N_NOOP("Filipino") },
    { "fi", I18N_NOOP("Finnish") },
    { "ca", I18N_NOOP("French (Canada)") },
    { "cd", I18N_NOOP("French (Democratic Republic of the Congo)") },
    { "tg", I18N_NOOP("French (Togo)") },
    { "fr", I18N_NOOP("French") },
    { "ge", I18N_NOOP("Georgian") },
    { "at", I18N_NOOP("German (Austria)") },
    { "ch", I18N_NOOP("German (Switzerland)") },
    { "de", I18N_NOOP("German") },
    { "gr", I18N_NOOP("Greek") },
    { "il", I18N_NOOP("Hebrew") },
    { "hu", I18N_NOOP("Hungarian") },
    { "is", I18N_NOOP("Icelandic") },
    { "in", I18N_NOOP("Indian") },
    { "jv", I18N_NOOP("Indonesian (Javanese)") },
    { "id", I18N_NOOP("Indonesian (Latin)") },
    { "iq", I18N_NOOP("Iraqi") },
    { "ie", I18N_NOOP("Irish") },
    { "it", I18N_NOOP("Italian") },
    { "jp", I18N_NOOP("Japanese") },
    { "kz", I18N_NOOP("Kazakh") },
    { "kh", I18N_NOOP("Khmer (Cambodia)") },
    { "kr", I18N_NOOP("Korean") },
    { "kg", I18N_NOOP("Kyrgyz") },
    { "la", I18N_NOOP("Lao") },
    { "lv", I18N_NOOP("Latvian") },
    { "lt", I18N_NOOP("Lithuanian") },
    { "mk", I18N_NOOP("Macedonian") },
    { "my", I18N_NOOP("Malay (Jawi, Arabic Keyboard)") },
    { "mt", I18N_NOOP("Maltese") },
    { "mao", I18N_NOOP("Maori") },
    { "md", I18N_NOOP("Moldavian") },
    { "mn", I18N_NOOP("Mongolian") },
    { "me", I18N_NOOP("Montenegrin") },
    { "gn", I18N_NOOP("N'Ko (AZERTY)") },
    { "np", I18N_NOOP("Nepali") },
    { "no", I18N_NOOP("Norwegian") },
    { "ir", I18N_NOOP("Persian") },
    { "pl", I18N_NOOP("Polish") },
    { "br", I18N_NOOP("Portuguese (Brazil)") },
    { "pt", I18N_NOOP("Portuguese") },
    { "ro", I18N_NOOP("Romanian") },
    { "ru", I18N_NOOP("Russian") },
    { "rs", I18N_NOOP("Serbian") },
    { "lk", I18N_NOOP("Sinhala (phonetic)") },
    { "sk", I18N_NOOP("Slovak") },
    { "si", I18N_NOOP("Slovenian") },
    { "latam", I18N_NOOP("Spanish (Latin American)") },
    { "es", I18N_NOOP("Spanish") },
    { "ke", I18N_NOOP("Swahili (Kenya)") },
    { "tz", I18N_NOOP("Swahili (Tanzania)") },
    { "se", I18N_NOOP("Swedish") },
    { "tw", I18N_NOOP("Taiwanese") },
    { "tj", I18N_NOOP("Tajik") },
    { "th", I18N_NOOP("Thai") },
    { "bw", I18N_NOOP("Tswana") },
    { "tr", I18N_NOOP("Turkish") },
    { "tm", I18N_NOOP("Turkmen") },
    { "ua", I18N_NOOP("Ukrainian") },
    { "pk", I18N_NOOP("Urdu (Pakistan)") },
    { "uz", I18N_NOOP("Uzbek") },
    { "vn", I18N_NOOP("Vietnamese") },
    { "sn", I18N_NOOP("Wolof") }
};
static const qint16 layoutDescriptionTblSize = sizeof(layoutDescriptionTbl) / sizeof(layoutDescriptionData);

static const struct variantDescriptionData {
    const char* name;
    const char* name2;
    const char* description;
} variantDescriptionTbl[] = {
    { "gh", "akan", I18N_NOOP("Akan") },
    { "al", "plisi", I18N_NOOP("Albanian (Plisi)") },
    { "al", "veqilharxhi", I18N_NOOP("Albanian (Veqilharxhi)") },
    { "dz", "ar", I18N_NOOP("Arabic (Algeria)") },
    { "ara", "azerty", I18N_NOOP("Arabic (AZERTY)") },
    { "ara", "azerty_digits", I18N_NOOP("Arabic (AZERTY, Eastern Arabic numerals)") },
    { "ara", "buckwalter", I18N_NOOP("Arabic (Buckwalter)") },
    { "ara", "digits", I18N_NOOP("Arabic (Eastern Arabic numerals)") },
    { "ara", "mac", I18N_NOOP("Arabic (Macintosh)") },
    { "ara", "olpc", I18N_NOOP("Arabic (OLPC)") },
    { "pk", "ara", I18N_NOOP("Arabic (Pakistan)") },
    { "ara", "qwerty", I18N_NOOP("Arabic (QWERTY)") },
    { "ara", "qwerty_digits", I18N_NOOP("Arabic (QWERTY, Eastern Arabic numerals)") },
    { "am", "eastern-alt", I18N_NOOP("Armenian (alt. eastern)") },
    { "am", "phonetic-alt", I18N_NOOP("Armenian (alt. phonetic)") },
    { "am", "eastern", I18N_NOOP("Armenian (eastern)") },
    { "am", "phonetic", I18N_NOOP("Armenian (phonetic)") },
    { "am", "western", I18N_NOOP("Armenian (western)") },
    { "es", "ast", I18N_NOOP("Asturian (Spain, with bottom-dot H and L)") },
    { "gh", "avn", I18N_NOOP("Avatime") },
    { "az", "cyrillic", I18N_NOOP("Azerbaijani (Cyrillic)") },
    { "in", "ben", I18N_NOOP("Bangla (India)") },
    { "in", "ben_inscript", I18N_NOOP("Bangla (India, Baishakhi InScript)") },
    { "in", "ben_baishakhi", I18N_NOOP("Bangla (India, Baishakhi)") },
    { "in", "ben_bornona", I18N_NOOP("Bangla (India, Bornona)") },
    { "in", "ben_gitanjali", I18N_NOOP("Bangla (India, Gitanjali)") },
    { "in", "ben_probhat", I18N_NOOP("Bangla (India, Probhat)") },
    { "bd", "probhat", I18N_NOOP("Bangla (Probhat)") },
    { "ru", "bak", I18N_NOOP("Bashkirian") },
    { "by", "intl", I18N_NOOP("Belarusian (intl.)") },
    { "by", "latin", I18N_NOOP("Belarusian (Latin)") },
    { "by", "legacy", I18N_NOOP("Belarusian (legacy)") },
    { "be", "oss", I18N_NOOP("Belgian (alt.)") },
    { "be", "iso-alternate", I18N_NOOP("Belgian (ISO, alt.)") },
    { "be", "oss_latin9", I18N_NOOP("Belgian (Latin-9 only, alt.)") },
    { "be", "nodeadkeys", I18N_NOOP("Belgian (no dead keys)") },
    { "be", "wang", I18N_NOOP("Belgian (Wang 724 AZERTY)") },
    { "dz", "ber", I18N_NOOP("Berber (Algeria, Tifinagh)") },
    { "ma", "tifinagh-alt", I18N_NOOP("Berber (Morocco, Tifinagh alt.)") },
    { "ma", "tifinagh-extended-phonetic", I18N_NOOP("Berber (Morocco, Tifinagh extended phonetic)") },
    { "ma", "tifinagh-extended", I18N_NOOP("Berber (Morocco, Tifinagh extended)") },
    { "ma", "tifinagh-phonetic", I18N_NOOP("Berber (Morocco, Tifinagh phonetic)") },
    { "ma", "tifinagh-alt-phonetic", I18N_NOOP("Berber (Morocco, Tifinagh phonetic, alt.)") },
    { "ma", "tifinagh", I18N_NOOP("Berber (Morocco, Tifinagh)") },
    { "ba", "us", I18N_NOOP("Bosnian (US)") },
    { "ba", "unicodeus", I18N_NOOP("Bosnian (US, with Bosnian digraphs)") },
    { "ba", "unicode", I18N_NOOP("Bosnian (with Bosnian digraphs)") },
    { "ba", "alternatequotes", I18N_NOOP("Bosnian (with guillemets)") },
    { "brai", "left_hand_invert", I18N_NOOP("Braille (left-handed inverted thumb)") },
    { "brai", "left_hand", I18N_NOOP("Braille (left-handed)") },
    { "brai", "right_hand_invert", I18N_NOOP("Braille (right-handed inverted thumb)") },
    { "brai", "right_hand", I18N_NOOP("Braille (right-handed)") },
    { "bg", "bekl", I18N_NOOP("Bulgarian (enhanced)") },
    { "bg", "bas_phonetic", I18N_NOOP("Bulgarian (new phonetic)") },
    { "bg", "phonetic", I18N_NOOP("Bulgarian (traditional phonetic)") },
    { "mm", "zawgyi", I18N_NOOP("Burmese Zawgyi") },
    { "cm", "azerty", I18N_NOOP("Cameroon (AZERTY, intl.)") },
    { "cm", "dvorak", I18N_NOOP("Cameroon (Dvorak, intl.)") },
    { "cm", "qwerty", I18N_NOOP("Cameroon Multilingual (QWERTY, intl.)") },
    { "ca", "multix", I18N_NOOP("Canadian (intl.)") },
    { "ca", "multi", I18N_NOOP("Canadian (intl., 1st part)") },
    { "ca", "multi-2gr", I18N_NOOP("Canadian (intl., 2nd part)") },
    { "es", "cat", I18N_NOOP("Catalan (Spain, with middle-dot L)") },
    { "us", "chr", I18N_NOOP("Cherokee") },
    { "ru", "cv_latin", I18N_NOOP("Chuvash (Latin)") },
    { "ru", "cv", I18N_NOOP("Chuvash") },
    { "ie", "CloGaelach", I18N_NOOP("CloGaelach") },
    { "ua", "crh_alt", I18N_NOOP("Crimean Tatar (Turkish Alt-Q)") },
    { "ua", "crh_f", I18N_NOOP("Crimean Tatar (Turkish F)") },
    { "ua", "crh", I18N_NOOP("Crimean Tatar (Turkish Q)") },
    { "hr", "us", I18N_NOOP("Croatian (US)") },
    { "hr", "unicodeus", I18N_NOOP("Croatian (US, with Croatian digraphs)") },
    { "hr", "unicode", I18N_NOOP("Croatian (with Croatian digraphs)") },
    { "hr", "alternatequotes", I18N_NOOP("Croatian (with guillemets)") },
    { "cz", "qwerty", I18N_NOOP("Czech (QWERTY)") },
    { "cz", "qwerty_bksl", I18N_NOOP("Czech (QWERTY, extended backslash)") },
    { "cz", "qwerty-mac", I18N_NOOP("Czech (QWERTY, Macintosh)") },
    { "cz", "ucw", I18N_NOOP("Czech (UCW, only accented letters)") },
    { "cz", "dvorak-ucw", I18N_NOOP("Czech (US, Dvorak, UCW support)") },
    { "cz", "bksl", I18N_NOOP("Czech (with <\\|> key)") },
    { "dk", "dvorak", I18N_NOOP("Danish (Dvorak)") },
    { "dk", "mac", I18N_NOOP("Danish (Macintosh)") },
    { "dk", "mac_nodeadkeys", I18N_NOOP("Danish (Macintosh, no dead keys)") },
    { "dk", "nodeadkeys", I18N_NOOP("Danish (no dead keys)") },
    { "dk", "winkeys", I18N_NOOP("Danish (Windows)") },
    { "af", "fa-olpc", I18N_NOOP("Dari (Afghanistan, OLPC)") },
    { "nl", "mac", I18N_NOOP("Dutch (Macintosh)") },
    { "nl", "std", I18N_NOOP("Dutch (standard)") },
    { "nl", "us", I18N_NOOP("Dutch (US)") },
    { "ca", "eng", I18N_NOOP("English (Canada)") },
    { "us", "dvorak-classic", I18N_NOOP("English (classic Dvorak)") },
    { "us", "colemak", I18N_NOOP("English (Colemak)") },
    { "us", "colemak_dh_iso", I18N_NOOP("English (Colemak-DH ISO)") },
    { "us", "colemak_dh", I18N_NOOP("English (Colemak-DH)") },
    { "us", "dvorak", I18N_NOOP("English (Dvorak)") },
    { "us", "dvorak-alt-intl", I18N_NOOP("English (Dvorak, alt. intl.)") },
    { "us", "dvorak-intl", I18N_NOOP("English (Dvorak, intl., with dead keys)") },
    { "us", "dvorak-l", I18N_NOOP("English (Dvorak, left-handed)") },
    { "us", "dvorak-mac", I18N_NOOP("English (Dvorak, Macintosh)") },
    { "us", "dvorak-r", I18N_NOOP("English (Dvorak, right-handed)") },
    { "gh", "gillbt", I18N_NOOP("English (Ghana, GILLBT)") },
    { "gh", "generic", I18N_NOOP("English (Ghana, multilingual)") },
    { "in", "eng", I18N_NOOP("English (India, with rupee)") },
    { "us", "altgr-intl", I18N_NOOP("English (intl., with AltGr dead keys)") },
    { "us", "mac", I18N_NOOP("English (Macintosh)") },
    { "ml", "us-intl", I18N_NOOP("English (Mali, US, intl.)") },
    { "ml", "us-mac", I18N_NOOP("English (Mali, US, Macintosh)") },
    { "us", "norman", I18N_NOOP("English (Norman)") },
    { "us", "dvp", I18N_NOOP("English (programmer Dvorak)") },
    { "us", "olpc2", I18N_NOOP("English (the divide/multiply toggle the layout)") },
    { "gb", "colemak", I18N_NOOP("English (UK, Colemak)") },
    { "gb", "colemak_dh", I18N_NOOP("English (UK, Colemak-DH)") },
    { "gb", "dvorak", I18N_NOOP("English (UK, Dvorak)") },
    { "gb", "dvorakukp", I18N_NOOP("English (UK, Dvorak, with UK punctuation)") },
    { "gb", "extd", I18N_NOOP("English (UK, extended, Windows)") },
    { "gb", "intl", I18N_NOOP("English (UK, intl., with dead keys)") },
    { "gb", "mac", I18N_NOOP("English (UK, Macintosh)") },
    { "gb", "mac_intl", I18N_NOOP("English (UK, Macintosh, intl.)") },
    { "us", "alt-intl", I18N_NOOP("English (US, alt. intl.)") },
    { "us", "euro", I18N_NOOP("English (US, euro on 5)") },
    { "us", "intl", I18N_NOOP("English (US, intl., with dead keys)") },
    { "us", "symbolic", I18N_NOOP("English (US, Symbolic)") },
    { "us", "workman", I18N_NOOP("English (Workman)") },
    { "us", "workman-intl", I18N_NOOP("English (Workman, intl., with dead keys)") },
    { "br", "nativo-epo", I18N_NOOP("Esperanto (Brazil, Nativo)") },
    { "epo", "legacy", I18N_NOOP("Esperanto (legacy)") },
    { "pt", "nativo-epo", I18N_NOOP("Esperanto (Portugal, Nativo)") },
    { "ee", "dvorak", I18N_NOOP("Estonian (Dvorak)") },
    { "ee", "nodeadkeys", I18N_NOOP("Estonian (no dead keys)") },
    { "ee", "us", I18N_NOOP("Estonian (US)") },
    { "gh", "ewe", I18N_NOOP("Ewe") },
    { "fo", "nodeadkeys", I18N_NOOP("Faroese (no dead keys)") },
    { "ph", "capewell-dvorak-bay", I18N_NOOP("Filipino (Capewell-Dvorak, Baybayin)") },
    { "ph", "capewell-dvorak", I18N_NOOP("Filipino (Capewell-Dvorak, Latin)") },
    { "ph", "capewell-qwerf2k6-bay", I18N_NOOP("Filipino (Capewell-QWERF 2006, Baybayin)") },
    { "ph", "capewell-qwerf2k6", I18N_NOOP("Filipino (Capewell-QWERF 2006, Latin)") },
    { "ph", "colemak-bay", I18N_NOOP("Filipino (Colemak, Baybayin)") },
    { "ph", "colemak", I18N_NOOP("Filipino (Colemak, Latin)") },
    { "ph", "dvorak-bay", I18N_NOOP("Filipino (Dvorak, Baybayin)") },
    { "ph", "dvorak", I18N_NOOP("Filipino (Dvorak, Latin)") },
    { "ph", "qwerty-bay", I18N_NOOP("Filipino (QWERTY, Baybayin)") },
    { "fi", "classic", I18N_NOOP("Finnish (classic)") },
    { "fi", "nodeadkeys", I18N_NOOP("Finnish (classic, no dead keys)") },
    { "fi", "mac", I18N_NOOP("Finnish (Macintosh)") },
    { "fi", "winkeys", I18N_NOOP("Finnish (Windows)") },
    { "fr", "oss", I18N_NOOP("French (alt.)") },
    { "fr", "oss_latin9", I18N_NOOP("French (alt., Latin-9 only)") },
    { "fr", "oss_nodeadkeys", I18N_NOOP("French (alt., no dead keys)") },
    { "fr", "azerty", I18N_NOOP("French (AZERTY)") },
    { "fr", "afnor", I18N_NOOP("French (AZERTY, AFNOR)") },
    { "fr", "bepo", I18N_NOOP("French (BEPO)") },
    { "fr", "bepo_afnor", I18N_NOOP("French (BEPO, AFNOR)") },
    { "fr", "bepo_latin9", I18N_NOOP("French (BEPO, Latin-9 only)") },
    { "fr", "bre", I18N_NOOP("French (Breton)") },
    { "cm", "french", I18N_NOOP("French (Cameroon)") },
    { "ca", "fr-dvorak", I18N_NOOP("French (Canada, Dvorak)") },
    { "ca", "fr-legacy", I18N_NOOP("French (Canada, legacy)") },
    { "fr", "dvorak", I18N_NOOP("French (Dvorak)") },
    { "fr", "latin9", I18N_NOOP("French (legacy, alt.)") },
    { "fr", "latin9_nodeadkeys", I18N_NOOP("French (legacy, alt., no dead keys)") },
    { "fr", "mac", I18N_NOOP("French (Macintosh)") },
    { "ml", "fr-oss", I18N_NOOP("French (Mali, alt.)") },
    { "ma", "french", I18N_NOOP("French (Morocco)") },
    { "fr", "nodeadkeys", I18N_NOOP("French (no dead keys)") },
    { "ch", "fr", I18N_NOOP("French (Switzerland)") },
    { "ch", "fr_mac", I18N_NOOP("French (Switzerland, Macintosh)") },
    { "ch", "fr_nodeadkeys", I18N_NOOP("French (Switzerland, no dead keys)") },
    { "fr", "us", I18N_NOOP("French (US)") },
    { "it", "fur", I18N_NOOP("Friulian (Italy)") },
    { "gh", "fula", I18N_NOOP("Fula") },
    { "gh", "ga", I18N_NOOP("Ga") },
    { "ge", "ergonomic", I18N_NOOP("Georgian (ergonomic)") },
    { "fr", "geo", I18N_NOOP("Georgian (France, AZERTY Tskapo)") },
    { "it", "geo", I18N_NOOP("Georgian (Italy)") },
    { "ge", "mess", I18N_NOOP("Georgian (MESS)") },
    { "at", "mac", I18N_NOOP("German (Austria, Macintosh)") },
    { "at", "nodeadkeys", I18N_NOOP("German (Austria, no dead keys)") },
    { "de", "deadacute", I18N_NOOP("German (dead acute)") },
    { "de", "deadgraveacute", I18N_NOOP("German (dead grave acute)") },
    { "de", "deadtilde", I18N_NOOP("German (dead tilde)") },
    { "de", "dvorak", I18N_NOOP("German (Dvorak)") },
    { "de", "e1", I18N_NOOP("German (E1)") },
    { "de", "e2", I18N_NOOP("German (E2)") },
    { "de", "mac", I18N_NOOP("German (Macintosh)") },
    { "de", "mac_nodeadkeys", I18N_NOOP("German (Macintosh, no dead keys)") },
    { "de", "neo", I18N_NOOP("German (Neo 2)") },
    { "de", "nodeadkeys", I18N_NOOP("German (no dead keys)") },
    { "de", "qwerty", I18N_NOOP("German (QWERTY)") },
    { "ch", "legacy", I18N_NOOP("German (Switzerland, legacy)") },
    { "ch", "de_mac", I18N_NOOP("German (Switzerland, Macintosh)") },
    { "ch", "de_nodeadkeys", I18N_NOOP("German (Switzerland, no dead keys)") },
    { "de", "T3", I18N_NOOP("German (T3)") },
    { "de", "us", I18N_NOOP("German (US)") },
    { "gr", "extended", I18N_NOOP("Greek (extended)") },
    { "gr", "nodeadkeys", I18N_NOOP("Greek (no dead keys)") },
    { "gr", "polytonic", I18N_NOOP("Greek (polytonic)") },
    { "gr", "simple", I18N_NOOP("Greek (simple)") },
    { "in", "guj", I18N_NOOP("Gujarati") },
    { "cn", "altgr-pinyin", I18N_NOOP("Hanyu Pinyin Letters (with AltGr dead keys)") },
    { "gh", "hausa", I18N_NOOP("Hausa (Ghana)") },
    { "ng", "hausa", I18N_NOOP("Hausa (Nigeria)") },
    { "us", "haw", I18N_NOOP("Hawaiian") },
    { "il", "biblical", I18N_NOOP("Hebrew (Biblical, Tiro)") },
    { "il", "lyx", I18N_NOOP("Hebrew (lyx)") },
    { "il", "phonetic", I18N_NOOP("Hebrew (phonetic)") },
    { "in", "bolnagri", I18N_NOOP("Hindi (Bolnagri)") },
    { "in", "hin-kagapa", I18N_NOOP("Hindi (KaGaPa, phonetic)") },
    { "in", "hin-wx", I18N_NOOP("Hindi (Wx)") },
    { "hu", "nodeadkeys", I18N_NOOP("Hungarian (no dead keys)") },
    { "hu", "qwerty", I18N_NOOP("Hungarian (QWERTY)") },
    { "hu", "101_qwerty_comma_dead", I18N_NOOP("Hungarian (QWERTY, 101-key, comma, dead keys)") },
    { "hu", "101_qwerty_comma_nodead", I18N_NOOP("Hungarian (QWERTY, 101-key, comma, no dead keys)") },
    { "hu", "101_qwerty_dot_dead", I18N_NOOP("Hungarian (QWERTY, 101-key, dot, dead keys)") },
    { "hu", "101_qwerty_dot_nodead", I18N_NOOP("Hungarian (QWERTY, 101-key, dot, no dead keys)") },
    { "hu", "102_qwerty_comma_dead", I18N_NOOP("Hungarian (QWERTY, 102-key, comma, dead keys)") },
    { "hu", "102_qwerty_comma_nodead", I18N_NOOP("Hungarian (QWERTY, 102-key, comma, no dead keys)") },
    { "hu", "102_qwerty_dot_dead", I18N_NOOP("Hungarian (QWERTY, 102-key, dot, dead keys)") },
    { "hu", "102_qwerty_dot_nodead", I18N_NOOP("Hungarian (QWERTY, 102-key, dot, no dead keys)") },
    { "hu", "101_qwertz_comma_dead", I18N_NOOP("Hungarian (QWERTZ, 101-key, comma, dead keys)") },
    { "hu", "101_qwertz_comma_nodead", I18N_NOOP("Hungarian (QWERTZ, 101-key, comma, no dead keys)") },
    { "hu", "101_qwertz_dot_dead", I18N_NOOP("Hungarian (QWERTZ, 101-key, dot, dead keys)") },
    { "hu", "101_qwertz_dot_nodead", I18N_NOOP("Hungarian (QWERTZ, 101-key, dot, no dead keys)") },
    { "hu", "102_qwertz_comma_dead", I18N_NOOP("Hungarian (QWERTZ, 102-key, comma, dead keys)") },
    { "hu", "102_qwertz_comma_nodead", I18N_NOOP("Hungarian (QWERTZ, 102-key, comma, no dead keys)") },
    { "hu", "102_qwertz_dot_dead", I18N_NOOP("Hungarian (QWERTZ, 102-key, dot, dead keys)") },
    { "hu", "102_qwertz_dot_nodead", I18N_NOOP("Hungarian (QWERTZ, 102-key, dot, no dead keys)") },
    { "hu", "standard", I18N_NOOP("Hungarian (standard)") },
    { "is", "dvorak", I18N_NOOP("Icelandic (Dvorak)") },
    { "is", "mac", I18N_NOOP("Icelandic (Macintosh)") },
    { "is", "mac_legacy", I18N_NOOP("Icelandic (Macintosh, legacy)") },
    { "ng", "igbo", I18N_NOOP("Igbo") },
    { "in", "iipa", I18N_NOOP("Indic IPA") },
    { "id", "phoneticx", I18N_NOOP("Indonesian (Arab Pegon, extended phonetic)") },
    { "id", "phonetic", I18N_NOOP("Indonesian (Arab Pegon, phonetic)") },
    { "ca", "ike", I18N_NOOP("Inuktitut") },
    { "ie", "UnicodeExpert", I18N_NOOP("Irish (UnicodeExpert)") },
    { "it", "ibm", I18N_NOOP("Italian (IBM 142)") },
    { "it", "intl", I18N_NOOP("Italian (intl., with dead keys)") },
    { "it", "mac", I18N_NOOP("Italian (Macintosh)") },
    { "it", "nodeadkeys", I18N_NOOP("Italian (no dead keys)") },
    { "it", "us", I18N_NOOP("Italian (US)") },
    { "it", "winkeys", I18N_NOOP("Italian (Windows)") },
    { "jp", "dvorak", I18N_NOOP("Japanese (Dvorak)") },
    { "jp", "kana86", I18N_NOOP("Japanese (Kana 86)") },
    { "jp", "kana", I18N_NOOP("Japanese (Kana)") },
    { "jp", "mac", I18N_NOOP("Japanese (Macintosh)") },
    { "jp", "OADG109A", I18N_NOOP("Japanese (OADG 109A)") },
    { "dz", "azerty-deadkeys", I18N_NOOP("Kabyle (AZERTY, with dead keys)") },
    { "dz", "qwerty-gb-deadkeys", I18N_NOOP("Kabyle (QWERTY, UK, with dead keys)") },
    { "dz", "qwerty-us-deadkeys", I18N_NOOP("Kabyle (QWERTY, US, with dead keys)") },
    { "ru", "xal", I18N_NOOP("Kalmyk") },
    { "in", "kan-kagapa", I18N_NOOP("Kannada (KaGaPa, phonetic)") },
    { "in", "kan", I18N_NOOP("Kannada") },
    { "pl", "csb", I18N_NOOP("Kashubian") },
    { "kz", "ext", I18N_NOOP("Kazakh (extended)") },
    { "kz", "latin", I18N_NOOP("Kazakh (Latin)") },
    { "kz", "kazrus", I18N_NOOP("Kazakh (with Russian)") },
    { "ke", "kik", I18N_NOOP("Kikuyu") },
    { "ru", "kom", I18N_NOOP("Komi") },
    { "kr", "kr104", I18N_NOOP("Korean (101/104-key compatible)") },
    { "ir", "ku_ara", I18N_NOOP("Kurdish (Iran, Arabic-Latin)") },
    { "ir", "ku_f", I18N_NOOP("Kurdish (Iran, F)") },
    { "ir", "ku_alt", I18N_NOOP("Kurdish (Iran, Latin Alt-Q)") },
    { "ir", "ku", I18N_NOOP("Kurdish (Iran, Latin Q)") },
    { "iq", "ku_ara", I18N_NOOP("Kurdish (Iraq, Arabic-Latin)") },
    { "iq", "ku_f", I18N_NOOP("Kurdish (Iraq, F)") },
    { "iq", "ku_alt", I18N_NOOP("Kurdish (Iraq, Latin Alt-Q)") },
    { "iq", "ku", I18N_NOOP("Kurdish (Iraq, Latin Q)") },
    { "sy", "ku_f", I18N_NOOP("Kurdish (Syria, F)") },
    { "sy", "ku_alt", I18N_NOOP("Kurdish (Syria, Latin Alt-Q)") },
    { "sy", "ku", I18N_NOOP("Kurdish (Syria, Latin Q)") },
    { "tr", "ku_f", I18N_NOOP("Kurdish (Turkey, F)") },
    { "tr", "ku_alt", I18N_NOOP("Kurdish (Turkey, Latin Alt-Q)") },
    { "tr", "ku", I18N_NOOP("Kurdish (Turkey, Latin Q)") },
    { "kg", "phonetic", I18N_NOOP("Kyrgyz (phonetic)") },
    { "la", "stea", I18N_NOOP("Lao (STEA)") },
    { "lv", "adapted", I18N_NOOP("Latvian (adapted)") },
    { "lv", "apostrophe", I18N_NOOP("Latvian (apostrophe)") },
    { "lv", "ergonomic", I18N_NOOP("Latvian (ergonomic, ŪGJRMV)") },
    { "lv", "fkey", I18N_NOOP("Latvian (F)") },
    { "lv", "modern", I18N_NOOP("Latvian (modern)") },
    { "lv", "tilde", I18N_NOOP("Latvian (tilde)") },
    { "lt", "ibm", I18N_NOOP("Lithuanian (IBM LST 1205-92)") },
    { "lt", "lekp", I18N_NOOP("Lithuanian (LEKP)") },
    { "lt", "lekpa", I18N_NOOP("Lithuanian (LEKPa)") },
    { "lt", "ratise", I18N_NOOP("Lithuanian (Ratise)") },
    { "lt", "std", I18N_NOOP("Lithuanian (standard)") },
    { "lt", "us", I18N_NOOP("Lithuanian (US)") },
    { "de", "dsb_qwertz", I18N_NOOP("Lower Sorbian (QWERTZ)") },
    { "de", "dsb", I18N_NOOP("Lower Sorbian") },
    { "mk", "nodeadkeys", I18N_NOOP("Macedonian (no dead keys)") },
    { "my", "phonetic", I18N_NOOP("Malay (Jawi, phonetic)") },
    { "in", "mal_enhanced", I18N_NOOP("Malayalam (enhanced InScript, with rupee)") },
    { "in", "mal_lalitha", I18N_NOOP("Malayalam (Lalitha)") },
    { "in", "mal", I18N_NOOP("Malayalam") },
    { "mt", "alt-gb", I18N_NOOP("Maltese (UK, with AltGr overrides)") },
    { "mt", "us", I18N_NOOP("Maltese (US)") },
    { "mt", "alt-us", I18N_NOOP("Maltese (US, with AltGr overrides)") },
    { "in", "eeyek", I18N_NOOP("Manipuri (Eeyek)") },
    { "in", "marathi", I18N_NOOP("Marathi (enhanced InScript)") },
    { "in", "mar-kagapa", I18N_NOOP("Marathi (KaGaPa, phonetic)") },
    { "ru", "chm", I18N_NOOP("Mari") },
    { "cm", "mmuock", I18N_NOOP("Mmuock") },
    { "md", "gag", I18N_NOOP("Moldavian (Gagauz)") },
    { "mm", "mnw-a1", I18N_NOOP("Mon (A1)") },
    { "mm", "mnw", I18N_NOOP("Mon") },
    { "cn", "mon_trad", I18N_NOOP("Mongolian (Bichig)") },
    { "cn", "mon_trad_galik", I18N_NOOP("Mongolian (Galik)") },
    { "cn", "mon_manchu_galik", I18N_NOOP("Mongolian (Manchu Galik)") },
    { "cn", "mon_trad_manchu", I18N_NOOP("Mongolian (Manchu)") },
    { "cn", "mon_todo_galik", I18N_NOOP("Mongolian (Todo Galik)") },
    { "cn", "mon_trad_todo", I18N_NOOP("Mongolian (Todo)") },
    { "cn", "mon_trad_xibe", I18N_NOOP("Mongolian (Xibe)") },
    { "me", "cyrillic", I18N_NOOP("Montenegrin (Cyrillic)") },
    { "me", "cyrillicalternatequotes", I18N_NOOP("Montenegrin (Cyrillic, with guillemets)") },
    { "me", "cyrillicyz", I18N_NOOP("Montenegrin (Cyrillic, ZE and ZHE swapped)") },
    { "me", "latinyz", I18N_NOOP("Montenegrin (Latin, QWERTY)") },
    { "me", "latinunicode", I18N_NOOP("Montenegrin (Latin, Unicode)") },
    { "me", "latinunicodeyz", I18N_NOOP("Montenegrin (Latin, Unicode, QWERTY)") },
    { "me", "latinalternatequotes", I18N_NOOP("Montenegrin (Latin, with guillemets)") },
    { "fi", "smi", I18N_NOOP("Northern Saami (Finland)") },
    { "no", "smi", I18N_NOOP("Northern Saami (Norway)") },
    { "no", "smi_nodeadkeys", I18N_NOOP("Northern Saami (Norway, no dead keys)") },
    { "se", "smi", I18N_NOOP("Northern Saami (Sweden)") },
    { "no", "colemak", I18N_NOOP("Norwegian (Colemak)") },
    { "no", "dvorak", I18N_NOOP("Norwegian (Dvorak)") },
    { "no", "mac", I18N_NOOP("Norwegian (Macintosh)") },
    { "no", "mac_nodeadkeys", I18N_NOOP("Norwegian (Macintosh, no dead keys)") },
    { "no", "nodeadkeys", I18N_NOOP("Norwegian (no dead keys)") },
    { "no", "winkeys", I18N_NOOP("Norwegian (Windows)") },
    { "fr", "oci", I18N_NOOP("Occitan") },
    { "ie", "ogam_is434", I18N_NOOP("Ogham (IS434)") },
    { "ie", "ogam", I18N_NOOP("Ogham") },
    { "in", "olck", I18N_NOOP("Ol Chiki") },
    { "tr", "otkf", I18N_NOOP("Old Turkic (F)") },
    { "tr", "otk", I18N_NOOP("Old Turkic") },
    { "in", "ori-bolnagri", I18N_NOOP("Oriya (Bolnagri)") },
    { "in", "ori-wx", I18N_NOOP("Oriya (Wx)") },
    { "in", "ori", I18N_NOOP("Oriya") },
    { "ge", "os", I18N_NOOP("Ossetian (Georgia)") },
    { "ru", "os_legacy", I18N_NOOP("Ossetian (legacy)") },
    { "ru", "os_winkeys", I18N_NOOP("Ossetian (Windows)") },
    { "tr", "otf", I18N_NOOP("Ottoman (F)") },
    { "tr", "ot", I18N_NOOP("Ottoman (Q)") },
    { "rs", "rue", I18N_NOOP("Pannonian Rusyn") },
    { "af", "ps-olpc", I18N_NOOP("Pashto (Afghanistan, OLPC)") },
    { "af", "ps", I18N_NOOP("Pashto") },
    { "ir", "pes_keypad", I18N_NOOP("Persian (with Persian keypad)") },
    { "gb", "pl", I18N_NOOP("Polish (British keyboard)") },
    { "pl", "dvorak", I18N_NOOP("Polish (Dvorak)") },
    { "pl", "dvorak_altquotes", I18N_NOOP("Polish (Dvorak, with Polish quotes on key 1)") },
    { "pl", "dvorak_quotes", I18N_NOOP("Polish (Dvorak, with Polish quotes on quotemark key)") },
    { "pl", "legacy", I18N_NOOP("Polish (legacy)") },
    { "pl", "dvp", I18N_NOOP("Polish (programmer Dvorak)") },
    { "pl", "qwertz", I18N_NOOP("Polish (QWERTZ)") },
    { "br", "dvorak", I18N_NOOP("Portuguese (Brazil, Dvorak)") },
    { "br", "thinkpad", I18N_NOOP("Portuguese (Brazil, IBM/Lenovo ThinkPad)") },
    { "br", "nativo-us", I18N_NOOP("Portuguese (Brazil, Nativo for US keyboards)") },
    { "br", "nativo", I18N_NOOP("Portuguese (Brazil, Nativo)") },
    { "br", "nodeadkeys", I18N_NOOP("Portuguese (Brazil, no dead keys)") },
    { "pt", "mac", I18N_NOOP("Portuguese (Macintosh)") },
    { "pt", "mac_nodeadkeys", I18N_NOOP("Portuguese (Macintosh, no dead keys)") },
    { "pt", "nativo-us", I18N_NOOP("Portuguese (Nativo for US keyboards)") },
    { "pt", "nativo", I18N_NOOP("Portuguese (Nativo)") },
    { "pt", "nodeadkeys", I18N_NOOP("Portuguese (no dead keys)") },
    { "in", "jhelum", I18N_NOOP("Punjabi (Gurmukhi Jhelum)") },
    { "in", "guru", I18N_NOOP("Punjabi (Gurmukhi)") },
    { "de", "ro", I18N_NOOP("Romanian (Germany)") },
    { "de", "ro_nodeadkeys", I18N_NOOP("Romanian (Germany, no dead keys)") },
    { "ro", "std", I18N_NOOP("Romanian (standard)") },
    { "ro", "winkeys", I18N_NOOP("Romanian (Windows)") },
    { "by", "ru", I18N_NOOP("Russian (Belarus)") },
    { "cz", "rus", I18N_NOOP("Russian (Czech, phonetic)") },
    { "ru", "dos", I18N_NOOP("Russian (DOS)") },
    { "ge", "ru", I18N_NOOP("Russian (Georgia)") },
    { "de", "ru", I18N_NOOP("Russian (Germany, phonetic)") },
    { "kz", "ruskaz", I18N_NOOP("Russian (Kazakhstan, with Kazakh)") },
    { "ru", "legacy", I18N_NOOP("Russian (legacy)") },
    { "ru", "mac", I18N_NOOP("Russian (Macintosh)") },
    { "ru", "phonetic", I18N_NOOP("Russian (phonetic)") },
    { "ru", "phonetic_azerty", I18N_NOOP("Russian (phonetic, AZERTY)") },
    { "ru", "phonetic_dvorak", I18N_NOOP("Russian (phonetic, Dvorak)") },
    { "ru", "phonetic_fr", I18N_NOOP("Russian (phonetic, French)") },
    { "ru", "phonetic_winkeys", I18N_NOOP("Russian (phonetic, Windows)") },
    { "ru", "phonetic_YAZHERTY", I18N_NOOP("Russian (phonetic, YAZHERTY)") },
    { "pl", "ru_phonetic_dvorak", I18N_NOOP("Russian (Poland, phonetic Dvorak)") },
    { "se", "rus", I18N_NOOP("Russian (Sweden, phonetic)") },
    { "se", "rus_nodeadkeys", I18N_NOOP("Russian (Sweden, phonetic, no dead keys)") },
    { "ru", "typewriter", I18N_NOOP("Russian (typewriter)") },
    { "ru", "typewriter-legacy", I18N_NOOP("Russian (typewriter, legacy)") },
    { "ua", "rstu_ru", I18N_NOOP("Russian (Ukraine, standard RSTU)") },
    { "us", "rus", I18N_NOOP("Russian (US, phonetic)") },
    { "tw", "saisiyat", I18N_NOOP("Saisiyat (Taiwan)") },
    { "lt", "sgs", I18N_NOOP("Samogitian") },
    { "in", "san-kagapa", I18N_NOOP("Sanskrit (KaGaPa, phonetic)") },
    { "gb", "gla", I18N_NOOP("Scottish Gaelic") },
    { "rs", "alternatequotes", I18N_NOOP("Serbian (Cyrillic, with guillemets)") },
    { "rs", "yz", I18N_NOOP("Serbian (Cyrillic, ZE and ZHE swapped)") },
    { "rs", "latin", I18N_NOOP("Serbian (Latin)") },
    { "rs", "latinyz", I18N_NOOP("Serbian (Latin, QWERTY)") },
    { "rs", "latinunicode", I18N_NOOP("Serbian (Latin, Unicode)") },
    { "rs", "latinunicodeyz", I18N_NOOP("Serbian (Latin, Unicode, QWERTY)") },
    { "rs", "latinalternatequotes", I18N_NOOP("Serbian (Latin, with guillemets)") },
    { "ru", "srp", I18N_NOOP("Serbian (Russia)") },
    { "us", "hbs", I18N_NOOP("Serbo-Croatian (US)") },
    { "mm", "zgt", I18N_NOOP("Shan (Zawgyi Tai)") },
    { "mm", "shn", I18N_NOOP("Shan") },
    { "it", "scn", I18N_NOOP("Sicilian") },
    { "pl", "szl", I18N_NOOP("Silesian") },
    { "pk", "snd", I18N_NOOP("Sindhi") },
    { "lk", "us", I18N_NOOP("Sinhala (US)") },
    { "sk", "bksl", I18N_NOOP("Slovak (extended backslash)") },
    { "sk", "qwerty", I18N_NOOP("Slovak (QWERTY)") },
    { "sk", "qwerty_bksl", I18N_NOOP("Slovak (QWERTY, extended backslash)") },
    { "si", "us", I18N_NOOP("Slovenian (US)") },
    { "si", "alternatequotes", I18N_NOOP("Slovenian (with guillemets)") },
    { "es", "deadtilde", I18N_NOOP("Spanish (dead tilde)") },
    { "es", "dvorak", I18N_NOOP("Spanish (Dvorak)") },
    { "latam", "colemak-gaming", I18N_NOOP("Spanish (Latin American, Colemak for gaming)") },
    { "latam", "colemak", I18N_NOOP("Spanish (Latin American, Colemak)") },
    { "latam", "deadtilde", I18N_NOOP("Spanish (Latin American, dead tilde)") },
    { "latam", "dvorak", I18N_NOOP("Spanish (Latin American, Dvorak)") },
    { "latam", "nodeadkeys", I18N_NOOP("Spanish (Latin American, no dead keys)") },
    { "es", "mac", I18N_NOOP("Spanish (Macintosh)") },
    { "es", "nodeadkeys", I18N_NOOP("Spanish (no dead keys)") },
    { "es", "winkeys", I18N_NOOP("Spanish (Windows)") },
    { "se", "dvorak", I18N_NOOP("Swedish (Dvorak)") },
    { "se", "us_dvorak", I18N_NOOP("Swedish (Dvorak, intl.)") },
    { "se", "mac", I18N_NOOP("Swedish (Macintosh)") },
    { "se", "nodeadkeys", I18N_NOOP("Swedish (no dead keys)") },
    { "se", "svdvorak", I18N_NOOP("Swedish (Svdvorak)") },
    { "se", "us", I18N_NOOP("Swedish (US)") },
    { "se", "swl", I18N_NOOP("Swedish Sign Language") },
    { "sy", "syc_phonetic", I18N_NOOP("Syriac (phonetic)") },
    { "sy", "syc", I18N_NOOP("Syriac") },
    { "tw", "indigenous", I18N_NOOP("Taiwanese (indigenous)") },
    { "tj", "legacy", I18N_NOOP("Tajik (legacy)") },
    { "in", "tam", I18N_NOOP("Tamil (InScript)") },
    { "lk", "tam_unicode", I18N_NOOP("Tamil (Sri Lanka, TamilNet '99)") },
    { "lk", "tam_TAB", I18N_NOOP("Tamil (Sri Lanka, TamilNet '99, TAB encoding)") },
    { "in", "tam_tamilnet_with_tam_nums", I18N_NOOP("Tamil (TamilNet '99 with Tamil numerals)") },
    { "in", "tam_tamilnet", I18N_NOOP("Tamil (TamilNet '99)") },
    { "in", "tam_tamilnet_TAB", I18N_NOOP("Tamil (TamilNet '99, TAB encoding)") },
    { "in", "tam_tamilnet_TSCII", I18N_NOOP("Tamil (TamilNet '99, TSCII encoding)") },
    { "ma", "rif", I18N_NOOP("Tarifit") },
    { "ru", "tt", I18N_NOOP("Tatar") },
    { "in", "tel-kagapa", I18N_NOOP("Telugu (KaGaPa, phonetic)") },
    { "in", "tel-sarala", I18N_NOOP("Telugu (Sarala)") },
    { "in", "tel", I18N_NOOP("Telugu") },
    { "th", "pat", I18N_NOOP("Thai (Pattachote)") },
    { "th", "tis", I18N_NOOP("Thai (TIS-820.2538)") },
    { "cn", "tib_asciinum", I18N_NOOP("Tibetan (with ASCII numerals)") },
    { "cn", "tib", I18N_NOOP("Tibetan") },
    { "tr", "alt", I18N_NOOP("Turkish (Alt-Q)") },
    { "tr", "f", I18N_NOOP("Turkish (F)") },
    { "de", "tr", I18N_NOOP("Turkish (Germany)") },
    { "tr", "intl", I18N_NOOP("Turkish (intl., with dead keys)") },
    { "tm", "alt", I18N_NOOP("Turkmen (Alt-Q)") },
    { "ru", "udm", I18N_NOOP("Udmurt") },
    { "ua", "homophonic", I18N_NOOP("Ukrainian (homophonic)") },
    { "ua", "legacy", I18N_NOOP("Ukrainian (legacy)") },
    { "ua", "macOS", I18N_NOOP("Ukrainian (macOS)") },
    { "ua", "phonetic", I18N_NOOP("Ukrainian (phonetic)") },
    { "ua", "rstu", I18N_NOOP("Ukrainian (standard RSTU)") },
    { "ua", "typewriter", I18N_NOOP("Ukrainian (typewriter)") },
    { "ua", "winkeys", I18N_NOOP("Ukrainian (Windows)") },
    { "in", "urd-phonetic3", I18N_NOOP("Urdu (alt. phonetic)") },
    { "pk", "urd-crulp", I18N_NOOP("Urdu (Pakistan, CRULP)") },
    { "pk", "urd-nla", I18N_NOOP("Urdu (Pakistan, NLA)") },
    { "in", "urd-phonetic", I18N_NOOP("Urdu (phonetic)") },
    { "in", "urd-winkeys", I18N_NOOP("Urdu (Windows)") },
    { "cn", "ug", I18N_NOOP("Uyghur") },
    { "af", "uz", I18N_NOOP("Uzbek (Afghanistan)") },
    { "af", "uz-olpc", I18N_NOOP("Uzbek (Afghanistan, OLPC)") },
    { "uz", "latin", I18N_NOOP("Uzbek (Latin)") },
    { "vn", "fr", I18N_NOOP("Vietnamese (French)") },
    { "vn", "us", I18N_NOOP("Vietnamese (US)") },
    { "ru", "sah", I18N_NOOP("Yakut") },
    { "ng", "yoruba", I18N_NOOP("Yoruba") }
};
static const qint16 variantDescriptionTblSize = sizeof(variantDescriptionTbl) / sizeof(variantDescriptionData);

static const struct optionDescriptionData {
    const char* name;
    const char* description;
} optionDescriptionTbl[] = {
    { "compose:caps-altgr", I18N_NOOP("3rd level of Caps Lock") },
    { "compose:lctrl-altgr", I18N_NOOP("3rd level of Left Ctrl") },
    { "compose:lwin-altgr", I18N_NOOP("3rd level of Left Win") },
    { "compose:menu-altgr", I18N_NOOP("3rd level of Menu") },
    { "compose:rctrl-altgr", I18N_NOOP("3rd level of Right Ctrl") },
    { "compose:rwin-altgr", I18N_NOOP("3rd level of Right Win") },
    { "compose:102-altgr", I18N_NOOP("3rd level of the \"< >\" key") },
    { "altwin:menu", I18N_NOOP("Add the standard behavior to Menu key") },
    { "grab:break_actions", I18N_NOOP("Allow breaking grabs with keyboard actions (warning: security risk)") },
    { "grab:debug", I18N_NOOP("Allow grab and window tree logging") },
    { "altwin:meta_alt", I18N_NOOP("Alt and Meta are on Alt") },
    { "altwin", I18N_NOOP("Alt and Win behavior") },
    { "altwin:alt_super_win", I18N_NOOP("Alt is mapped to Right Win, Super to Menu") },
    { "altwin:alt_win", I18N_NOOP("Alt is mapped to Win and the usual Alt") },
    { "altwin:swap_alt_win", I18N_NOOP("Alt is swapped with Win") },
    { "grp:alt_caps_toggle", I18N_NOOP("Alt+Caps Lock") },
    { "grp:ctrl_alt_toggle", I18N_NOOP("Alt+Ctrl") },
    { "grp:alt_shift_toggle", I18N_NOOP("Alt+Shift") },
    { "grp:alt_space_toggle", I18N_NOOP("Alt+Space") },
    { "lv3:alt_switch", I18N_NOOP("Any Alt") },
    { "grp:win_switch", I18N_NOOP("Any Win (while pressed)") },
    { "lv3:win_switch", I18N_NOOP("Any Win") },
    { "apple:alupckeys", I18N_NOOP("Apple Aluminium emulates Pause, PrtSc, Scroll Lock") },
    { "ctrl:aa_ctrl", I18N_NOOP("At the bottom left") },
    { "esperanto:colemak", I18N_NOOP("At the corresponding key in a Colemak layout") },
    { "esperanto:dvorak", I18N_NOOP("At the corresponding key in a Dvorak layout") },
    { "esperanto:qwerty", I18N_NOOP("At the corresponding key in a QWERTY layout") },
    { "lv3:bksl_switch", I18N_NOOP("Backslash") },
    { "lv3:bksl_switch_latch", I18N_NOOP("Backslash; acts as onetime lock when pressed together with another 3rd level chooser") },
    { "grp:alts_toggle", I18N_NOOP("Both Alt together") },
    { "grp:ctrls_toggle", I18N_NOOP("Both Ctrl together") },
    { "shift:both_capslock", I18N_NOOP("Both Shift together enable Caps Lock") },
    { "shift:both_capslock_cancel", I18N_NOOP("Both Shift together enable Caps Lock; one Shift key disables it") },
    { "shift:both_shiftlock", I18N_NOOP("Both Shift together enable Shift Lock") },
    { "grp:shifts_toggle", I18N_NOOP("Both Shift together") },
    { "grp:caps_switch", I18N_NOOP("Caps Lock (while pressed), Alt+Caps Lock for the original Caps Lock action") },
    { "caps:shift_nocancel", I18N_NOOP("Caps Lock acts as Shift with locking; Shift does not affect Caps Lock") },
    { "caps:shift", I18N_NOOP("Caps Lock acts as Shift with locking; Shift \"pauses\" Caps Lock") },
    { "ctrl:nocaps", I18N_NOOP("Caps Lock as Ctrl") },
    { "ctrl:swapcaps_hyper", I18N_NOOP("Caps Lock as Ctrl, Ctrl as Hyper") },
    { "caps", I18N_NOOP("Caps Lock behavior") },
    { "caps:none", I18N_NOOP("Caps Lock is disabled") },
    { "grp:shift_caps_switch", I18N_NOOP("Caps Lock to first layout; Shift+Caps Lock to last layout") },
    { "caps:capslock", I18N_NOOP("Caps Lock toggles normal capitalization of alphabetic characters") },
    { "caps:shiftlock", I18N_NOOP("Caps Lock toggles Shift Lock (affects all keys)") },
    { "caps:internal_nocancel", I18N_NOOP("Caps Lock uses internal capitalization; Shift does not affect Caps Lock") },
    { "caps:internal", I18N_NOOP("Caps Lock uses internal capitalization; Shift \"pauses\" Caps Lock") },
    { "grp:caps_toggle", I18N_NOOP("Caps Lock") },
    { "lv3:caps_switch_latch", I18N_NOOP("Caps Lock; acts as onetime lock when pressed together with another 3rd-level chooser") },
    { "compat", I18N_NOOP("Compatibility options") },
    { "mod_led:compose", I18N_NOOP("Compose") },
    { "altwin:ctrl_alt_win", I18N_NOOP("Ctrl is mapped to Alt, Alt to Win") },
    { "altwin:ctrl_rwin", I18N_NOOP("Ctrl is mapped to Right Win and the usual Ctrl") },
    { "altwin:ctrl_win", I18N_NOOP("Ctrl is mapped to Win and the usual Ctrl") },
    { "ctrl", I18N_NOOP("Ctrl position") },
    { "terminate:ctrl_alt_bksp", I18N_NOOP("Ctrl+Alt+Backspace") },
    { "grp:ctrl_shift_toggle", I18N_NOOP("Ctrl+Shift") },
    { "currencysign", I18N_NOOP("Currency signs") },
    { "numpad:pc", I18N_NOOP("Default numeric keypad keys") },
    { "misc:apl", I18N_NOOP("Enable APL overlay characters") },
    { "misc:typo", I18N_NOOP("Enable extra typographic characters") },
    { "lv3:enter_switch", I18N_NOOP("Enter on keypad") },
    { "esperanto", I18N_NOOP("Esperanto letters with superscripts") },
    { "eurosign:2", I18N_NOOP("Euro on 2") },
    { "eurosign:4", I18N_NOOP("Euro on 4") },
    { "eurosign:5", I18N_NOOP("Euro on 5") },
    { "eurosign:e", I18N_NOOP("Euro on E") },
    { "kpdl:kposs", I18N_NOOP("Four-level key with abstract separators") },
    { "kpdl:commaoss", I18N_NOOP("Four-level key with comma") },
    { "kpdl:dotoss", I18N_NOOP("Four-level key with dot") },
    { "kpdl:dotoss_latin9", I18N_NOOP("Four-level key with dot, Latin-9 only") },
    { "kpdl:momayyezoss", I18N_NOOP("Four-level key with momayyez") },
    { "keypad:hex", I18N_NOOP("Hexadecimal") },
    { "altwin:hyper_win", I18N_NOOP("Hyper is mapped to Win") },
    { "japan", I18N_NOOP("Japanese keyboard options") },
    { "japan:kana_lock", I18N_NOOP("Kana Lock key is locking") },
    { "Compose", I18N_NOOP("key          Position of Compose key") },
    { "terminate", I18N_NOOP("Key sequence to kill the X server") },
    { "lv5", I18N_NOOP("Key to choose 5th level") },
    { "lv2", I18N_NOOP("Key to choose the 2nd level") },
    { "lv3", I18N_NOOP("Key to choose the 3rd level") },
    { "korean", I18N_NOOP("Korean Hangul/Hanja keys") },
    { "keypad", I18N_NOOP("Layout of numeric keypad") },
    { "grp:lswitch", I18N_NOOP("Left Alt (while pressed)") },
    { "ctrl:swap_lalt_lctl_lwin", I18N_NOOP("Left Alt as Ctrl, Left Ctrl as Win, Left Win as Left Alt") },
    { "altwin:swap_lalt_lwin", I18N_NOOP("Left Alt is swapped with Left Win") },
    { "grp:lalt_toggle", I18N_NOOP("Left Alt") },
    { "grp:lalt_lshift_toggle", I18N_NOOP("Left Alt+Left Shift") },
    { "ctrl:lctrl_meta", I18N_NOOP("Left Ctrl as Meta") },
    { "grp:lctrl_rctrl_switch", I18N_NOOP("Left Ctrl to first layout; Right Ctrl to last layout") },
    { "grp:lctrl_toggle", I18N_NOOP("Left Ctrl") },
    { "grp:lctrl_lshift_toggle", I18N_NOOP("Left Ctrl+Left Shift") },
    { "grp:lctrl_lwin_rctrl_menu", I18N_NOOP("Left Ctrl+Left Win to first layout; Right Ctrl+Menu to second layout") },
    { "grp:lctrl_lwin_toggle", I18N_NOOP("Left Ctrl+Left Win") },
    { "grp:lshift_toggle", I18N_NOOP("Left Shift") },
    { "grp:lwin_switch", I18N_NOOP("Left Win (while pressed)") },
    { "lv5:lwin_switch_lock", I18N_NOOP("Left Win chooses 5th level and acts as a one-time lock if pressed with another 5th level chooser") },
    { "grp:win_menu_switch", I18N_NOOP("Left Win to first layout; Right Win/Menu to last layout") },
    { "grp:lwin_toggle", I18N_NOOP("Left Win") },
    { "kpdl:comma", I18N_NOOP("Legacy key with comma") },
    { "kpdl:dot", I18N_NOOP("Legacy key with dot") },
    { "keypad:legacy_wang", I18N_NOOP("Legacy Wang 724") },
    { "keypad:legacy", I18N_NOOP("Legacy") },
    { "caps:backspace", I18N_NOOP("Make Caps Lock an additional Backspace") },
    { "caps:ctrl_modifier", I18N_NOOP("Make Caps Lock an additional Ctrl") },
    { "caps:escape", I18N_NOOP("Make Caps Lock an additional Esc") },
    { "caps:escape_shifted_capslock", I18N_NOOP("Make Caps Lock an additional Esc, but Shift + Caps Lock is the regular Caps Lock") },
    { "caps:hyper", I18N_NOOP("Make Caps Lock an additional Hyper") },
    { "caps:menu", I18N_NOOP("Make Caps Lock an additional Menu key") },
    { "caps:numlock", I18N_NOOP("Make Caps Lock an additional Num Lock") },
    { "caps:super", I18N_NOOP("Make Caps Lock an additional Super") },
    { "korean:ralt_hangul", I18N_NOOP("Make right Alt a Hangul key") },
    { "korean:ralt_hanja", I18N_NOOP("Make right Alt a Hanja key") },
    { "korean:rctrl_hangul", I18N_NOOP("Make right Ctrl a Hangul key") },
    { "korean:rctrl_hanja", I18N_NOOP("Make right Ctrl a Hanja key") },
    { "japan:hztg_escape", I18N_NOOP("Make Zenkaku Hankaku an additional Esc") },
    { "grp:menu_switch", I18N_NOOP("Menu (while pressed), Shift+Menu for Menu") },
    { "ctrl:menu_rctrl", I18N_NOOP("Menu as Right Ctrl") },
    { "lv5:menu_switch", I18N_NOOP("Menu chooses 5th level") },
    { "altwin:menu_win", I18N_NOOP("Menu is mapped to Win") },
    { "grp:menu_toggle", I18N_NOOP("Menu") },
    { "altwin:left_meta_win", I18N_NOOP("Meta is mapped to Left Win") },
    { "altwin:meta_win", I18N_NOOP("Meta is mapped to Win") },
    { "japan:nicola_f_bs", I18N_NOOP("NICOLA-F style Backspace") },
    { "nbsp:level2", I18N_NOOP("Non-breaking space at the 2nd level") },
    { "nbsp:level3", I18N_NOOP("Non-breaking space at the 3rd level") },
    { "nbsp:level3s", I18N_NOOP("Non-breaking space at the 3rd level, nothing at the 4th level") },
    { "nbsp:level3n", I18N_NOOP("Non-breaking space at the 3rd level, thin non-breaking space at the 4th level") },
    { "nbsp:level4", I18N_NOOP("Non-breaking space at the 4th level") },
    { "nbsp:level4nl", I18N_NOOP("Non-breaking space at the 4th level, thin non-breaking space at the 6th level (via Ctrl+Shift)") },
    { "nbsp:level4n", I18N_NOOP("Non-breaking space at the 4th level, thin non-breaking space at the 6th level") },
    { "nbsp", I18N_NOOP("Non-breaking space input") },
    { "numpad:microsoft", I18N_NOOP("Num Lock on: digits; Shift for arrows. Num Lock off: arrows (as in Windows)") },
    { "grp_led:num", I18N_NOOP("Num Lock") },
    { "numpad:mac", I18N_NOOP("Numeric keypad always enters digits (as in macOS)") },
    { "kpdl", I18N_NOOP("Numeric keypad Delete behavior") },
    { "solaris", I18N_NOOP("Old Solaris keycodes compatibility") },
    { "compose:paus", I18N_NOOP("Pause") },
    { "keypad:atm", I18N_NOOP("Phone and ATM style") },
    { "compose:prsc", I18N_NOOP("PrtSc") },
    { "grp:switch", I18N_NOOP("Right Alt (while pressed)") },
    { "lv5:ralt_switch_lock", I18N_NOOP("Right Alt chooses 5th level and acts as a one-time lock if pressed with another 5th level chooser") },
    { "lv5:ralt_switch", I18N_NOOP("Right Alt chooses 5th level") },
    { "lv3:ralt_alt", I18N_NOOP("Right Alt never chooses 3rd level") },
    { "grp:toggle", I18N_NOOP("Right Alt") },
    { "lv3:ralt_switch_multikey", I18N_NOOP("Right Alt; Shift+Right Alt as Compose") },
    { "grp:rctrl_switch", I18N_NOOP("Right Ctrl (while pressed)") },
    { "ctrl:rctrl_ralt", I18N_NOOP("Right Ctrl as Right Alt") },
    { "grp:rctrl_toggle", I18N_NOOP("Right Ctrl") },
    { "grp:rctrl_rshift_toggle", I18N_NOOP("Right Ctrl+Right Shift") },
    { "grp:rshift_toggle", I18N_NOOP("Right Shift") },
    { "grp:rwin_switch", I18N_NOOP("Right Win (while pressed)") },
    { "lv5:rwin_switch_lock", I18N_NOOP("Right Win chooses 5th level and acts as a one-time lock if pressed with another 5th level chooser") },
    { "grp:rwin_toggle", I18N_NOOP("Right Win") },
    { "rupeesign:4", I18N_NOOP("Rupee on 4") },
    { "grp:sclk_toggle", I18N_NOOP("Scroll Lock") },
    { "kpdl:semi", I18N_NOOP("Semicolon on third level") },
    { "keypad:pointerkeys", I18N_NOOP("Shift + Num Lock enables PointerKeys") },
    { "shift:breaks_caps", I18N_NOOP("Shift cancels Caps Lock") },
    { "numpad:shift3", I18N_NOOP("Shift does not cancel Num Lock, chooses 3rd level instead") },
    { "grp:shift_caps_toggle", I18N_NOOP("Shift+Caps Lock") },
    { "srvrkeys:none", I18N_NOOP("Special keys (Ctrl+Alt+<key>) handled in a server") },
    { "solaris:sun_compat", I18N_NOOP("Sun key compatibility") },
    { "ctrl:swapcaps", I18N_NOOP("Swap Ctrl and Caps Lock") },
    { "caps:swapescape", I18N_NOOP("Swap Esc and Caps Lock") },
    { "ctrl:swap_lalt_lctl", I18N_NOOP("Swap Left Alt with Left Ctrl") },
    { "ctrl:swap_lwin_lctl", I18N_NOOP("Swap Left Win with Left Ctrl") },
    { "ctrl:swap_rwin_rctl", I18N_NOOP("Swap Right Win with Right Ctrl") },
    { "grp", I18N_NOOP("Switching to another layout") },
    { "lv5:lsgt_switch_lock", I18N_NOOP("The \"< >\" key chooses 5th level and acts as a one-time lock if pressed with another 5th level chooser") },
    { "lv5:lsgt_switch", I18N_NOOP("The \"< >\" key chooses 5th level") },
    { "lv2:lsgt_switch", I18N_NOOP("The \"< >\" key") },
    { "lv3:lsgt_switch_latch", I18N_NOOP("The \"< >\" key; acts as onetime lock when pressed together with another 3rd level chooser") },
    { "ctrl:ac_ctrl", I18N_NOOP("To the left of \"A\"") },
    { "keypad:future", I18N_NOOP("Unicode arrows and math operators on default level") },
    { "keypad:oss", I18N_NOOP("Unicode arrows and math operators") },
    { "mod_led", I18N_NOOP("Use keyboard LED to indicate modifiers") },
    { "grp_led", I18N_NOOP("Use keyboard LED to show alternative layout") },
    { "nbsp:none", I18N_NOOP("Usual space at any level") },
    { "keypad:future_wang", I18N_NOOP("Wang 724 keypad with Unicode arrows and math operators on default level") },
    { "keypad:oss_wang", I18N_NOOP("Wang 724 keypad with Unicode arrows and math operators") },
    { "altwin:prtsc_rwin", I18N_NOOP("Win is mapped to PrtSc and the usual Win") },
    { "grp:win_space_toggle", I18N_NOOP("Win+Space") },
    { "nbsp:zwnj2", I18N_NOOP("Zero-width non-joiner at the 2nd level") },
    { "nbsp:zwnj2nb3", I18N_NOOP("Zero-width non-joiner at the 2nd level, non-breaking space at the 3rd level") },
    { "nbsp:zwnj2nb3s", I18N_NOOP("Zero-width non-joiner at the 2nd level, non-breaking space at the 3rd level, nothing at the 4th level") },
    { "nbsp:zwnj2nb3nnb4", I18N_NOOP("Zero-width non-joiner at the 2nd level, non-breaking space at the 3rd level, thin non-breaking space at the 4th level") },
    { "nbsp:zwnj2nb3zwj4", I18N_NOOP("Zero-width non-joiner at the 2nd level, non-breaking space at the 3rd level, zero-width joiner at the 4th level") },
    { "nbsp:zwnj2zwj3", I18N_NOOP("Zero-width non-joiner at the 2nd level, zero-width joiner at the 3rd level") },
    { "nbsp:zwnj2zwj3nb4", I18N_NOOP("Zero-width non-joiner at the 2nd level, zero-width joiner at the 3rd level, non-breaking space at the 4th level") },
    { "nbsp:zwnj3zwj4", I18N_NOOP("Zero-width non-joiner at the 3rd level, zero-width joiner at the 4th level") },
};
static const qint16 optionDescriptionTblSize = sizeof(optionDescriptionTbl) / sizeof(optionDescriptionData);


bool KKeyboardType::operator==(const KKeyboardType &other) const
{
    return (rule == other.rule
        && model == other.model
        && layout == other.layout
        && variant == other.variant
        && option == other.option
    );
}

class KKeyboardLayoutPrivate
{
public:
    KKeyboardLayoutPrivate(KKeyboardLayout *parent);
    ~KKeyboardLayoutPrivate();

    void _k_checkLayouts();

    QList<KKeyboardType> currentlayouts;
    QString setxkbmapexe;

private:
    KKeyboardLayout* m_parent;
    QTimer* m_timer;
};

KKeyboardLayoutPrivate::KKeyboardLayoutPrivate(KKeyboardLayout *parent)
    : m_parent(parent),
    m_timer(new QTimer(parent))
{
    m_timer->setInterval(s_layouttimeout);
    QObject::connect(
        m_timer, SIGNAL(timeout()),
        m_parent, SLOT(_k_checkLayouts())
    );
    m_timer->start();
}

KKeyboardLayoutPrivate::~KKeyboardLayoutPrivate()
{
    m_timer->stop();
}

void KKeyboardLayoutPrivate::_k_checkLayouts()
{
    const QList<KKeyboardType> layouts = m_parent->layouts();
    if (layouts != currentlayouts) {
        currentlayouts = layouts;
        kDebug() << "XKB layouts changed";
        emit m_parent->layoutChanged();
    }
}

KKeyboardLayout::KKeyboardLayout(QObject *parent)
    : QObject(parent),
    d(new KKeyboardLayoutPrivate(this))
{
}

KKeyboardLayout::~KKeyboardLayout()
{
    delete d;
}

QList<KKeyboardType> KKeyboardLayout::layouts() const
{
    QList<KKeyboardType> result;
#if defined(HAVE_XKB)
    XkbRF_VarDefsRec xkbvardefs;
    const Bool xkbresult = XkbRF_GetNamesProp(QX11Info::display(), NULL, &xkbvardefs);
    if (xkbresult != True) {
        kWarning() << "Could not get XKB layouts";
    } else {
        const QByteArray xkbmodel = QByteArray::fromRawData(xkbvardefs.model, qstrlen(xkbvardefs.model));
        const QByteArray xkblayout = QByteArray::fromRawData(xkbvardefs.layout, qstrlen(xkbvardefs.layout));
        const QByteArray xkbvariants = QByteArray::fromRawData(xkbvardefs.variant, qstrlen(xkbvardefs.variant));
        const QByteArray xkboptions = QByteArray::fromRawData(xkbvardefs.options, qstrlen(xkbvardefs.options));
        const QList<QByteArray> splitlayouts = xkblayout.split(s_xkbseparator);
        const QList<QByteArray> splitvariants = xkbvariants.split(s_xkbseparator);
        for (int i = 0; i < splitlayouts.size(); i++) {
            KKeyboardType kkeyboardtype;
            kkeyboardtype.model = xkbmodel;
            kkeyboardtype.layout = splitlayouts.at(i);
            if (i < splitvariants.size()) {
                kkeyboardtype.variant = splitvariants.at(i);
            }
            kkeyboardtype.option = xkboptions;
            result.append(kkeyboardtype);
        }
    }
#endif
    if (result.size() == 0) {
        result.append(KKeyboardLayout::defaultLayout());
    }
    return result;
}

bool KKeyboardLayout::setLayouts(const QList<KKeyboardType> &layouts)
{
    if (layouts.size() == 0) {
        kWarning() << "Empty keyboard layouts";
        return false;
    }

    QByteArray xkbrule;
    QByteArray xkbmodel;
    QByteArray xkblayouts;
    QList<QByteArray> xkbvariants;
    QList<QByteArray> xkboptions;
    foreach (const KKeyboardType &layout, layouts) {
        if (layout.layout.isEmpty()) {
            kWarning() << "Invalid keyboard layout";
            return false;
        }

        // NOTE: -rules and -model can be speicified only once
        if (!xkbrule.isEmpty() && xkbrule != layout.rule) {
            kWarning() << "Conflicting keyboard layout rules";
            return false;
        }
        xkbrule = layout.rule;
        if (!xkbmodel.isEmpty() && xkbmodel != layout.model) {
            kWarning() << "Conflicting keyboard layout models";
            return false;
        }
        xkbmodel = layout.model;

        // NOTE: layouts and variants have to be joined, variant can be empty
        if (!xkblayouts.isEmpty()) {
            xkblayouts.append(s_xkbseparator);
        }
        xkblayouts.append(layout.layout);
        xkbvariants.append(layout.variant);
        // NOTE: options can be empty, also -option argument must be used per-option
        if (!layout.option.isEmpty()) {
            xkboptions.append(layout.option);
        }
    }

    kDebug() << "Changing XKB layouts to" << xkbrule << xkbmodel << xkblayouts << xkbvariants << xkboptions;
    // setting the property is simply not enough, X11 server has to load rules and whatnot
    if (d->setxkbmapexe.isEmpty()) {
        d->setxkbmapexe = KStandardDirs::findExe("setxkbmap");
    }
    if (d->setxkbmapexe.isEmpty()) {
        kWarning() << "Could not find setxkbmap";
        return false;
    }

    // NOTE: options have to be reset first
    static const QStringList setxkbmapresetargs = QStringList()
        << QString::fromLatin1("-option");
    int setxkbmapstatus = QProcess::execute(d->setxkbmapexe, setxkbmapresetargs);
    if (setxkbmapstatus != 0) {
        kWarning() << "Could not reset XKB map" << setxkbmapstatus;
        return false;
    }

    QStringList setxkbmapargs = QStringList()
        << QString::fromLatin1("-layout")
        << QString::fromLatin1(xkblayouts.constData(), xkblayouts.size());
    if (!xkbrule.isEmpty()) {
        setxkbmapargs << QString::fromLatin1("-rules");
        setxkbmapargs << QString::fromLatin1(xkbrule.constData(), xkbrule.size());
    }
    if (!xkbmodel.isEmpty()) {
        setxkbmapargs << QString::fromLatin1("-model");
        setxkbmapargs << QString::fromLatin1(xkbmodel.constData(), xkbmodel.size());
    }
    if (!xkbvariants.isEmpty()) {
        QByteArray xkbvariantsdata;
        foreach (const QByteArray &xkbvariant, xkbvariants) {
            xkbvariantsdata.append(xkbvariant);
            xkbvariantsdata.append(s_xkbseparator);
        }
        xkbvariantsdata.chop(1);
        setxkbmapargs << QString::fromLatin1("-variant");
        setxkbmapargs << QString::fromLatin1(xkbvariantsdata.constData(), xkbvariantsdata.size());
    }
    foreach (const QByteArray &option, xkboptions) {
        setxkbmapargs << QString::fromLatin1("-option");
        setxkbmapargs << QString::fromLatin1(option.constData(), option.size());
    }
    setxkbmapstatus = QProcess::execute(d->setxkbmapexe, setxkbmapargs);
    if (setxkbmapstatus != 0) {
        kWarning() << "Could not set XKB map" << setxkbmapstatus << setxkbmapargs;
        return false;
    }
    return true;
}

KKeyboardType KKeyboardLayout::defaultLayout()
{
    // should all else fail this is a default that will work for most cases (assuming the XKB data
    // is installed that is)
    KKeyboardType defaultlayout;
    defaultlayout.rule = "base";
    defaultlayout.model = "pc105";
    defaultlayout.layout = "us";
    return defaultlayout;
}

QList<QByteArray> KKeyboardLayout::modelNames()
{
    QList<QByteArray> result;
    result.reserve(modelDescriptionTblSize);
    for (int i = 0; i < modelDescriptionTblSize; i++) {
        result.append(modelDescriptionTbl[i].name);
    }
    return result;
}

QList<QByteArray> KKeyboardLayout::layoutNames()
{
    QList<QByteArray> result;
    result.reserve(layoutDescriptionTblSize);
    for (int i = 0; i < layoutDescriptionTblSize; i++) {
        result.append(layoutDescriptionTbl[i].name);
    }
    return result;
}

QList<QByteArray> KKeyboardLayout::variantNames(const QByteArray &layout)
{
    QList<QByteArray> result;
    for (int i = 0; i < variantDescriptionTblSize; i++) {
        if (qstrcmp(layout.constData(), variantDescriptionTbl[i].name) == 0) {
            result.append(variantDescriptionTbl[i].name2);
        }
    }
    return result;
}

QList<QByteArray> KKeyboardLayout::optionNames()
{
    QList<QByteArray> result;
    result.reserve(optionDescriptionTblSize);
    for (int i = 0; i < optionDescriptionTblSize; i++) {
        result.append(optionDescriptionTbl[i].name);
    }
    return result;
}

QString KKeyboardLayout::modelDescription(const QByteArray &model)
{
    for (int i = 0; i < modelDescriptionTblSize; i++) {
        if (qstrcmp(model.constData(), modelDescriptionTbl[i].name) == 0) {
            return ki18nc("Keyboard model description", modelDescriptionTbl[i].description).toString();
        }
    }
    return QString::fromLatin1(model.constData(), model.size());
}

QString KKeyboardLayout::layoutDescription(const QByteArray &layout)
{
    for (int i = 0; i < layoutDescriptionTblSize; i++) {
        if (qstrcmp(layout.constData(), layoutDescriptionTbl[i].name) == 0) {
            return ki18nc("Keyboard layout description", layoutDescriptionTbl[i].description).toString();
        }
    }
    return QString::fromLatin1(layout.constData(), layout.size());
}

QString KKeyboardLayout::variantDescription(const QByteArray &layout, const QByteArray &variant)
{
    if (variant.isEmpty()) {
        return i18nc("Keyboard variant description", "None");
    }
    for (int i = 0; i < variantDescriptionTblSize; i++) {
        if (qstrcmp(layout.constData(), variantDescriptionTbl[i].name) == 0
            && qstrcmp(variant.constData(), variantDescriptionTbl[i].name2) == 0) {
            return ki18nc("Keyboard variant description", variantDescriptionTbl[i].description).toString();
        }
    }
    return QString::fromLatin1(variant.constData(), variant.size());
}

QString KKeyboardLayout::optionDescription(const QByteArray &option)
{
    if (option.isEmpty()) {
        return i18nc("Keyboard option description", "None");
    }
     for (int i = 0; i < optionDescriptionTblSize; i++) {
        if (qstrcmp(option.constData(), optionDescriptionTbl[i].name) == 0) {
            return ki18nc("Keyboard option description", optionDescriptionTbl[i].description).toString();
        }
    }
    return QString::fromLatin1(option.constData(), option.size());
}

#include "moc_kkeyboardlayout.cpp"
