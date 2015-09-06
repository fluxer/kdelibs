#!/bin/sh

# a little helper script to regenerate predicate_parser.[c,h] and predicate_lexer.c

YACC="bison"
LEX="flex"

$YACC -p Solid -d -o predicate_parser.c predicate_parser.y
$LEX -PSolid -B -i -opredicate_lexer.c predicate_lexer.l
