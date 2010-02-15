##  Copyright (c) 1998 by Lucent Technologies 


# The following replacement for c.lex should give correct (ANSI)
# In particular, we don't allow
# 
# char *t = "abd
# lkj";
# 
# GCC accepts this, but SGI cc does not. This program is not ANSI
# compliant.


###           "Syntax, my lad. It has been restored
###            to the highest place in the republic."
###
###                             -- John Steinbeck


Semantic_Value = tokens::Semantic_Value;
Source_Position = Int;
Lex_Result = tokens::Token( Semantic_Value, Source_Position );

Err_Warn = { err:  (Source_Position, Source_Position, String) -> Void,
             warn: (Source_Position, Source_Position, String) -> Void
           };

Lex_Arg =  { comment_nesting_depth : Ref Int, 
             source_map : source_map::Sourcemap,
	     charlist : Ref List String,
	     stringstart : Ref Int,  #  start of current string or comment
	     err_warn: Err_Warn
           };

Arg = Lex_Arg;
Token( X, Y ) = tokens::Token( X, Y );

fun ordof (s, i) =   char::to_int (string::get (s, i));

fun dec (i_ref : Ref Int) =   i_ref := *i_ref - 1;
fun inc (i_ref : Ref Int) =   i_ref := *i_ref + 1;

fun chr i = string::from_char(char::from_int i);
fun ord s = char::to_int(string::get(s, 0));
fun explode s = char_vector::fold_right (fn (c, l) =  str c ! l) [] s;
fun implode str_list = string::cat str_list;

fun hd [] => {   print "c.lex: hd of empty\n";
	        raise exception EMPTY;
            };
    hd (h ! l) => h;
end;

eof = fn ({ comment_nesting_depth,err_warn,source_map,stringstart,charlist}: Lex_Arg) =
	    { pos = int::max(*stringstart+2, source_map::curr_pos source_map);

	      if (*comment_nesting_depth > 0 )
                  err_warn.err ( *stringstart, pos, "unclosed comment" );
	      fi;

	      tokens::eof(pos,pos);
	     };

fun add_string (charlist,s: String) = charlist := s ! *charlist;
fun make_string charlist = (implode(reverse *charlist) before charlist := NIL);

fun make_hex_int (s,a,b,err_warn: Err_Warn)
    =
   (  case (string_conversion::scan_string
               (large_int::scan  string_conversion::HEX)
               s
      )

	   THE i =>  i;
	   _     =>  { err_warn.err(a,b,"trouble in parsing int");int::to_large(0);};
      esac
      except OVERFLOW =  {   err_warn.err(a,b,"large int const");
                             int::to_large(0);
                         }
   );

fun make_hex_char (args as (s, a, b, err_warn: Err_Warn)): Int /* returns a character sized integer */
    = 
	{ i = make_hex_int args;

	  if (i>255)  
	       err_warn.warn (a,b,"overflow in hexadecimal escape sequence");
	       integer::to_int(i % 256);
	  else
       	    integer::to_int i;
          fi;
        };

fun make_oct_int (s,a,b,err_warn: Err_Warn)
    =
    (   case (string_conversion::scan_string
                 (large_int::scan string_conversion::OCT)
                 s
        )

		 THE i => i;

		 _ => {   err_warn.err(a,b,"trouble in parsing int");
                           int::to_large(0);
                       };
        esac
	except
            OVERFLOW
                =
                {   err_warn.err(a,b,"large int const");
                    int::to_large(0);
                }
   );


fun make_oct_char (args as (s, a, b, err_warn: Err_Warn)) /* returns a character sized integer */
    = 
	{ i = make_oct_int args;

	  if (i>255)  
	       err_warn.warn (a,b,"overflow in octal escape sequence");
	       integer::to_int(i % 256);
	  else
       	    integer::to_int i;
          fi;
        };

fun make_int (s,a,b,err_warn: Err_Warn)
    =
    (   case (string_conversion::scan_string
                 (large_int::scan string_conversion::DEC)
                 s
        )

	     THE i => i;

	      _ => {   err_warn.err(a,b,"trouble in parsing int");
                       int::to_large(0);
                   };
        esac 
	except
            OVERFLOW
                =
                {   err_warn.err(a,b,"large int const");
                    int::to_large(0);
                }
    );

fun make_real_num (s,a,b,err_warn: Err_Warn)
    =
    (   case (string_conversion::scan_string
                 float::scan
                 s
        )

	     THE r => r;

	     _ => {   err_warn.err(a,b,"trouble in parsing real");
		      0.0;
		  };
        esac
	except
            OVERFLOW
                =
                {   err_warn.err(a,b,"large real const");
                    0.0;
                }
    );

backslasha = 7;

fun special_char (c,fst,last,err_warn: Err_Warn)
    =
    case c
	"\\a" => 7;
        "\\b" => 8;
        "\\f" => 12;
        "\\n" => 10;
        "\\r" => 13;
        "\\t" => 9;
        "\\v" => 11;
        _ => ordof(c,1);
	      /* strictly speaking, should only handle
		\?, \\, \", \', but it is common
		to simply ignore slash, and just use next char */
    esac;



/* Notes on lexer states:
   initial -- predefined start state and the default token state
   S -- inside a string (entered from INTITAL with ")
   C -- inside a comment (entered from initial with /-* )
 */


%%

%header (generic package clex_g(package tokens: Ckit_Tokens; 
			 package tok_table : Token_Table;
			 sharing tok_table::tokens == tokens;));

%arg ({ comment_nesting_depth,err_warn,source_map,charlist,stringstart });
%s ccc sss; 


id	= [_A-Za-z][_A-Za-z0-9]*; 
octdigit	= [0-7];
hexdigit	= [0-9a-fA-F];
hexnum	= 0[xX]{hexdigit}+[uUlL]?[uUlL]?; 
octnum	= 0{octdigit}+[uUlL]?[uUlL]?;
decnum	= (0|([1-9][0-9]*))[uUlL]?[uUlL]?;
realnum = (([0-9]+(\.[0-9]+)?)|(\.[0-9]+))([eE][+-]?[0-9]+)?[lL]?;
ws	= ("\012"|[\t\ ])*;

simplecharconst  = '[^\n\\]';
escapecharconst  = '\\[^\n]';

directive = #(.)*\n;

%%

<initial,ccc>^{ws}{directive}     => (source_map::parse_directive source_map 
                         (yypos,yytext); continue());
<initial,ccc>\n		=> (source_map::newline source_map yypos; continue());
<initial,ccc>{ws}		=> (continue()); 


<initial>"/*"		=> (yybegin ccc; continue());
<ccc>"*/"	 	=> (yybegin initial; continue());
<ccc>.		=> (continue());


<initial>\"		=> (charlist := [""]; stringstart := yypos; yybegin sss; continue());
<sss>\"	        => (yybegin initial;tokens::string_constant(make_string charlist,*stringstart,yypos+1));
<sss>\n		=> (err_warn.err (*stringstart,yypos,"unclosed string");
		    source_map::newline source_map yypos;
		    yybegin initial; tokens::string_constant(make_string charlist,*stringstart,yypos));
<sss>[^"\\\n]*	=> (add_string(charlist,yytext); continue());
<sss>\\\n	       	=> (source_map::newline source_map yypos; continue());
<sss>\\0		 => (add_string(charlist,chr 0);continue());
<sss>\\{octdigit}{3} => (add_string(charlist, chr(make_oct_char(substring(yytext, 1, size(yytext) - 1), yypos, yypos+size(yytext), err_warn))); continue());
<sss>\\x{hexdigit}+ => (add_string(charlist, chr(make_hex_char(substring(yytext, 2, size(yytext) - 2), yypos, yypos+size(yytext), err_warn))); continue());
<sss>\\\^[@-_]	=> (add_string(charlist,chr(ordof(yytext,2)-ord("@"))); continue());
<sss>\\.     	=> (add_string(charlist, chr(special_char(yytext, yypos, yypos+size(yytext), err_warn))); continue());

<initial>":"		=> (tokens::colon(yypos,yypos+1));
<initial>";"		=> (tokens::semicolon(yypos,yypos+1));
<initial>"("		=> (tokens::lparen(yypos,yypos+1));
<initial>")"		=> (tokens::rparen(yypos,yypos+1));
<initial>"["		=> (tokens::lbrace(yypos,yypos+1));
<initial>"]"		=> (tokens::rbrace(yypos,yypos+1));
<initial>"{"		=> (tokens::lcurly(yypos,yypos+1));
<initial>"}"		=> (tokens::rcurly(yypos,yypos+1));
<initial>"."		=> (tokens::dot(yypos,yypos+1));
<initial>"..."	        => (tokens::elipsis(yypos,yypos+3));
<initial>","		=> (tokens::comma(yypos,yypos+1));
<initial>"*"		=> (tokens::times(yypos,yypos+1));
<initial>"!"		=> (tokens::bang(yypos,yypos+1));
<initial>"^"		=> (tokens::hat(yypos,yypos+1));
<initial>"+"		=> (tokens::plus(yypos,yypos+1));
<initial>"-"		=> (tokens::minus(yypos,yypos+1));
<initial>"++"		=> (tokens::inc(yypos,yypos+2));
<initial>"--"		=> (tokens::dec(yypos,yypos+2));
<initial>"->"		=> (tokens::arrow(yypos,yypos+1));
<initial>"/"		=> (tokens::divide(yypos,yypos+1));
<initial>"~"	        => (tokens::tilde(yypos,yypos+1));
<initial>"?"		=> (tokens::question(yypos,yypos+1));
<initial>"|"		=> (tokens::bar(yypos,yypos+1));
<initial>"&"		=> (tokens::amp(yypos,yypos+1));
<initial>"%"		=> (tokens::percent(yypos,yypos+1));
<initial>"<="		=> (tokens::lte(yypos,yypos+2));
<initial>">="		=> (tokens::gte(yypos,yypos+2));
<initial>"=="		=> (tokens::eq(yypos,yypos+2));
<initial>"="	        => (tokens::equals(yypos,yypos+1));
<initial>"+="		=> (tokens::plusequals(yypos,yypos+2));
<initial>"-="		=> (tokens::minusequals(yypos,yypos+2));
<initial>"^="		=> (tokens::xorequals(yypos,yypos+2));
<initial>"%="		=> (tokens::modequals(yypos,yypos+2));
<initial>"*="		=> (tokens::timesequals(yypos,yypos+2));
<initial>"/="		=> (tokens::divequals(yypos,yypos+2));
<initial>"|="		=> (tokens::orequals(yypos,yypos+2));
<initial>"&="		=> (tokens::andequals(yypos,yypos+2));
<initial>"<<="	        => (tokens::lshiftequals(yypos,yypos+3));
<initial>">>="	        => (tokens::rshiftequals(yypos,yypos+3));
<initial>"<"		=> (tokens::lt(yypos,yypos+1));
<initial>">"		=> (tokens::gt(yypos,yypos+1));
<initial>"!="		=> (tokens::neq(yypos,yypos+2));
<initial>"||"		=> (tokens::or_t(yypos,yypos+2));
<initial>"&&"		=> (tokens::and_t(yypos,yypos+2));
<initial>"<<"		=> (tokens::lshift(yypos,yypos+2));
<initial>">>"		=> (tokens::rshift(yypos,yypos+2));

<initial>{octnum}	=> (tokens::decnum(make_oct_int(yytext,yypos,yypos+size(yytext),err_warn),yypos, yypos+size(yytext)));
<initial>{hexnum}	=> (tokens::decnum(make_hex_int(yytext,yypos,yypos+size(yytext),err_warn),yypos, yypos+size(yytext)));
<initial>{decnum}	=> (tokens::decnum(make_int (yytext,yypos,yypos+size(yytext),err_warn), yypos,yypos+size(yytext)));
<initial>{realnum}      =>
(tokens::realnum(make_real_num(yytext,yypos,yypos+size(yytext),err_warn), yypos, yypos
+ size(yytext)));

<initial>"'\\"{octdigit}{1,3}"'"	=> ({ s = substring(yytext, 2, size(yytext) - 3);
				              tokens::cconst(integer::from_int (make_oct_char(s,yypos,yypos+size(yytext),err_warn)),
						      yypos,
					      yypos+size(yytext));
                                            });

<initial>"'\\x"{hexdigit}+"'"	=>  ({ s = substring(yytext, 3, size(yytext) - 4);
				      tokens::cconst(integer::from_int (make_hex_char(s,yypos,yypos+size(yytext),err_warn)),
						      yypos,
						      yypos+size(yytext));
	                             });


<initial>{simplecharconst}	=> ({ cval = ordof(yytext,1);
	                              tokens::cconst(int::to_large cval,yypos,yypos+size(yytext));
                                    });
<initial>{escapecharconst} => (tokens::cconst(integer::from_int(special_char(substring(yytext,1,size(yytext) - 2),yypos,yypos+size(yytext),err_warn)), yypos, yypos+size(yytext)));
<initial>{id}        	=> (tok_table::check_token(yytext,yypos));
<initial>.        	=> (continue());

