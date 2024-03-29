/* C++ code produced by gperf version 3.1 */
/* Command-line: gperf -a -o -t -k '*' -D -Wwordlist_Entity -s 2 kentities.gperf  */

#if !((' ' == 32) && ('!' == 33) && ('"' == 34) && ('#' == 35) \
      && ('%' == 37) && ('&' == 38) && ('\'' == 39) && ('(' == 40) \
      && (')' == 41) && ('*' == 42) && ('+' == 43) && (',' == 44) \
      && ('-' == 45) && ('.' == 46) && ('/' == 47) && ('0' == 48) \
      && ('1' == 49) && ('2' == 50) && ('3' == 51) && ('4' == 52) \
      && ('5' == 53) && ('6' == 54) && ('7' == 55) && ('8' == 56) \
      && ('9' == 57) && (':' == 58) && (';' == 59) && ('<' == 60) \
      && ('=' == 61) && ('>' == 62) && ('?' == 63) && ('A' == 65) \
      && ('B' == 66) && ('C' == 67) && ('D' == 68) && ('E' == 69) \
      && ('F' == 70) && ('G' == 71) && ('H' == 72) && ('I' == 73) \
      && ('J' == 74) && ('K' == 75) && ('L' == 76) && ('M' == 77) \
      && ('N' == 78) && ('O' == 79) && ('P' == 80) && ('Q' == 81) \
      && ('R' == 82) && ('S' == 83) && ('T' == 84) && ('U' == 85) \
      && ('V' == 86) && ('W' == 87) && ('X' == 88) && ('Y' == 89) \
      && ('Z' == 90) && ('[' == 91) && ('\\' == 92) && (']' == 93) \
      && ('^' == 94) && ('_' == 95) && ('a' == 97) && ('b' == 98) \
      && ('c' == 99) && ('d' == 100) && ('e' == 101) && ('f' == 102) \
      && ('g' == 103) && ('h' == 104) && ('i' == 105) && ('j' == 106) \
      && ('k' == 107) && ('l' == 108) && ('m' == 109) && ('n' == 110) \
      && ('o' == 111) && ('p' == 112) && ('q' == 113) && ('r' == 114) \
      && ('s' == 115) && ('t' == 116) && ('u' == 117) && ('v' == 118) \
      && ('w' == 119) && ('x' == 120) && ('y' == 121) && ('z' == 122) \
      && ('{' == 123) && ('|' == 124) && ('}' == 125) && ('~' == 126))
/* The character set is not based on ISO-646.  */
#error "gperf generated tables don't work with this execution character set. Please report a bug to <bug-gperf@gnu.org>."
#endif

#line 1 "kentities.gperf"

/*   This file is part of the KDE libraries
  
     Copyright (C) 1999 Lars Knoll (knoll@mpi-hd.mpg.de)
  
     This library is free software; you can redistribute it and/or
     modify it under the terms of the GNU Library General Public
     License as published by the Free Software Foundation; either
     version 2 of the License, or (at your option) any later version.
  
     This library is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
     Library General Public License for more details.
  
     You should have received a copy of the GNU Library General Public License
     along with this library; see the file COPYING.LIB.  If not, write to
     the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
     Boston, MA 02110-1301, USA.
  
  ----------------------------------------------------------------------------
  
    kentities.gperf: input file to generate a hash table for entities
    kentities.c: DO NOT EDIT! generated by the command
    "gperf -a -o -t -k '*' -D -Wwordlist_Entity -s 2 kentities.gperf > kentities.cpp"
    from kentities.gperf
*/
#line 38 "kentities.gperf"
struct entity {
    const char *name;
    int code;
};
/* maximum key range = 1134, duplicates = 0 */

class EntitiesHash
{
private:
  static inline unsigned int hash_Entity (const char *str, size_t len);
public:
  static const struct entity *kde_findEntity (const char *str, size_t len);
};

inline unsigned int
EntitiesHash::hash_Entity (const char *str, size_t len)
{
  static const unsigned short asso_values[] =
    {
      1142, 1142, 1142, 1142, 1142, 1142, 1142, 1142, 1142, 1142,
      1142, 1142, 1142, 1142, 1142, 1142, 1142, 1142, 1142, 1142,
      1142, 1142, 1142, 1142, 1142, 1142, 1142, 1142, 1142, 1142,
      1142, 1142, 1142, 1142, 1142, 1142, 1142, 1142, 1142, 1142,
      1142, 1142, 1142, 1142, 1142, 1142, 1142, 1142, 1142,    0,
        10,    5,    0, 1142, 1142, 1142, 1142, 1142, 1142, 1142,
      1142, 1142, 1142, 1142, 1142,   95,  465,  125,   40,  210,
        20,   30,    0,  295, 1142,    5,    5,    0,   30,   85,
        35,    0,   65,   50,   10,  140,    0, 1142,    0,   55,
        45, 1142, 1142, 1142, 1142, 1142, 1142,    0,   15,   70,
         0,   20,  230,  275,  225,   10,  275,    0,  130,  185,
        15,    5,   90,   80,    5,  170,   25,   25,   55,   40,
        75,  180,   35,    0, 1142, 1142, 1142, 1142, 1142, 1142,
      1142, 1142, 1142, 1142, 1142, 1142, 1142, 1142, 1142, 1142,
      1142, 1142, 1142, 1142, 1142, 1142, 1142, 1142, 1142, 1142,
      1142, 1142, 1142, 1142, 1142, 1142, 1142, 1142, 1142, 1142,
      1142, 1142, 1142, 1142, 1142, 1142, 1142, 1142, 1142, 1142,
      1142, 1142, 1142, 1142, 1142, 1142, 1142, 1142, 1142, 1142,
      1142, 1142, 1142, 1142, 1142, 1142, 1142, 1142, 1142, 1142,
      1142, 1142, 1142, 1142, 1142, 1142, 1142, 1142, 1142, 1142,
      1142, 1142, 1142, 1142, 1142, 1142, 1142, 1142, 1142, 1142,
      1142, 1142, 1142, 1142, 1142, 1142, 1142, 1142, 1142, 1142,
      1142, 1142, 1142, 1142, 1142, 1142, 1142, 1142, 1142, 1142,
      1142, 1142, 1142, 1142, 1142, 1142, 1142, 1142, 1142, 1142,
      1142, 1142, 1142, 1142, 1142, 1142, 1142, 1142, 1142, 1142,
      1142, 1142, 1142, 1142, 1142, 1142, 1142
    };
  unsigned int hval = len;

  switch (hval)
    {
      default:
        hval += asso_values[static_cast<unsigned char>(str[7])];
      /*FALLTHROUGH*/
      case 7:
        hval += asso_values[static_cast<unsigned char>(str[6])];
      /*FALLTHROUGH*/
      case 6:
        hval += asso_values[static_cast<unsigned char>(str[5])];
      /*FALLTHROUGH*/
      case 5:
        hval += asso_values[static_cast<unsigned char>(str[4])];
      /*FALLTHROUGH*/
      case 4:
        hval += asso_values[static_cast<unsigned char>(str[3])];
      /*FALLTHROUGH*/
      case 3:
        hval += asso_values[static_cast<unsigned char>(str[2])];
      /*FALLTHROUGH*/
      case 2:
        hval += asso_values[static_cast<unsigned char>(str[1]+1)];
      /*FALLTHROUGH*/
      case 1:
        hval += asso_values[static_cast<unsigned char>(str[0])];
        break;
    }
  return hval;
}

const struct entity *
EntitiesHash::kde_findEntity (const char *str, size_t len)
{
  enum
    {
      TOTAL_KEYWORDS = 281,
      MIN_WORD_LENGTH = 2,
      MAX_WORD_LENGTH = 8,
      MIN_HASH_VALUE = 8,
      MAX_HASH_VALUE = 1141
    };

  static const struct entity wordlist_Entity[] =
    {
#line 123 "kentities.gperf"
      {"and", 0x2227},
#line 272 "kentities.gperf"
      {"rho", 0x03c1},
#line 149 "kentities.gperf"
      {"darr", 0x2193},
#line 150 "kentities.gperf"
      {"dcaron", 0x10f},
#line 265 "kentities.gperf"
      {"rarr", 0x2192},
#line 266 "kentities.gperf"
      {"rcaron", 0x0159},
#line 102 "kentities.gperf"
      {"Tcaron", 0x0164},
#line 193 "kentities.gperf"
      {"int", 0x222b},
#line 222 "kentities.gperf"
      {"ncaron", 0x0148},
#line 169 "kentities.gperf"
      {"eta", 0x03b7},
#line 158 "kentities.gperf"
      {"ecaron", 0x011b},
#line 101 "kentities.gperf"
      {"Tau", 0x03a4},
#line 307 "kentities.gperf"
      {"uarr", 0x2191},
#line 296 "kentities.gperf"
      {"tcaron", 0x0165},
#line 78 "kentities.gperf"
      {"Mu", 0x039c},
#line 91 "kentities.gperf"
      {"Phi", 0x03a6},
#line 79 "kentities.gperf"
      {"Ncaron", 0x0147},
#line 196 "kentities.gperf"
      {"isin", 0x2208},
#line 301 "kentities.gperf"
      {"thorn", 0x00fe},
#line 320 "kentities.gperf"
      {"zcaron", 0x017e},
#line 295 "kentities.gperf"
      {"tau", 0x03c4},
#line 103 "kentities.gperf"
      {"Theta", 0x0398},
#line 57 "kentities.gperf"
      {"Dcaron", 0x010e},
#line 230 "kentities.gperf"
      {"nu", 0x03bd},
#line 94 "kentities.gperf"
      {"Psi", 0x03a8},
#line 117 "kentities.gperf"
      {"acute", 0x00b4},
#line 113 "kentities.gperf"
      {"Zcaron", 0x017d},
#line 98 "kentities.gperf"
      {"Scaron", 0x0160},
#line 97 "kentities.gperf"
      {"Rho", 0x03a1},
#line 228 "kentities.gperf"
      {"nsub", 0x2284},
#line 298 "kentities.gperf"
      {"theta", 0x03b8},
#line 297 "kentities.gperf"
      {"there4", 0x2234},
#line 81 "kentities.gperf"
      {"Nu", 0x039d},
#line 172 "kentities.gperf"
      {"euro", 0x20ac},
#line 116 "kentities.gperf"
      {"acirc", 0x00e2},
#line 139 "kentities.gperf"
      {"chi", 0x03c7},
#line 232 "kentities.gperf"
      {"ocirc", 0x00f4},
#line 96 "kentities.gperf"
      {"Rcaron", 0x0158},
#line 95 "kentities.gperf"
      {"QUOT", 34},
#line 188 "kentities.gperf"
      {"icirc", 0x00ee},
#line 135 "kentities.gperf"
      {"ccaron", 0x010d},
#line 262 "kentities.gperf"
      {"radic", 0x221a},
#line 122 "kentities.gperf"
      {"amp", 38},
#line 160 "kentities.gperf"
      {"ecirc", 0x00ea},
#line 251 "kentities.gperf"
      {"phi", 0x03c6},
#line 246 "kentities.gperf"
      {"para", 0x00b6},
#line 308 "kentities.gperf"
      {"ucirc", 0x00fb},
#line 168 "kentities.gperf"
      {"equiv", 0x2261},
#line 259 "kentities.gperf"
      {"psi", 0x03c8},
#line 194 "kentities.gperf"
      {"iota", 0x03b9},
#line 237 "kentities.gperf"
      {"omicron", 0x03bf},
#line 226 "kentities.gperf"
      {"not", 0x00ac},
#line 264 "kentities.gperf"
      {"raquo", 0x00bb},
#line 247 "kentities.gperf"
      {"part", 0x2202},
#line 268 "kentities.gperf"
      {"rdquo", 0x201d},
#line 275 "kentities.gperf"
      {"rsquo", 0x2019},
#line 274 "kentities.gperf"
      {"rsaquo", 0x203a},
#line 77 "kentities.gperf"
      {"LT", 60},
#line 55 "kentities.gperf"
      {"Chi", 0x03a7},
#line 130 "kentities.gperf"
      {"bdquo", 0x201e},
#line 53 "kentities.gperf"
      {"Ccaron", 0x010c},
#line 213 "kentities.gperf"
      {"lt", 60},
#line 203 "kentities.gperf"
      {"larr", 0x2190},
#line 227 "kentities.gperf"
      {"notin", 0x2209},
#line 115 "kentities.gperf"
      {"aacute", 0x00e1},
#line 48 "kentities.gperf"
      {"AMP", 38},
#line 231 "kentities.gperf"
      {"oacute", 0x00f3},
#line 260 "kentities.gperf"
      {"quot", 34},
#line 267 "kentities.gperf"
      {"rceil", 0x2309},
#line 187 "kentities.gperf"
      {"iacute", 0x00ed},
#line 69 "kentities.gperf"
      {"GT", 62},
#line 310 "kentities.gperf"
      {"uml", 0x00a8},
#line 84 "kentities.gperf"
      {"Ocirc", 0x00d4},
#line 146 "kentities.gperf"
      {"curren", 0x00a4},
#line 239 "kentities.gperf"
      {"or", 0x2228},
#line 134 "kentities.gperf"
      {"cap", 0x2229},
#line 220 "kentities.gperf"
      {"nabla", 0x2207},
#line 157 "kentities.gperf"
      {"eacute", 0x00e9},
#line 45 "kentities.gperf"
      {"Acirc", 0x00c2},
#line 306 "kentities.gperf"
      {"uacute", 0x00fa},
#line 128 "kentities.gperf"
      {"atilde", 0x00e3},
#line 243 "kentities.gperf"
      {"otilde", 0x00f5},
#line 198 "kentities.gperf"
      {"kappa", 0x03ba},
#line 277 "kentities.gperf"
      {"scaron", 0x0161},
#line 75 "kentities.gperf"
      {"Kappa", 0x039a},
#line 229 "kentities.gperf"
      {"ntilde", 0x00f1},
#line 87 "kentities.gperf"
      {"Omicron", 0x039f},
#line 111 "kentities.gperf"
      {"Yacute", 0x00dd},
#line 145 "kentities.gperf"
      {"cup", 0x222a},
#line 304 "kentities.gperf"
      {"trade", 0x2122},
#line 80 "kentities.gperf"
      {"Ntilde", 0x00d1},
#line 155 "kentities.gperf"
      {"dol", 0x0024},
#line 278 "kentities.gperf"
      {"sdot", 0x22c5},
#line 255 "kentities.gperf"
      {"pound", 0x00a3},
#line 76 "kentities.gperf"
      {"Lambda", 0x039b},
#line 105 "kentities.gperf"
      {"Ucirc", 0x00db},
#line 136 "kentities.gperf"
      {"ccedil", 0x00e7},
#line 66 "kentities.gperf"
      {"Eta", 0x0397},
#line 235 "kentities.gperf"
      {"oline", 0x203e},
#line 61 "kentities.gperf"
      {"Ecaron", 0x011a},
#line 219 "kentities.gperf"
      {"mu", 0x03bc},
#line 285 "kentities.gperf"
      {"sub", 0x2282},
#line 83 "kentities.gperf"
      {"Oacute", 0x00d3},
#line 224 "kentities.gperf"
      {"ne", 0x2260},
#line 184 "kentities.gperf"
      {"harr", 0x2194},
#line 144 "kentities.gperf"
      {"crarr", 0x21b5},
#line 44 "kentities.gperf"
      {"Aacute", 0x00c1},
#line 209 "kentities.gperf"
      {"loz", 0x25ca},
#line 125 "kentities.gperf"
      {"apos", 0x0027},
#line 202 "kentities.gperf"
      {"laquo", 0x00ab},
#line 195 "kentities.gperf"
      {"iquest", 0x00bf},
#line 286 "kentities.gperf"
      {"sube", 0x2286},
#line 205 "kentities.gperf"
      {"ldquo", 0x201c},
#line 132 "kentities.gperf"
      {"brvbar", 0x00a6},
#line 257 "kentities.gperf"
      {"prod", 0x220f},
#line 212 "kentities.gperf"
      {"lsquo", 0x2018},
#line 211 "kentities.gperf"
      {"lsaquo", 0x2039},
#line 170 "kentities.gperf"
      {"eth", 0x00f0},
#line 131 "kentities.gperf"
      {"beta", 0x03b2},
#line 192 "kentities.gperf"
      {"infin", 0x221e},
#line 89 "kentities.gperf"
      {"Otilde", 0x00d5},
#line 110 "kentities.gperf"
      {"Xi", 0x039e},
#line 214 "kentities.gperf"
      {"macr", 0x00af},
#line 124 "kentities.gperf"
      {"ang", 0x2220},
#line 50 "kentities.gperf"
      {"Atilde", 0x00c3},
#line 166 "kentities.gperf"
      {"ensp", 0x2002},
#line 54 "kentities.gperf"
      {"Ccedil", 0x00c7},
#line 225 "kentities.gperf"
      {"ni", 0x220b},
#line 321 "kentities.gperf"
      {"zeta", 0x03b6},
#line 204 "kentities.gperf"
      {"lceil", 0x2308},
#line 164 "kentities.gperf"
      {"emsp", 0x2003},
#line 62 "kentities.gperf"
      {"Ecirc", 0x00ca},
#line 104 "kentities.gperf"
      {"Uacute", 0x00da},
#line 182 "kentities.gperf"
      {"gt", 62},
#line 114 "kentities.gperf"
      {"Zeta", 0x0396},
#line 119 "kentities.gperf"
      {"agrave", 0x00e0},
#line 92 "kentities.gperf"
      {"Pi", 0x03a0},
#line 263 "kentities.gperf"
      {"rang", 0x232a},
#line 234 "kentities.gperf"
      {"ograve", 0x00f2},
#line 292 "kentities.gperf"
      {"sup", 0x2283},
#line 288 "kentities.gperf"
      {"sup1", 0x00b9},
#line 236 "kentities.gperf"
      {"omega", 0x03c9},
#line 190 "kentities.gperf"
      {"igrave", 0x00ec},
#line 291 "kentities.gperf"
      {"sup3", 0x00b3},
#line 191 "kentities.gperf"
      {"image", 0x2111},
#line 300 "kentities.gperf"
      {"thinsp", 0x2009},
#line 290 "kentities.gperf"
      {"sup2", 0x00b2},
#line 161 "kentities.gperf"
      {"egrave", 0x00e8},
#line 133 "kentities.gperf"
      {"bull", 0x2022},
#line 163 "kentities.gperf"
      {"empty", 0x2205},
#line 309 "kentities.gperf"
      {"ugrave", 0x00f9},
#line 293 "kentities.gperf"
      {"supe", 0x2287},
#line 317 "kentities.gperf"
      {"yacute", 0x00fd},
#line 138 "kentities.gperf"
      {"cent", 0x00a2},
#line 221 "kentities.gperf"
      {"nbsp", 0x00a0},
#line 200 "kentities.gperf"
      {"lambda", 0x03bb},
#line 316 "kentities.gperf"
      {"xi", 0x03be},
#line 59 "kentities.gperf"
      {"ETH", 0x00d0},
#line 276 "kentities.gperf"
      {"sbquo", 0x201a},
#line 258 "kentities.gperf"
      {"prop", 0x221d},
#line 159 "kentities.gperf"
      {"eague", 0x00e9},
#line 156 "kentities.gperf"
      {"dollar", 0x0024},
#line 206 "kentities.gperf"
      {"le", 0x2264},
#line 280 "kentities.gperf"
      {"shy", 0x00ad},
#line 241 "kentities.gperf"
      {"ordm", 0x00ba},
#line 154 "kentities.gperf"
      {"divide", 0x00f7},
#line 252 "kentities.gperf"
      {"pi", 0x03c0},
#line 269 "kentities.gperf"
      {"real", 0x211c},
#line 60 "kentities.gperf"
      {"Eacute", 0x00c9},
#line 129 "kentities.gperf"
      {"auml", 0x00e4},
#line 273 "kentities.gperf"
      {"rlm", 0x200f},
#line 245 "kentities.gperf"
      {"ouml", 0x00f6},
#line 197 "kentities.gperf"
      {"iuml", 0x00ef},
#line 71 "kentities.gperf"
      {"Icirc", 0x00ce},
#line 322 "kentities.gperf"
      {"zwj", 0x200d},
#line 152 "kentities.gperf"
      {"delta", 0x03b4},
#line 171 "kentities.gperf"
      {"euml", 0x00eb},
#line 85 "kentities.gperf"
      {"Ograve", 0x00d2},
#line 314 "kentities.gperf"
      {"uuml", 0x00fc},
#line 86 "kentities.gperf"
      {"Omega", 0x03a9},
#line 315 "kentities.gperf"
      {"weierp", 0x2118},
#line 323 "kentities.gperf"
      {"zwnj", 0x200c},
#line 46 "kentities.gperf"
      {"Agrave", 0x00c0},
#line 240 "kentities.gperf"
      {"ordf", 0x00aa},
#line 173 "kentities.gperf"
      {"exist", 0x2203},
#line 287 "kentities.gperf"
      {"sum", 0x2211},
#line 73 "kentities.gperf"
      {"Iota", 0x0399},
#line 238 "kentities.gperf"
      {"oplus", 0x2295},
#line 250 "kentities.gperf"
      {"perp", 0x22a5},
#line 68 "kentities.gperf"
      {"Gamma", 0x0393},
#line 244 "kentities.gperf"
      {"otimes", 0x2297},
#line 253 "kentities.gperf"
      {"piv", 0x03d6},
#line 140 "kentities.gperf"
      {"circ", 0x02c6},
#line 93 "kentities.gperf"
      {"Prime", 0x2033},
#line 165 "kentities.gperf"
      {"endash", 0x2013},
#line 318 "kentities.gperf"
      {"yen", 0x00a5},
#line 112 "kentities.gperf"
      {"Yuml", 0x0178},
#line 58 "kentities.gperf"
      {"Delta", 0x0394},
#line 271 "kentities.gperf"
      {"rfloor", 0x230b},
#line 143 "kentities.gperf"
      {"copy", 0x00a9},
#line 223 "kentities.gperf"
      {"ndash", 0x2013},
#line 162 "kentities.gperf"
      {"emdash", 0x2014},
#line 167 "kentities.gperf"
      {"epsilon", 0x03b5},
#line 201 "kentities.gperf"
      {"lang", 0x2329},
#line 248 "kentities.gperf"
      {"percnt", 0x0025},
#line 312 "kentities.gperf"
      {"upsilon", 0x03c5},
#line 137 "kentities.gperf"
      {"cedil", 0x00b8},
#line 284 "kentities.gperf"
      {"spades", 0x2660},
#line 289 "kentities.gperf"
      {"supl", 0x00b9},
#line 106 "kentities.gperf"
      {"Ugrave", 0x00d9},
#line 142 "kentities.gperf"
      {"cong", 0x2245},
#line 302 "kentities.gperf"
      {"tilde", 0x02dc},
#line 70 "kentities.gperf"
      {"Iacute", 0x00cd},
#line 90 "kentities.gperf"
      {"Ouml", 0x00d6},
#line 208 "kentities.gperf"
      {"lowast", 0x2217},
#line 51 "kentities.gperf"
      {"Auml", 0x00c4},
#line 141 "kentities.gperf"
      {"clubs", 0x2663},
#line 174 "kentities.gperf"
      {"fnof", 0x0192},
#line 126 "kentities.gperf"
      {"aring", 0x00e5},
#line 177 "kentities.gperf"
      {"frac14", 0x00bc},
#line 147 "kentities.gperf"
      {"dArr", 0x21d3},
#line 256 "kentities.gperf"
      {"prime", 0x2032},
#line 178 "kentities.gperf"
      {"frac34", 0x00be},
#line 261 "kentities.gperf"
      {"rArr", 0x21d2},
#line 127 "kentities.gperf"
      {"asymp", 0x2248},
#line 176 "kentities.gperf"
      {"frac12", 0x00bd},
#line 210 "kentities.gperf"
      {"lrm", 0x200e},
#line 100 "kentities.gperf"
      {"THORN", 0x00de},
#line 217 "kentities.gperf"
      {"middot", 0x00b7},
#line 279 "kentities.gperf"
      {"sect", 0x00a7},
#line 313 "kentities.gperf"
      {"uring", 0x016f},
#line 305 "kentities.gperf"
      {"uArr", 0x21d1},
#line 121 "kentities.gperf"
      {"alpha", 0x03b1},
#line 181 "kentities.gperf"
      {"ge", 0x2265},
#line 151 "kentities.gperf"
      {"deg", 0x00b0},
#line 270 "kentities.gperf"
      {"reg", 0x00ae},
#line 109 "kentities.gperf"
      {"Uuml", 0x00dc},
#line 311 "kentities.gperf"
      {"upsih", 0x03d2},
#line 189 "kentities.gperf"
      {"iexcl", 0x00a1},
#line 63 "kentities.gperf"
      {"Egrave", 0x00c8},
#line 82 "kentities.gperf"
      {"OElig", 0x0152},
#line 43 "kentities.gperf"
      {"AElig", 0x00c6},
#line 216 "kentities.gperf"
      {"micro", 0x00b5},
#line 64 "kentities.gperf"
      {"Eague", 0x00c9},
#line 319 "kentities.gperf"
      {"yuml", 0x00ff},
#line 207 "kentities.gperf"
      {"lfloor", 0x230a},
#line 107 "kentities.gperf"
      {"Upsilon", 0x03a5},
#line 242 "kentities.gperf"
      {"oslash", 0x00f8},
#line 49 "kentities.gperf"
      {"Aring", 0x00c5},
#line 67 "kentities.gperf"
      {"Euml", 0x00cb},
#line 294 "kentities.gperf"
      {"szlig", 0x00df},
#line 175 "kentities.gperf"
      {"forall", 0x2200},
#line 148 "kentities.gperf"
      {"dagger", 0x2020},
#line 47 "kentities.gperf"
      {"Alpha", 0x0391},
#line 215 "kentities.gperf"
      {"mdash", 0x2014},
#line 72 "kentities.gperf"
      {"Igrave", 0x00cc},
#line 199 "kentities.gperf"
      {"lArr", 0x21d0},
#line 108 "kentities.gperf"
      {"Uring", 0x016e},
#line 299 "kentities.gperf"
      {"thetasym", 0x03d1},
#line 65 "kentities.gperf"
      {"Epsilon", 0x0395},
#line 283 "kentities.gperf"
      {"sim", 0x223c},
#line 153 "kentities.gperf"
      {"diams", 0x2666},
#line 56 "kentities.gperf"
      {"Dagger", 0x2021},
#line 88 "kentities.gperf"
      {"Oslash", 0x00d8},
#line 118 "kentities.gperf"
      {"aelig", 0x00e6},
#line 233 "kentities.gperf"
      {"oelig", 0x0153},
#line 249 "kentities.gperf"
      {"permil", 0x2030},
#line 185 "kentities.gperf"
      {"hearts", 0x2665},
#line 180 "kentities.gperf"
      {"gamma", 0x03b3},
#line 74 "kentities.gperf"
      {"Iuml", 0x00cf},
#line 218 "kentities.gperf"
      {"minus", 0x2212},
#line 254 "kentities.gperf"
      {"plusmn", 0x00b1},
#line 303 "kentities.gperf"
      {"times", 0x00d7},
#line 183 "kentities.gperf"
      {"hArr", 0x21d4},
#line 179 "kentities.gperf"
      {"frasl", 0x2044},
#line 52 "kentities.gperf"
      {"Beta", 0x0392},
#line 99 "kentities.gperf"
      {"Sigma", 0x03a3},
#line 186 "kentities.gperf"
      {"hellip", 0x2026},
#line 281 "kentities.gperf"
      {"sigma", 0x03c3},
#line 120 "kentities.gperf"
      {"alefsym", 0x2135},
#line 282 "kentities.gperf"
      {"sigmaf", 0x03c2}
    };

  static const short lookup[] =
    {
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,   0,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,   1,  -1,  -1,  -1,  -1,  -1,   2,
       -1,   3,  -1,  -1,   4,  -1,   5,  -1,  -1,  -1,
       -1,   6,  -1,   7,  -1,  -1,   8,  -1,   9,  -1,
       -1,  10,  -1,  11,  12,  -1,  13,  14,  15,  -1,
       -1,  16,  -1,  -1,  17,  18,  19,  -1,  20,  -1,
       21,  22,  23,  24,  -1,  25,  26,  -1,  -1,  -1,
       -1,  27,  -1,  28,  29,  30,  31,  32,  -1,  33,
       34,  -1,  -1,  35,  -1,  36,  37,  -1,  -1,  38,
       39,  40,  -1,  -1,  -1,  41,  -1,  -1,  42,  -1,
       43,  -1,  -1,  44,  45,  46,  -1,  -1,  -1,  -1,
       47,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  48,  49,
       -1,  -1,  50,  51,  -1,  52,  -1,  -1,  -1,  53,
       54,  -1,  -1,  -1,  -1,  55,  56,  57,  58,  -1,
       59,  -1,  -1,  -1,  -1,  -1,  60,  61,  -1,  62,
       63,  64,  -1,  65,  -1,  -1,  66,  -1,  -1,  67,
       68,  69,  70,  71,  -1,  72,  73,  74,  75,  -1,
       76,  77,  -1,  -1,  -1,  78,  79,  -1,  -1,  -1,
       -1,  80,  -1,  -1,  -1,  -1,  81,  -1,  -1,  -1,
       82,  83,  -1,  -1,  -1,  84,  85,  -1,  -1,  -1,
       -1,  -1,  86,  -1,  -1,  -1,  87,  -1,  88,  -1,
       89,  90,  -1,  91,  92,  93,  94,  -1,  -1,  -1,
       95,  -1,  -1,  -1,  -1,  -1,  96,  -1,  97,  -1,
       98,  99, 100, 101,  -1,  -1, 102, 103,  -1,  -1,
       -1,  -1,  -1,  -1, 104, 105, 106,  -1, 107, 108,
      109, 110,  -1,  -1, 111, 112, 113,  -1,  -1, 114,
      115, 116,  -1, 117, 118, 119, 120, 121,  -1, 122,
       -1,  -1,  -1, 123,  -1,  -1, 124,  -1,  -1, 125,
       -1, 126, 127,  -1, 128, 129,  -1,  -1,  -1, 130,
      131, 132, 133,  -1, 134,  -1,  -1,  -1,  -1,  -1,
       -1, 135, 136,  -1, 137,  -1, 138,  -1, 139, 140,
      141, 142,  -1,  -1, 143, 144, 145,  -1,  -1, 146,
       -1, 147,  -1,  -1, 148, 149, 150,  -1,  -1, 151,
       -1, 152,  -1,  -1, 153,  -1,  -1,  -1,  -1, 154,
       -1, 155, 156, 157,  -1, 158,  -1,  -1,  -1, 159,
      160, 161, 162, 163, 164,  -1, 165, 166,  -1, 167,
       -1, 168,  -1,  -1, 169,  -1,  -1,  -1, 170, 171,
       -1,  -1,  -1,  -1, 172, 173,  -1,  -1, 174,  -1,
      175,  -1,  -1,  -1, 176,  -1, 177,  -1,  -1, 178,
      179, 180,  -1,  -1, 181,  -1, 182,  -1,  -1, 183,
      184,  -1,  -1, 185, 186, 187,  -1,  -1,  -1, 188,
      189, 190,  -1, 191, 192, 193, 194,  -1, 195, 196,
      197, 198,  -1,  -1, 199, 200, 201, 202,  -1, 203,
       -1, 204, 205,  -1,  -1, 206, 207,  -1,  -1, 208,
       -1, 209,  -1,  -1, 210, 211, 212,  -1,  -1, 213,
       -1, 214,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 215,
      216,  -1,  -1,  -1, 217, 218, 219,  -1,  -1, 220,
      221, 222,  -1,  -1, 223, 224, 225,  -1, 226,  -1,
      227,  -1,  -1,  -1,  -1,  -1, 228,  -1,  -1, 229,
      230,  -1,  -1,  -1, 231, 232,  -1, 233, 234,  -1,
       -1,  -1,  -1, 235, 236, 237,  -1,  -1,  -1,  -1,
      238, 239,  -1,  -1,  -1, 240,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1, 241,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1, 242,  -1,  -1,  -1,  -1,
      243,  -1,  -1,  -1, 244,  -1, 245, 246,  -1,  -1,
       -1, 247,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
      248,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1, 249,  -1,  -1,  -1,  -1,  -1,
      250, 251,  -1,  -1,  -1,  -1, 252,  -1,  -1,  -1,
      253,  -1,  -1,  -1,  -1, 254, 255,  -1,  -1, 256,
       -1,  -1,  -1,  -1,  -1, 257,  -1,  -1,  -1,  -1,
       -1,  -1,  -1, 258,  -1,  -1,  -1, 259,  -1,  -1,
       -1,  -1,  -1, 260,  -1, 261, 262,  -1,  -1,  -1,
       -1, 263,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
      264,  -1,  -1,  -1,  -1, 265, 266,  -1,  -1,  -1,
       -1, 267,  -1,  -1,  -1, 268,  -1,  -1,  -1, 269,
       -1,  -1,  -1,  -1,  -1, 270, 271,  -1,  -1,  -1,
      272,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1, 273, 274,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1, 275,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
      276,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1, 277,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
      278,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1, 279,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1, 280
    };

  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      unsigned int key = hash_Entity (str, len);

      if (key <= MAX_HASH_VALUE)
        {
          int index = lookup[key];

          if (index >= 0)
            {
              const char *s = wordlist_Entity[index].name;

              if (*str == *s && !strncmp (str + 1, s + 1, len - 1) && s[len] == '\0')
                return &wordlist_Entity[index];
            }
        }
    }
  return 0;
}
#line 324 "kentities.gperf"


