generic burg_lr_vals_fun(package token:  Token;)		# Token		is from   src/app/yacc/lib/base.api
 : api package parser_data : Parser_Data; 			# Parser_Data	is from   src/app/yacc/lib/base.api
       package tokens : Burg_Tokens; 				# Burg_Tokens	is from   src/app/burg/burg-gram.sig
   end
 = 
pkg
package parser_data=
pkg
package header = 
pkg
# burg-gram
#
# Grammar for BURG.



package a=   burg_ast;	# burg_ast	is from   src/app/burg/burg-ast.pkg

fun output_raw s
    =
    print (s: String);


end;
package lr_table = token::lr_table;
package token = token;
stipulate include lr_table; herein 
my table={   action_rows =
"\
\\001\000\001\000\000\000\000\000\
\\001\000\002\000\010\000\003\000\009\000\004\000\008\000\005\000\007\000\
\\006\000\006\000\014\000\005\000\000\000\
\\001\000\007\000\024\000\000\000\
\\001\000\008\000\038\000\000\000\
\\001\000\011\000\040\000\000\000\
\\001\000\011\000\045\000\000\000\
\\001\000\012\000\029\000\000\000\
\\001\000\014\000\021\000\016\000\020\000\000\000\
\\001\000\015\000\039\000\000\000\
\\001\000\015\000\046\000\000\000\
\\001\000\016\000\012\000\000\000\
\\001\000\016\000\013\000\000\000\
\\001\000\016\000\014\000\000\000\
\\001\000\016\000\015\000\000\000\
\\001\000\016\000\018\000\000\000\
\\001\000\016\000\026\000\000\000\
\\001\000\016\000\028\000\000\000\
\\001\000\016\000\032\000\000\000\
\\049\000\000\000\
\\050\000\000\000\
\\051\000\000\000\
\\052\000\013\000\022\000\000\000\
\\053\000\000\000\
\\054\000\000\000\
\\055\000\000\000\
\\056\000\000\000\
\\057\000\000\000\
\\058\000\000\000\
\\059\000\012\000\023\000\000\000\
\\060\000\000\000\
\\061\000\000\000\
\\062\000\000\000\
\\063\000\000\000\
\\064\000\000\000\
\\065\000\010\000\030\000\000\000\
\\066\000\000\000\
\\067\000\009\000\037\000\000\000\
\\068\000\000\000\
\\069\000\010\000\035\000\000\000\
\\070\000\000\000\
\\071\000\009\000\043\000\000\000\
\\072\000\000\000\
\";
    action_row_numbers =
"\019\000\001\000\020\000\030\000\
\\010\000\011\000\012\000\013\000\
\\014\000\007\000\025\000\024\000\
\\023\000\022\000\021\000\026\000\
\\028\000\031\000\002\000\018\000\
\\014\000\015\000\016\000\027\000\
\\029\000\006\000\034\000\017\000\
\\016\000\038\000\033\000\036\000\
\\003\000\008\000\004\000\016\000\
\\032\000\040\000\035\000\036\000\
\\005\000\009\000\037\000\039\000\
\\040\000\041\000\000\000";
   goto_table =
"\
\\001\000\046\000\010\000\001\000\000\000\
\\003\000\002\000\000\000\
\\000\000\
\\011\000\009\000\000\000\
\\000\000\
\\000\000\
\\000\000\
\\000\000\
\\004\000\015\000\013\000\014\000\000\000\
\\012\000\017\000\000\000\
\\000\000\
\\000\000\
\\000\000\
\\000\000\
\\000\000\
\\000\000\
\\000\000\
\\000\000\
\\000\000\
\\000\000\
\\004\000\023\000\000\000\
\\000\000\
\\008\000\025\000\000\000\
\\000\000\
\\000\000\
\\000\000\
\\000\000\
\\007\000\029\000\000\000\
\\008\000\031\000\000\000\
\\005\000\032\000\000\000\
\\000\000\
\\009\000\034\000\000\000\
\\000\000\
\\000\000\
\\000\000\
\\008\000\039\000\000\000\
\\000\000\
\\006\000\040\000\000\000\
\\000\000\
\\009\000\042\000\000\000\
\\000\000\
\\000\000\
\\000\000\
\\000\000\
\\006\000\045\000\000\000\
\\000\000\
\\000\000\
\";
   numstates = 47;
   numrules = 24;
 s = REF "";  index = REF 0;
    string_to_int = fn () = 
    {    i = *index;
         index := i+2;
         char::to_int (string::get(*s, i)) + char::to_int (string::get(*s, i+1)) * 256;
    };

    string_to_list = fn s' =
    {   len = string::length s';
        fun f () =
           if *index < len then
           string_to_int() . f();
           else NIL; fi;
        index := 0;
        s := s';
        f ();
   };

   string_to_pairlist =   fn (conv_key, conv_entry) =   f
   where 
         fun f () =
             case string_to_int ()
             of
                 0 => EMPTY;
                 n => PAIR (conv_key (n - 1), conv_entry (string_to_int()), f());
             esac;
   end;

   string_to_pairlist_default =   fn (conv_key, conv_entry) =
    {   conv_row = string_to_pairlist (conv_key, conv_entry);
       fn () =
       {   default = conv_entry (string_to_int());
           row = conv_row();
          (row, default);
       };
   };

    string_to_table = fn (convert_row, s') =
    {   len = string::length s';
        fun f ()=
           if *index < len then convert_row() . f();
           else NIL; fi;
        s := s';
        index := 0;
        f ();
     };

stipulate
  memo = rw_vector::make_rw_vector (numstates+numrules, ERROR);
  my _ ={   fun g i
                =
                {   rw_vector::set (memo, i, REDUCE (i-numstates));
                    g (i+1);
                };

            fun f i
                =
                if   i == numstates
                then g i;
                else    rw_vector::set (memo, i, SHIFT (STATE i));
                         f (i+1);
                fi;

            f 0
            except
                (SUBSCRIPT|INDEX_OUT_OF_BOUNDS) =  ();
        };
herein
    entry_to_action =   fn 0 => ACCEPT;
                           1 => ERROR;
                           j => rw_vector::get (memo, (j - 2));
                         end;
end;

   goto_table = rw_vector::from_list (string_to_table (string_to_pairlist (NONTERM, STATE), goto_table));
   action_rows = string_to_table (string_to_pairlist_default (TERM, entry_to_action), action_rows);
   action_row_numbers = string_to_list action_row_numbers;
   action_table
    =
    {   action_row_lookup
            =
            {   a=rw_vector::from_list (action_rows);

                fn i =   rw_vector::get (a, i);
            };

        rw_vector::from_list (map action_row_lookup action_row_numbers);
    };

    lr_table::make_lr_table {
        actions => action_table,
        gotos   => goto_table,
        rule_count   => numrules,
        state_count  => numstates,
        initial_state => STATE 0   };
};
end;
stipulate include header; herein
Source_Position = Int;
Arg = Void;
package values = 
pkg
Semantic_Value = TM_VOID | NT_VOID  Void | RAW  (List String ) | ID  (String) | INT  (Int) | PPERCENT  (List String ) | QQ_POSTLUDE  (Void) | QQ_PRELUDE  (Void) | QQ_RAW  (Void)
 | QQ_BINDINGLIST  (List ((String, Null_Or String)) ) | QQ_RULE  (a::Rule_Ast) | QQ_RULES  (List a::Rule_Ast ) | QQ_DECLS  (List a::Decl_Ast ) | QQ_PATTERNTAIL  (List a::Pattern_Ast )
 | QQ_PATTERN  (a::Pattern_Ast) | QQ_RULENAME  (String) | QQ_COSTTAIL  (List Int ) | QQ_COST  (List Int ) | QQ_BINDING  ((String, Null_Or String)) | QQ_DECL  (a::Decl_Ast) | QQ_SPEC  (a::Spec_Ast)
 | QQ_FULL  (a::Spec_Ast);
end;
Semantic_Value = values::Semantic_Value;
Result = a::Spec_Ast;
end;
package error_recovery=
pkg
include lr_table;
infix 60 @@;
fun x @@ y = y . x;
is_keyword =
fn _ => FALSE; end;
my preferred_change:   List( (List( Terminal ), List( Terminal )) ) = 
NIL;
no_shift = 
fn _ => FALSE; end;
show_terminal =
fn (TERM 0) => "K_EOF"
; (TERM 1) => "K_TERM"
; (TERM 2) => "K_START"
; (TERM 3) => "K_TERMPREFIX"
; (TERM 4) => "K_RULEPREFIX"
; (TERM 5) => "K_SIG"
; (TERM 6) => "K_COLON"
; (TERM 7) => "K_SEMICOLON"
; (TERM 8) => "K_COMMA"
; (TERM 9) => "K_LPAREN"
; (TERM 10) => "K_RPAREN"
; (TERM 11) => "K_EQUAL"
; (TERM 12) => "K_PIPE"
; (TERM 13) => "PPERCENT"
; (TERM 14) => "INT"
; (TERM 15) => "ID"
; (TERM 16) => "RAW"
; _ => "bogus-term"; end;
stipulate include header; herein
errtermvalue=
fn _ => values::TM_VOID;
 end; end;
my terms:  List( Terminal ) = NIL
 @@ (TERM 12) @@ (TERM 11) @@ (TERM 10) @@ (TERM 9) @@ (TERM 8) @@ (TERM 7) @@ (TERM 6) @@ (TERM 5) @@ (TERM 4) @@ (TERM 3) @@ (TERM 2) @@ (TERM 1) @@ (TERM 0);
end;
package actions =
pkg 
exception MLY_ACTION Int;
stipulate include header; herein
actions = 
fn (i392, default_position, stack, 
    (()): Arg) = 
case (i392, stack)
of  ( 0,  ( ( _,  ( values::PPERCENT ppercent2,  _,  ppercent2right)) .  ( _,  ( values::QQ_RULES rules,  _,  _)) .  ( _,  ( values::PPERCENT ppercent1,  _,  _)) .  ( _,  ( values::QQ_DECLS decls,  
decls1left,  _)) .  rest671)) => {  my  result = values::QQ_FULL (a::SPEC{ head => ppercent1,
						decls => reverse decls,
						rules => reverse rules,
						tail  => ppercent2});
 ( 
lr_table::NONTERM 0,  ( result,  decls1left,  ppercent2right),  rest671);
 } 
;  ( 1,  ( rest671)) => {  my  result = values::QQ_DECLS ([]);
 ( lr_table::NONTERM 9,  ( result,  default_position,  default_position),  rest671);
 } 
;  ( 2,  ( ( _,  ( values::QQ_DECL decl,  _,  decl1right)) .  ( _,  ( values::QQ_DECLS decls,  decls1left,  _)) .  rest671)) => {  my  result = values::QQ_DECLS (decl . decls);
 ( lr_table::NONTERM 9
,  ( result,  decls1left,  decl1right),  rest671);
 } 
;  ( 3,  ( ( _,  ( values::QQ_BINDINGLIST bindinglist,  _,  bindinglist1right)) .  ( _,  ( _,  k_term1left,  _)) .  rest671)) => {  my  result = values::QQ_DECL (a::TERM (reverse bindinglist));
 ( 
lr_table::NONTERM 2,  ( result,  k_term1left,  bindinglist1right),  rest671);
 } 
;  ( 4,  ( ( _,  ( values::ID id,  _,  id1right)) .  ( _,  ( _,  k_start1left,  _)) .  rest671)) => {  my  result = values::QQ_DECL (a::START id);
 ( lr_table::NONTERM 2,  ( result,  k_start1left,  
id1right),  rest671);
 } 
;  ( 5,  ( ( _,  ( values::ID id,  _,  id1right)) .  ( _,  ( _,  k_termprefix1left,  _)) .  rest671)) => {  my  result = values::QQ_DECL (a::TERMPREFIX id);
 ( lr_table::NONTERM 2,  ( result,  
k_termprefix1left,  id1right),  rest671);
 } 
;  ( 6,  ( ( _,  ( values::ID id,  _,  id1right)) .  ( _,  ( _,  k_ruleprefix1left,  _)) .  rest671)) => {  my  result = values::QQ_DECL (a::RULEPREFIX id);
 ( lr_table::NONTERM 2,  ( result,  
k_ruleprefix1left,  id1right),  rest671);
 } 
;  ( 7,  ( ( _,  ( values::ID id,  _,  id1right)) .  ( _,  ( _,  k_sig1left,  _)) .  rest671)) => {  my  result = values::QQ_DECL (a::BEGIN_API  id);
 ( lr_table::NONTERM 2,  ( result,  k_sig1left,  
id1right),  rest671);
 } 
;  ( 8,  ( ( _,  ( values::QQ_BINDING binding,  binding1left,  binding1right)) .  rest671)) => {  my  result = values::QQ_BINDINGLIST ([binding]);
 ( lr_table::NONTERM 12,  ( result,  binding1left,  
binding1right),  rest671);
 } 
;  ( 9,  ( ( _,  ( values::QQ_BINDING binding,  _,  binding1right)) .  _ .  ( _,  ( values::QQ_BINDINGLIST bindinglist,  bindinglist1left,  _)) .  rest671)) => {  my  result = values::QQ_BINDINGLIST (
binding . bindinglist);
 ( lr_table::NONTERM 12,  ( result,  bindinglist1left,  binding1right),  rest671);
 } 
;  ( 10,  ( ( _,  ( values::ID id,  id1left,  id1right)) .  rest671)) => {  my  result = values::QQ_BINDING ((id, NULL));
 ( lr_table::NONTERM 3,  ( result,  id1left,  id1right),  rest671);
 } 
;  ( 11,  ( ( _,  ( values::ID id2,  _,  id2right)) .  _ .  ( _,  ( values::ID id1,  id1left,  _)) .  rest671)) => {  my  result = values::QQ_BINDING ((id1, THE id2));
 ( lr_table::NONTERM 3,  ( 
result,  id1left,  id2right),  rest671);
 } 
;  ( 12,  ( rest671)) => {  my  result = values::QQ_RULES ([]);
 ( lr_table::NONTERM 10,  ( result,  default_position,  default_position),  rest671);
 } 
;  ( 13,  ( ( _,  ( values::QQ_RULE rule,  _,  rule1right)) .  ( _,  ( values::QQ_RULES rules,  rules1left,  _)) .  rest671)) => {  my  result = values::QQ_RULES (rule . rules);
 ( lr_table::NONTERM 
10,  ( result,  rules1left,  rule1right),  rest671);
 } 
;  ( 14,  ( ( _,  ( _,  _,  k_semicolon1right)) .  ( _,  ( values::QQ_COST cost,  _,  _)) .  ( _,  ( values::QQ_RULENAME rulename,  _,  _)) .  _ .  ( _,  ( values::QQ_PATTERN pattern,  _,  _)) .  _ . 
 ( _,  ( values::ID id,  id1left,  _)) .  rest671)) => {  my  result = values::QQ_RULE (a::RULE(id, pattern, rulename, cost));
 ( lr_table::NONTERM 11,  ( result,  id1left,  k_semicolon1right),  
rest671);
 } 
;  ( 15,  ( ( _,  ( values::ID id,  id1left,  id1right)) .  rest671)) => {  my  result = values::QQ_RULENAME (id);
 ( lr_table::NONTERM 6,  ( result,  id1left,  id1right),  rest671);
 } 
;  ( 16,  ( ( _,  ( values::ID id,  id1left,  id1right)) .  rest671)) => {  my  result = values::QQ_PATTERN (a::PAT(id, []));
 ( lr_table::NONTERM 7,  ( result,  id1left,  id1right),  rest671);
 } 
;  ( 17,  ( ( _,  ( _,  _,  k_rparen1right)) .  ( _,  ( values::QQ_PATTERNTAIL patterntail,  _,  _)) .  ( _,  ( values::QQ_PATTERN pattern,  _,  _)) .  _ .  ( _,  ( values::ID id,  id1left,  _)) .  
rest671)) => {  my  result = values::QQ_PATTERN (a::PAT(id, pattern . patterntail));
 ( lr_table::NONTERM 7,  ( result,  id1left,  k_rparen1right),  rest671);
 } 
;  ( 18,  ( rest671)) => {  my  result = values::QQ_PATTERNTAIL ([]);
 ( lr_table::NONTERM 8,  ( result,  default_position,  default_position),  rest671);
 } 
;  ( 19,  ( ( _,  ( values::QQ_PATTERNTAIL patterntail,  _,  patterntail1right)) .  ( _,  ( values::QQ_PATTERN pattern,  _,  _)) .  ( _,  ( _,  k_comma1left,  _)) .  rest671)) => {  my  result = 
values::QQ_PATTERNTAIL (pattern . patterntail);
 ( lr_table::NONTERM 8,  ( result,  k_comma1left,  patterntail1right),  rest671);
 } 
;  ( 20,  ( rest671)) => {  my  result = values::QQ_COST ([]);
 ( lr_table::NONTERM 4,  ( result,  default_position,  default_position),  rest671);
 } 
;  ( 21,  ( ( _,  ( _,  _,  k_rparen1right)) .  ( _,  ( values::QQ_COSTTAIL costtail,  _,  _)) .  ( _,  ( values::INT int,  _,  _)) .  ( _,  ( _,  k_lparen1left,  _)) .  rest671)) => {  my  result = 
values::QQ_COST (int . costtail);
 ( lr_table::NONTERM 4,  ( result,  k_lparen1left,  k_rparen1right),  rest671);
 } 
;  ( 22,  ( rest671)) => {  my  result = values::QQ_COSTTAIL ([]);
 ( lr_table::NONTERM 5,  ( result,  default_position,  default_position),  rest671);
 } 
;  ( 23,  ( ( _,  ( values::QQ_COSTTAIL costtail,  _,  costtail1right)) .  ( _,  ( values::INT int,  _,  _)) .  ( _,  ( _,  k_comma1left,  _)) .  rest671)) => {  my  result = values::QQ_COSTTAIL (
int . costtail);
 ( lr_table::NONTERM 5,  ( result,  k_comma1left,  costtail1right),  rest671);
 } 
; _ => raise exception (MLY_ACTION i392); esac;
end;
void = values::TM_VOID;
extract = fn a = (fn values::QQ_FULL x => x;
 _ => { exception PARSE_INTERNAL;
	 raise exception PARSE_INTERNAL; }; end ) a ;
end;
end;
package tokens : Burg_Tokens		# Burg_Tokens	is from   src/app/burg/burg-gram.sig
=
pkg
Semantic_Value = parser_data::Semantic_Value;
Token (X,Y) = token::Token(X,Y);
fun k_eof (p1, p2) = token::TOKEN (parser_data::lr_table::TERM 0, (parser_data::values::TM_VOID, p1, p2));
fun k_term (p1, p2) = token::TOKEN (parser_data::lr_table::TERM 1, (parser_data::values::TM_VOID, p1, p2));
fun k_start (p1, p2) = token::TOKEN (parser_data::lr_table::TERM 2, (parser_data::values::TM_VOID, p1, p2));
fun k_termprefix (p1, p2) = token::TOKEN (parser_data::lr_table::TERM 3, (parser_data::values::TM_VOID, p1, p2));
fun k_ruleprefix (p1, p2) = token::TOKEN (parser_data::lr_table::TERM 4, (parser_data::values::TM_VOID, p1, p2));
fun k_sig (p1, p2) = token::TOKEN (parser_data::lr_table::TERM 5, (parser_data::values::TM_VOID, p1, p2));
fun k_colon (p1, p2) = token::TOKEN (parser_data::lr_table::TERM 6, (parser_data::values::TM_VOID, p1, p2));
fun k_semicolon (p1, p2) = token::TOKEN (parser_data::lr_table::TERM 7, (parser_data::values::TM_VOID, p1, p2));
fun k_comma (p1, p2) = token::TOKEN (parser_data::lr_table::TERM 8, (parser_data::values::TM_VOID, p1, p2));
fun k_lparen (p1, p2) = token::TOKEN (parser_data::lr_table::TERM 9, (parser_data::values::TM_VOID, p1, p2));
fun k_rparen (p1, p2) = token::TOKEN (parser_data::lr_table::TERM 10, (parser_data::values::TM_VOID, p1, p2));
fun k_equal (p1, p2) = token::TOKEN (parser_data::lr_table::TERM 11, (parser_data::values::TM_VOID, p1, p2));
fun k_pipe (p1, p2) = token::TOKEN (parser_data::lr_table::TERM 12, (parser_data::values::TM_VOID, p1, p2));
fun ppercent (i, p1, p2) = token::TOKEN (parser_data::lr_table::TERM 13, (parser_data::values::PPERCENT i, p1, p2));
fun int (i, p1, p2) = token::TOKEN (parser_data::lr_table::TERM 14, (parser_data::values::INT i, p1, p2));
fun id (i, p1, p2) = token::TOKEN (parser_data::lr_table::TERM 15, (parser_data::values::ID i, p1, p2));
fun raw (i, p1, p2) = token::TOKEN (parser_data::lr_table::TERM 16, (parser_data::values::RAW i, p1, p2));
end;
end;
