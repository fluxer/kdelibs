/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.0.4"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1


/* Substitute the variable and function names.  */
#define yyparse         khtmlxpathyyparse
#define yylex           khtmlxpathyylex
#define yyerror         khtmlxpathyyerror
#define yydebug         khtmlxpathyydebug
#define yynerrs         khtmlxpathyynerrs

#define yylval          khtmlxpathyylval
#define yychar          khtmlxpathyychar

/* Copy the first part of user declarations.  */
#line 1 "parser.y" /* yacc.c:339  */

#include "functions.h"
#include "path.h"
#include "predicate.h"
#include "util.h"
#include "tokenizer.h"

#include "expression.h"
#include "util.h"
#include "variablereference.h"

#include "dom/dom_string.h"
#include "dom/dom_exception.h"
#include "dom/dom3_xpath.h"
#include "xml/dom_stringimpl.h"
#include "xml/dom3_xpathimpl.h"

using namespace DOM;
using namespace DOM::XPath;
using namespace khtml;
using namespace khtml::XPath;



#include <QList>
#include <QPair>
#include <QDebug>

#ifndef NDEBUG
#define YYDEBUG 1
#endif

Expression * khtmlParseXPathStatement( const DOM::DOMString &statement, int& ec );

static Expression *_topExpr;
static int xpathParseException;


#line 113 "parser.tab.c" /* yacc.c:339  */

# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* In a future release of Bison, this section will be replaced
   by #include "parser.tab.h".  */
#ifndef YY_KHTMLXPATHYY_PARSER_TAB_H_INCLUDED
# define YY_KHTMLXPATHYY_PARSER_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int khtmlxpathyydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    MULOP = 258,
    RELOP = 259,
    EQOP = 260,
    PLUS = 261,
    MINUS = 262,
    OR = 263,
    AND = 264,
    AXISNAME = 265,
    NODETYPE = 266,
    PI = 267,
    FUNCTIONNAME = 268,
    LITERAL = 269,
    VARIABLEREFERENCE = 270,
    NUMBER = 271,
    DOTDOT = 272,
    SLASHSLASH = 273,
    NAMETEST = 274,
    ERROR = 275
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 41 "parser.y" /* yacc.c:355  */

	khtml::XPath::Step::AxisType axisType;
	int        num;
	DOM::DOMString *str; // we use this and not DOMStringImpl*, so the
	                     // memory management for this is entirely manual,
	                     // and not an RC/manual hybrid
	khtml::XPath::Expression *expr;
	QList<khtml::XPath::Predicate *> *predList;
	QList<khtml::XPath::Expression *> *argList;
	khtml::XPath::Step *step;
	khtml::XPath::LocationPath *locationPath;

#line 187 "parser.tab.c" /* yacc.c:355  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE khtmlxpathyylval;

int khtmlxpathyyparse (void);

#endif /* !YY_KHTMLXPATHYY_PARSER_TAB_H_INCLUDED  */

/* Copy the second part of user declarations.  */
#line 54 "parser.y" /* yacc.c:358  */


#line 206 "parser.tab.c" /* yacc.c:358  */

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

#ifndef YY_ATTRIBUTE
# if (defined __GNUC__                                               \
      && (2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__)))  \
     || defined __SUNPRO_C && 0x5110 <= __SUNPRO_C
#  define YY_ATTRIBUTE(Spec) __attribute__(Spec)
# else
#  define YY_ATTRIBUTE(Spec) /* empty */
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# define YY_ATTRIBUTE_PURE   YY_ATTRIBUTE ((__pure__))
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# define YY_ATTRIBUTE_UNUSED YY_ATTRIBUTE ((__unused__))
#endif

#if !defined _Noreturn \
     && (!defined __STDC_VERSION__ || __STDC_VERSION__ < 201112)
# if defined _MSC_VER && 1200 <= _MSC_VER
#  define _Noreturn __declspec (noreturn)
# else
#  define _Noreturn YY_ATTRIBUTE ((__noreturn__))
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif


#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYSIZE_T yynewbytes;                                            \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / sizeof (*yyptr);                          \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  43
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   117

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  30
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  26
/* YYNRULES -- Number of rules.  */
#define YYNRULES  60
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  90

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   275

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      23,    24,     2,     2,    28,     2,    27,    21,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    22,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    25,     2,    26,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    29,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,    95,    95,   102,   107,   114,   119,   124,   132,   138,
     143,   151,   157,   164,   170,   177,   181,   183,   190,   204,
     209,   211,   222,   228,   235,   242,   249,   254,   261,   267,
     272,   278,   284,   288,   300,   314,   320,   327,   332,   334,
     343,   348,   353,   358,   366,   371,   378,   380,   387,   389,
     396,   398,   405,   407,   414,   416,   421,   428,   430,   437,
     439
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "MULOP", "RELOP", "EQOP", "PLUS",
  "MINUS", "OR", "AND", "AXISNAME", "NODETYPE", "PI", "FUNCTIONNAME",
  "LITERAL", "VARIABLEREFERENCE", "NUMBER", "DOTDOT", "SLASHSLASH",
  "NAMETEST", "ERROR", "'/'", "'@'", "'('", "')'", "'['", "']'", "'.'",
  "','", "'|'", "$accept", "Expr", "LocationPath", "AbsoluteLocationPath",
  "RelativeLocationPath", "Step", "AxisSpecifier", "NodeTest",
  "PredicateList", "Predicate", "DescendantOrSelf", "AbbreviatedStep",
  "PrimaryExpr", "FunctionCall", "ArgumentList", "Argument", "UnionExpr",
  "PathExpr", "FilterExpr", "OrExpr", "AndExpr", "EqualityExpr",
  "RelationalExpr", "AdditiveExpr", "MultiplicativeExpr", "UnaryExpr", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,    47,    64,    40,    41,    91,    93,    46,    44,   124
};
# endif

#define YYPACT_NINF -45

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-45)))

#define YYTABLE_NINF -1

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int8 yypact[] =
{
      72,    72,   -45,   -17,     4,    14,   -45,   -45,   -45,   -45,
     -45,   -45,    -2,   -45,    72,   -45,    39,   -45,   -45,     3,
     -45,    19,    17,    -2,   -45,    17,   -45,    18,   -45,    15,
      35,    37,    43,    46,     6,    48,   -45,   -45,    28,     5,
      54,     3,    29,   -45,    -2,    -2,    17,    72,    17,   -45,
       3,    17,    90,    -2,    -2,    72,    72,    72,    72,    72,
      72,    72,   -45,    30,   -45,   -45,   -45,    -6,   -45,   -45,
     -45,   -45,    17,    31,   -45,   -45,     3,     3,    37,    43,
      46,     6,    48,    48,   -45,   -45,   -45,    72,   -45,   -45
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,    16,     0,     0,     0,    30,    28,    31,    27,
      25,    18,     5,    17,     0,    26,     0,    40,     4,     3,
       8,     0,    11,     0,    15,    44,    32,    59,    38,    41,
       2,    46,    48,    50,    52,    54,    57,    60,     0,     0,
       0,     6,     0,     1,     0,     0,    13,     0,    12,    22,
       7,    45,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    19,     0,    20,    33,    37,     0,    35,    29,
       9,    10,    14,     0,    23,    39,    42,    43,    47,    49,
      51,    53,    55,    56,    58,    21,    34,     0,    24,    36
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -45,     2,   -45,   -45,    -9,   -10,   -45,    34,   -20,   -44,
     -18,   -45,   -45,   -45,   -45,   -31,   -45,    10,   -45,   -45,
       8,    24,    40,    16,   -19,    -1
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,    66,    17,    18,    19,    20,    21,    22,    48,    49,
      23,    24,    25,    26,    67,    68,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_uint8 yytable[] =
{
      37,    45,    16,    41,    74,    51,    38,    74,     2,     3,
       4,    54,    59,    60,    50,     9,    42,    11,    86,    63,
      13,    10,    87,    45,    44,    15,    72,    39,    74,    64,
       3,     4,    45,    10,    70,    71,    53,    40,    11,    43,
      82,    83,    47,    55,    76,    77,    56,    52,    57,    73,
      58,    61,    62,    69,    85,    46,    89,    88,    45,    45,
      84,     1,    75,    78,     2,     3,     4,     5,     6,     7,
       8,     9,    10,    11,    81,    12,    13,    14,    65,     1,
      79,    15,     2,     3,     4,     5,     6,     7,     8,     9,
      10,    11,     0,    12,    13,    14,     0,    80,     0,    15,
       2,     3,     4,     5,     6,     7,     8,     9,    10,    11,
       0,    12,    13,    14,     0,     0,     0,    15
};

static const yytype_int8 yycheck[] =
{
       1,    19,     0,    12,    48,    25,    23,    51,    10,    11,
      12,    29,     6,     7,    23,    17,    14,    19,    24,    14,
      22,    18,    28,    41,    21,    27,    46,    23,    72,    24,
      11,    12,    50,    18,    44,    45,    21,    23,    19,     0,
      59,    60,    25,     8,    53,    54,     9,    29,     5,    47,
       4,     3,    24,    24,    24,    21,    87,    26,    76,    77,
      61,     7,    52,    55,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    58,    21,    22,    23,    24,     7,
      56,    27,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    -1,    21,    22,    23,    -1,    57,    -1,    27,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      -1,    21,    22,    23,    -1,    -1,    -1,    27
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     7,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    21,    22,    23,    27,    31,    32,    33,    34,
      35,    36,    37,    40,    41,    42,    43,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    55,    23,    23,
      23,    34,    31,     0,    21,    40,    37,    25,    38,    39,
      34,    38,    29,    21,    40,     8,     9,     5,     4,     6,
       7,     3,    24,    14,    24,    24,    31,    44,    45,    24,
      35,    35,    38,    31,    39,    47,    34,    34,    50,    51,
      52,    53,    54,    54,    55,    24,    24,    28,    26,    45
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    30,    31,    32,    32,    33,    33,    33,    34,    34,
      34,    35,    35,    35,    35,    35,    36,    36,    37,    37,
      37,    37,    38,    38,    39,    40,    41,    41,    42,    42,
      42,    42,    42,    43,    43,    44,    44,    45,    46,    46,
      47,    47,    47,    47,    48,    48,    49,    49,    50,    50,
      51,    51,    52,    52,    53,    53,    53,    54,    54,    55,
      55
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     1,     1,     1,     2,     2,     1,     3,
       3,     1,     2,     2,     3,     1,     1,     1,     1,     3,
       3,     4,     1,     2,     3,     1,     1,     1,     1,     3,
       1,     1,     1,     3,     4,     1,     3,     1,     1,     3,
       1,     1,     3,     3,     1,     2,     1,     3,     1,     3,
       1,     3,     1,     3,     1,     3,     3,     1,     3,     1,
       2
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
do                                                              \
  if (yychar == YYEMPTY)                                        \
    {                                                           \
      yychar = (Token);                                         \
      yylval = (Value);                                         \
      YYPOPSTACK (yylen);                                       \
      yystate = *yyssp;                                         \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    {                                                           \
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;                                                  \
    }                                                           \
while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256



/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)

/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  YYUSE (yytype);
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, int yyrule)
{
  unsigned long int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[yyssp[yyi + 1 - yynrhs]],
                       &(yyvsp[(yyi + 1) - (yynrhs)])
                                              );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
yystrlen (const char *yystr)
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            /* Fall through.  */
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (! (yysize <= yysize1
                         && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                    return 2;
                  yysize = yysize1;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    yysize = yysize1;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
{
  YYUSE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        YYSTYPE *yyvs1 = yyvs;
        yytype_int16 *yyss1 = yyss;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * sizeof (*yyssp),
                    &yyvs1, yysize * sizeof (*yyvsp),
                    &yystacksize);

        yyss = yyss1;
        yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yytype_int16 *yyss1 = yyss;
        union yyalloc *yyptr =
          (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
                  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 96 "parser.y" /* yacc.c:1646  */
    {
		_topExpr = (yyvsp[0].expr);
	}
#line 1353 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 3:
#line 103 "parser.y" /* yacc.c:1646  */
    {
		(yyval.locationPath)->m_absolute = false;
	}
#line 1361 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 4:
#line 108 "parser.y" /* yacc.c:1646  */
    {
		(yyval.locationPath)->m_absolute = true;
	}
#line 1369 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 5:
#line 115 "parser.y" /* yacc.c:1646  */
    {
		(yyval.locationPath) = new LocationPath;
	}
#line 1377 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 6:
#line 120 "parser.y" /* yacc.c:1646  */
    {
		(yyval.locationPath) = (yyvsp[0].locationPath);
	}
#line 1385 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 7:
#line 125 "parser.y" /* yacc.c:1646  */
    {
		(yyval.locationPath) = (yyvsp[0].locationPath);
		(yyval.locationPath)->m_steps.prepend( (yyvsp[-1].step) );
	}
#line 1394 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 8:
#line 133 "parser.y" /* yacc.c:1646  */
    {
		(yyval.locationPath) = new LocationPath;
		(yyval.locationPath)->m_steps.append( (yyvsp[0].step) );
	}
#line 1403 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 9:
#line 139 "parser.y" /* yacc.c:1646  */
    {
		(yyval.locationPath)->m_steps.append( (yyvsp[0].step) );
	}
#line 1411 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 10:
#line 144 "parser.y" /* yacc.c:1646  */
    {
		(yyval.locationPath)->m_steps.append( (yyvsp[-1].step) );
		(yyval.locationPath)->m_steps.append( (yyvsp[0].step) );
	}
#line 1420 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 11:
#line 152 "parser.y" /* yacc.c:1646  */
    {
		(yyval.step) = new Step( Step::ChildAxis, *(yyvsp[0].str) );
		delete (yyvsp[0].str);
	}
#line 1429 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 12:
#line 158 "parser.y" /* yacc.c:1646  */
    {
		(yyval.step) = new Step( Step::ChildAxis, *(yyvsp[-1].str), *(yyvsp[0].predList) );
		delete (yyvsp[-1].str);
		delete (yyvsp[0].predList);
	}
#line 1439 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 13:
#line 165 "parser.y" /* yacc.c:1646  */
    {
		(yyval.step) = new Step( (yyvsp[-1].axisType), *(yyvsp[0].str) );
		delete (yyvsp[0].str);
	}
#line 1448 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 14:
#line 171 "parser.y" /* yacc.c:1646  */
    {
		(yyval.step) = new Step( (yyvsp[-2].axisType), *(yyvsp[-1].str),  *(yyvsp[0].predList) );
		delete (yyvsp[-1].str);
		delete (yyvsp[0].predList);
	}
#line 1458 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 17:
#line 184 "parser.y" /* yacc.c:1646  */
    {
		(yyval.axisType) = Step::AttributeAxis;
	}
#line 1466 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 18:
#line 191 "parser.y" /* yacc.c:1646  */
    {
		const int colon = (yyvsp[0].str)->find( ':' );
		if ( colon > -1 ) {
			DOMString prefix( (yyvsp[0].str)->substring( 0, colon ) );
			XPathNSResolverImpl *resolver = Expression::evaluationContext().resolver;
			if ( !resolver || resolver->lookupNamespaceURI( prefix ).isNull() ) {
				kWarning(6011) << "Found unknown namespace prefix " << prefix.string();
				xpathParseException = DOMException::NAMESPACE_ERR;
				YYABORT;
			}
		}
	}
#line 1483 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 19:
#line 205 "parser.y" /* yacc.c:1646  */
    {
		(yyval.str) = new DOMString( *(yyvsp[-2].str) + DOMString("()") );
	}
#line 1491 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 21:
#line 212 "parser.y" /* yacc.c:1646  */
    {
		QString s = (yyvsp[-3].str)->string() + QLatin1Char(' ') + (yyvsp[-1].str)->string();
		s = s.trimmed();
		(yyval.str) = new DOMString( s );
		delete (yyvsp[-3].str);
		delete (yyvsp[-1].str);
	}
#line 1503 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 22:
#line 223 "parser.y" /* yacc.c:1646  */
    {
		(yyval.predList) = new QList<Predicate *>;
		(yyval.predList)->append( new Predicate( (yyvsp[0].expr) ) );
	}
#line 1512 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 23:
#line 229 "parser.y" /* yacc.c:1646  */
    {
		(yyval.predList)->append( new Predicate( (yyvsp[0].expr) ) );
	}
#line 1520 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 24:
#line 236 "parser.y" /* yacc.c:1646  */
    {
		(yyval.expr) = (yyvsp[-1].expr);
	}
#line 1528 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 25:
#line 243 "parser.y" /* yacc.c:1646  */
    {
		(yyval.step) = new Step( Step::DescendantOrSelfAxis, "node()" );
	}
#line 1536 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 26:
#line 250 "parser.y" /* yacc.c:1646  */
    {
		(yyval.step) = new Step( Step::SelfAxis, "node()" );
	}
#line 1544 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 27:
#line 255 "parser.y" /* yacc.c:1646  */
    {
		(yyval.step) = new Step( Step::ParentAxis, "node()" );
	}
#line 1552 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 28:
#line 262 "parser.y" /* yacc.c:1646  */
    {
		(yyval.expr) = new VariableReference( *(yyvsp[0].str) );
		delete (yyvsp[0].str);
	}
#line 1561 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 29:
#line 268 "parser.y" /* yacc.c:1646  */
    {
		(yyval.expr) = (yyvsp[-1].expr);
	}
#line 1569 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 30:
#line 273 "parser.y" /* yacc.c:1646  */
    {
		(yyval.expr) = new String( *(yyvsp[0].str) );
		delete (yyvsp[0].str);
	}
#line 1578 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 31:
#line 279 "parser.y" /* yacc.c:1646  */
    {
		(yyval.expr) = new Number( (yyvsp[0].str)->toFloat() );
		delete (yyvsp[0].str);
	}
#line 1587 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 33:
#line 289 "parser.y" /* yacc.c:1646  */
    {
		Function* f = FunctionLibrary::self().getFunction( *(yyvsp[-2].str) );
		delete (yyvsp[-2].str);
		if (!f) {
			xpathParseException = XPathException::toCode(INVALID_EXPRESSION_ERR);
			YYABORT;
		}
		
		(yyval.expr) = f;
	}
#line 1602 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 34:
#line 301 "parser.y" /* yacc.c:1646  */
    {
		Function* f = FunctionLibrary::self().getFunction( *(yyvsp[-3].str), *(yyvsp[-1].argList) );
		delete (yyvsp[-3].str);
		delete (yyvsp[-1].argList);
		if (!f) {
			xpathParseException = XPathException::toCode(INVALID_EXPRESSION_ERR);
			YYABORT;
		}
		(yyval.expr) = f;
	}
#line 1617 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 35:
#line 315 "parser.y" /* yacc.c:1646  */
    {
		(yyval.argList) = new QList<Expression *>;
		(yyval.argList)->append( (yyvsp[0].expr) );
	}
#line 1626 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 36:
#line 321 "parser.y" /* yacc.c:1646  */
    {
		(yyval.argList)->append( (yyvsp[0].expr) );
	}
#line 1634 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 39:
#line 335 "parser.y" /* yacc.c:1646  */
    {
		(yyval.expr) = new Union;
		(yyval.expr)->addSubExpression( (yyvsp[-2].expr) );
		(yyval.expr)->addSubExpression( (yyvsp[0].expr) );
	}
#line 1644 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 40:
#line 344 "parser.y" /* yacc.c:1646  */
    {
		(yyval.expr) = (yyvsp[0].locationPath);
	}
#line 1652 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 41:
#line 349 "parser.y" /* yacc.c:1646  */
    {
		(yyval.expr) = (yyvsp[0].expr);
	}
#line 1660 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 42:
#line 354 "parser.y" /* yacc.c:1646  */
    {
		(yyval.expr) = new Path( static_cast<Filter *>( (yyvsp[-2].expr) ), (yyvsp[0].locationPath) );
	}
#line 1668 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 43:
#line 359 "parser.y" /* yacc.c:1646  */
    {
		(yyvsp[0].locationPath)->m_steps.prepend( (yyvsp[-1].step) );
		(yyval.expr) = new Path( static_cast<Filter *>( (yyvsp[-2].expr) ), (yyvsp[0].locationPath) );
	}
#line 1677 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 44:
#line 367 "parser.y" /* yacc.c:1646  */
    {
		(yyval.expr) = (yyvsp[0].expr);
	}
#line 1685 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 45:
#line 372 "parser.y" /* yacc.c:1646  */
    {
		(yyval.expr) = new Filter( (yyvsp[-1].expr), *(yyvsp[0].predList) );
	}
#line 1693 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 47:
#line 381 "parser.y" /* yacc.c:1646  */
    {
		(yyval.expr) = new LogicalOp( LogicalOp::OP_Or, (yyvsp[-2].expr), (yyvsp[0].expr) );
	}
#line 1701 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 49:
#line 390 "parser.y" /* yacc.c:1646  */
    {
		(yyval.expr) = new LogicalOp( LogicalOp::OP_And, (yyvsp[-2].expr), (yyvsp[0].expr) );
	}
#line 1709 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 51:
#line 399 "parser.y" /* yacc.c:1646  */
    {
		(yyval.expr) = new RelationOp( (yyvsp[-1].num), (yyvsp[-2].expr), (yyvsp[0].expr) );
	}
#line 1717 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 53:
#line 408 "parser.y" /* yacc.c:1646  */
    {
		(yyval.expr) = new RelationOp( (yyvsp[-1].num), (yyvsp[-2].expr), (yyvsp[0].expr) );
	}
#line 1725 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 55:
#line 417 "parser.y" /* yacc.c:1646  */
    {
		(yyval.expr) = new NumericOp( NumericOp::OP_Add, (yyvsp[-2].expr), (yyvsp[0].expr) );
	}
#line 1733 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 56:
#line 422 "parser.y" /* yacc.c:1646  */
    {
		(yyval.expr) = new NumericOp( NumericOp::OP_Sub, (yyvsp[-2].expr), (yyvsp[0].expr) );
	}
#line 1741 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 58:
#line 431 "parser.y" /* yacc.c:1646  */
    {
		(yyval.expr) = new NumericOp( (yyvsp[-1].num), (yyvsp[-2].expr), (yyvsp[0].expr) );
	}
#line 1749 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 60:
#line 440 "parser.y" /* yacc.c:1646  */
    {
		(yyval.expr) = new Negative;
		(yyval.expr)->addSubExpression( (yyvsp[0].expr) );
	}
#line 1758 "parser.tab.c" /* yacc.c:1646  */
    break;


#line 1762 "parser.tab.c" /* yacc.c:1646  */
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYTERROR;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  return yyresult;
}
#line 446 "parser.y" /* yacc.c:1906  */


namespace khtml {
namespace XPath {

Expression *khtmlParseXPathStatement( const DOM::DOMString &statement, int& ec )
{
//	qDebug() << "Parsing " << statement;
	xpathParseException = 0;
	_topExpr = 0;
	initTokenizer( statement );
	yyparse();

	if (xpathParseException)
		ec = xpathParseException;
	return _topExpr;
}

void khtmlxpathyyerror(const char *str)
{
	// XXX Clean xpathyylval.str up to avoid leaking it.
	fprintf(stderr, "error: %s\n", str);
	xpathParseException = XPathException::toCode(INVALID_EXPRESSION_ERR);
}

} // namespace XPath
} // namespace khtml

// kate: indent-width 4; replace-tabs off; tab-width 4; space-indent off;
