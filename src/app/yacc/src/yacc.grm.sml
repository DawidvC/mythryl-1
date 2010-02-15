
generic package mlyacc_lr_vals_g (package header : Header
		          where Precedence == header::Precedence;
			package token : Token;)
 { 
package parser_data{
package header { 
#  Mythryl-Yacc Parser Generator (c) 1989 Andrew W. Appel, David R. Tarditi 

#  parser for the ML parser generator 



###             "If we don't change direction soon,
###              we'll end up where we're going."
###
###                         -- Professor Irwin Corey



include header;

};
package lr_table = token::lr_table;
package token = token;
stipulate include lr_table; herein 
my table={   action_rows =
"\
\\001\000\001\000\076\000\000\000\
\\001\000\005\000\025\000\008\000\024\000\014\000\023\000\016\000\022\000\
\\019\000\021\000\020\000\020\000\021\000\019\000\022\000\018\000\
\\024\000\017\000\025\000\016\000\026\000\015\000\027\000\014\000\
\\028\000\013\000\029\000\012\000\031\000\011\000\035\000\010\000\
\\036\000\009\000\037\000\008\000\039\000\007\000\040\000\006\000\000\000\
\\001\000\006\000\063\000\000\000\
\\001\000\006\000\074\000\000\000\
\\001\000\006\000\086\000\000\000\
\\001\000\006\000\098\000\000\000\
\\001\000\007\000\085\000\033\000\084\000\000\000\
\\001\000\009\000\000\000\000\000\
\\001\000\010\000\061\000\000\000\
\\001\000\011\000\003\000\000\000\
\\001\000\012\000\026\000\000\000\
\\001\000\012\000\028\000\000\000\
\\001\000\012\000\029\000\000\000\
\\001\000\012\000\032\000\000\000\
\\001\000\012\000\044\000\013\000\043\000\000\000\
\\001\000\012\000\044\000\013\000\043\000\017\000\042\000\032\000\041\000\
\\038\000\040\000\000\000\
\\001\000\012\000\048\000\000\000\
\\001\000\012\000\053\000\000\000\
\\001\000\012\000\071\000\015\000\070\000\000\000\
\\001\000\012\000\071\000\015\000\070\000\033\000\069\000\000\000\
\\001\000\012\000\077\000\000\000\
\\001\000\012\000\080\000\000\000\
\\001\000\012\000\101\000\000\000\
\\001\000\032\000\036\000\000\000\
\\001\000\032\000\037\000\000\000\
\\001\000\032\000\050\000\000\000\
\\001\000\032\000\057\000\000\000\
\\001\000\032\000\100\000\000\000\
\\001\000\032\000\104\000\000\000\
\\106\000\012\000\053\000\000\000\
\\107\000\000\000\
\\108\000\000\000\
\\109\000\004\000\058\000\000\000\
\\110\000\004\000\058\000\000\000\
\\111\000\000\000\
\\112\000\000\000\
\\113\000\000\000\
\\114\000\000\000\
\\115\000\000\000\
\\116\000\000\000\
\\117\000\000\000\
\\118\000\000\000\
\\119\000\000\000\
\\120\000\000\000\
\\121\000\000\000\
\\122\000\001\000\065\000\002\000\064\000\000\000\
\\123\000\000\000\
\\124\000\000\000\
\\125\000\000\000\
\\126\000\001\000\065\000\002\000\064\000\000\000\
\\127\000\000\000\
\\128\000\000\000\
\\129\000\004\000\075\000\000\000\
\\130\000\000\000\
\\131\000\000\000\
\\132\000\004\000\060\000\000\000\
\\133\000\000\000\
\\134\000\001\000\065\000\002\000\064\000\000\000\
\\135\000\023\000\091\000\000\000\
\\136\000\001\000\065\000\002\000\064\000\000\000\
\\137\000\023\000\059\000\000\000\
\\138\000\004\000\094\000\000\000\
\\139\000\000\000\
\\140\000\000\000\
\\141\000\000\000\
\\142\000\012\000\034\000\000\000\
\\143\000\000\000\
\\144\000\000\000\
\\145\000\000\000\
\\146\000\000\000\
\\147\000\000\000\
\\148\000\000\000\
\\149\000\001\000\065\000\002\000\064\000\000\000\
\\150\000\012\000\044\000\013\000\043\000\017\000\042\000\032\000\041\000\
\\038\000\040\000\000\000\
\\151\000\000\000\
\\152\000\001\000\065\000\002\000\064\000\000\000\
\\153\000\001\000\065\000\002\000\064\000\000\000\
\\154\000\001\000\065\000\002\000\064\000\000\000\
\\155\000\000\000\
\\156\000\000\000\
\\157\000\000\000\
\\158\000\000\000\
\\159\000\000\000\
\\160\000\030\000\096\000\000\000\
\";
    action_row_numbers =
"\009\000\031\000\001\000\030\000\
\\010\000\046\000\011\000\012\000\
\\013\000\065\000\065\000\023\000\
\\024\000\015\000\048\000\065\000\
\\065\000\011\000\047\000\016\000\
\\065\000\025\000\017\000\065\000\
\\026\000\032\000\060\000\035\000\
\\055\000\040\000\008\000\038\000\
\\065\000\034\000\043\000\002\000\
\\049\000\073\000\068\000\071\000\
\\019\000\014\000\078\000\036\000\
\\041\000\033\000\044\000\037\000\
\\042\000\029\000\063\000\003\000\
\\052\000\039\000\000\000\050\000\
\\020\000\015\000\013\000\021\000\
\\064\000\015\000\015\000\015\000\
\\072\000\006\000\004\000\070\000\
\\081\000\080\000\079\000\062\000\
\\065\000\065\000\065\000\058\000\
\\059\000\054\000\056\000\045\000\
\\074\000\075\000\069\000\018\000\
\\015\000\061\000\083\000\051\000\
\\053\000\015\000\005\000\077\000\
\\065\000\027\000\022\000\057\000\
\\015\000\083\000\066\000\082\000\
\\076\000\028\000\067\000\007\000";
   goto_table =
"\
\\001\000\103\000\000\000\
\\006\000\002\000\000\000\
\\005\000\003\000\000\000\
\\000\000\
\\000\000\
\\000\000\
\\002\000\025\000\000\000\
\\000\000\
\\013\000\029\000\014\000\028\000\000\000\
\\003\000\031\000\000\000\
\\003\000\033\000\000\000\
\\000\000\
\\000\000\
\\007\000\037\000\017\000\036\000\000\000\
\\000\000\
\\003\000\043\000\000\000\
\\003\000\044\000\000\000\
\\002\000\045\000\000\000\
\\000\000\
\\000\000\
\\003\000\047\000\000\000\
\\000\000\
\\010\000\050\000\011\000\049\000\000\000\
\\003\000\054\000\015\000\053\000\016\000\052\000\000\000\
\\000\000\
\\000\000\
\\000\000\
\\000\000\
\\000\000\
\\000\000\
\\000\000\
\\000\000\
\\003\000\060\000\000\000\
\\000\000\
\\000\000\
\\000\000\
\\000\000\
\\007\000\037\000\017\000\064\000\000\000\
\\000\000\
\\000\000\
\\004\000\066\000\008\000\065\000\000\000\
\\007\000\070\000\000\000\
\\000\000\
\\000\000\
\\000\000\
\\000\000\
\\000\000\
\\000\000\
\\000\000\
\\010\000\071\000\000\000\
\\000\000\
\\000\000\
\\000\000\
\\000\000\
\\000\000\
\\000\000\
\\000\000\
\\007\000\037\000\017\000\076\000\000\000\
\\013\000\077\000\014\000\028\000\000\000\
\\000\000\
\\000\000\
\\007\000\037\000\017\000\079\000\000\000\
\\007\000\037\000\017\000\080\000\000\000\
\\007\000\037\000\017\000\081\000\000\000\
\\000\000\
\\000\000\
\\000\000\
\\000\000\
\\000\000\
\\000\000\
\\000\000\
\\000\000\
\\003\000\086\000\009\000\085\000\000\000\
\\003\000\054\000\015\000\087\000\016\000\052\000\000\000\
\\003\000\088\000\000\000\
\\000\000\
\\000\000\
\\000\000\
\\000\000\
\\000\000\
\\000\000\
\\000\000\
\\000\000\
\\004\000\090\000\000\000\
\\007\000\037\000\017\000\091\000\000\000\
\\000\000\
\\012\000\093\000\000\000\
\\000\000\
\\000\000\
\\007\000\037\000\017\000\095\000\000\000\
\\000\000\
\\000\000\
\\003\000\097\000\000\000\
\\000\000\
\\000\000\
\\000\000\
\\007\000\037\000\017\000\100\000\000\000\
\\012\000\101\000\000\000\
\\000\000\
\\000\000\
\\000\000\
\\000\000\
\\000\000\
\\000\000\
\";
   numstates = 104;
   numrules = 55;
 s = REF "";  index = REF 0;
    string_to_int = fn () = 
    {    i = *index;
         index := i+2;
         char::to_int (string::get(*s, i)) + char::to_int (string::get(*s, i+1)) * 256;
    };

    string_to_list = fn s' =
    {   len = string::length s';
        fun f () =
           if (*index < len)
           string_to_int() ! f();
           else NIL; fi;
        index := 0;
        s := s';
        f ();
   };

   string_to_pairlist =   fn (conv_key, conv_entry) =   f
   where 
         fun f ()
             =
             case (string_to_int ())
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
        fun f ()
            =
           if (*index < len)
              convert_row() ! f();
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
                if   (i == numstates)
                     g i;
                else    rw_vector::set (memo, i, SHIFT (STATE i));
                         f (i+1);
                fi;

            f 0
            except
                (SUBSCRIPT|INDEX_OUT_OF_BOUNDS) =  ();
        };
herein
    entry_to_action
        =
        fn 0 =>  ACCEPT;
           1 =>  ERROR;
           j =>  rw_vector::get (memo, (j - 2));
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
Arg = header::Input_Source;
package values { 
Semantic_Value = TM_VOID | NT_VOID  Void -> Void | UNKNOWN Void ->  (String) | TYVAR Void ->  (String) | PROG Void ->  (String) | PREC Void ->  (header::Precedence) | INT Void ->  (String)
 | IDDOT Void ->  (String) | ID Void ->  ((String, Int)) | HEADER Void ->  (String) | TY Void ->  (String) | CHANGE_DEC Void ->  ((( List header::Symbol, List header::Symbol)))
 | CHANGE_DECL Void ->  (List (( List header::Symbol, List header::Symbol)) ) | SUBST_DEC Void ->  ((( List header::Symbol, List header::Symbol)))
 | SUBST_DECL Void ->  (List (( List header::Symbol, List header::Symbol)) ) | G_RULE_PREC Void ->  (Null_Or header::Symbol ) | G_RULE_LIST Void ->  (List header::Rule )
 | G_RULE Void ->  (List header::Rule ) | RHS_LIST Void ->  (List { rhs: List header::Symbol , code: String, prec: Null_Or header::Symbol  } ) | RECORD_LIST Void ->  (String)
 | QUAL_ID Void ->  (String) | MPC_DECLS Void ->  (header::Decl_Data) | MPC_DECL Void ->  (header::Decl_Data) | LABEL Void ->  (String) | ID_LIST Void ->  (List header::Symbol )
 | CONSTR_LIST Void ->  (List ((header::Symbol, Null_Or header::Some_Type)) ) | BEGIN Void ->  ((String, header::Decl_Data, (List header::Rule)));
};
Semantic_Value = values::Semantic_Value;
Result = (String, header::Decl_Data, (List header::Rule));
end;
package error_recovery{
include lr_table;
infix val 60 @@;
fun x @@ y = y ! x;
is_keyword =
fn _ => FALSE; end;
my preferred_change:   List( (List( Terminal ), List( Terminal )) ) = 
NIL;
no_shift = 
fn (TERM 8) => TRUE; _ => FALSE; end;
show_terminal =
fn (TERM 0) => "ARROW"
; (TERM 1) => "ASTERISK"
; (TERM 2) => "BLOCK"
; (TERM 3) => "BAR"
; (TERM 4) => "CHANGE"
; (TERM 5) => "COLON"
; (TERM 6) => "COMMA"
; (TERM 7) => "DELIMITER"
; (TERM 8) => "EOF_T"
; (TERM 9) => "FOR_T"
; (TERM 10) => "HEADER"
; (TERM 11) => "ID"
; (TERM 12) => "IDDOT"
; (TERM 13) => "PERCENT_HEADER"
; (TERM 14) => "INT"
; (TERM 15) => "KEYWORD"
; (TERM 16) => "LBRACE"
; (TERM 17) => "LPAREN"
; (TERM 18) => "NAME"
; (TERM 19) => "NODEFAULT"
; (TERM 20) => "NONTERM"
; (TERM 21) => "NOSHIFT"
; (TERM 22) => "OF_T"
; (TERM 23) => "PERCENT_EOP"
; (TERM 24) => "PERCENT_PURE"
; (TERM 25) => "PERCENT_POS"
; (TERM 26) => "PERCENT_ARG"
; (TERM 27) => "PERCENT_TOKEN_SIG_INFO"
; (TERM 28) => "PREC"
; (TERM 29) => "PREC_TAG"
; (TERM 30) => "PREFER"
; (TERM 31) => "PROG"
; (TERM 32) => "RBRACE"
; (TERM 33) => "RPAREN"
; (TERM 34) => "SUBST"
; (TERM 35) => "START"
; (TERM 36) => "TERM"
; (TERM 37) => "TYVAR"
; (TERM 38) => "VERBOSE"
; (TERM 39) => "VALUE"
; (TERM 40) => "UNKNOWN"
; (TERM 41) => "BOGUS_VALUE"
; _ => "bogus-term"; end;
stipulate include header; herein
errtermvalue=
fn _ => values::TM_VOID;
 end; end;
my terms:  List( Terminal ) = NIL
 @@ (TERM 41) @@ (TERM 39) @@ (TERM 38) @@ (TERM 36) @@ (TERM 35) @@ (TERM 34) @@ (TERM 33) @@ (TERM 32) @@ (TERM 30) @@ (TERM 29) @@ (TERM 27) @@ (TERM 26) @@ (TERM 25) @@ (TERM 24) @@ (TERM 23) @@ 
(TERM 22) @@ (TERM 21) @@ (TERM 20) @@ (TERM 19) @@ (TERM 18) @@ (TERM 17) @@ (TERM 16) @@ (TERM 15) @@ (TERM 13) @@ (TERM 9) @@ (TERM 8) @@ (TERM 7) @@ (TERM 6) @@ (TERM 5) @@ (TERM 4) @@ (TERM 3)
 @@ (TERM 2) @@ (TERM 1) @@ (TERM 0);
};
package actions {
exception MLY_ACTION Int;
stipulate include header; herein
actions = 
fn (i392, default_position, stack, 
    (input_source): Arg) = 
case (i392, stack)
  ( 0,  ( ( _,  ( values::G_RULE_LIST g_rule_list1,  _,  g_rule_list1right)) !  _ !  ( _,  ( values::MPC_DECLS mpc_decls1,  _,  _)) !  ( _,  ( values::HEADER header1,  header1left,  _)) !  rest671))
 => {  my  result = values::BEGIN (fn  _ =  {  my  (header as header1) = header1 ();
 my  (mpc_decls as mpc_decls1) = mpc_decls1 ();
 my  (g_rule_list as g_rule_list1) = g_rule_list1 ();
 (
header, mpc_decls, reverse g_rule_list);
 } );
 ( lr_table::NONTERM 0,  ( result,  header1left,  g_rule_list1right),  rest671);
 } 
;  ( 1,  ( ( _,  ( values::MPC_DECL mpc_decl1,  mpc_declleft,  mpc_decl1right)) !  ( _,  ( values::MPC_DECLS mpc_decls1,  mpc_decls1left,  _)) !  rest671)) => {  my  result = values::MPC_DECLS (fn  _
 =  {  my  (mpc_decls as mpc_decls1) = mpc_decls1 ();
 my  (mpc_decl as mpc_decl1) = mpc_decl1 ();
 (join_decls (mpc_decls, mpc_decl, input_source, mpc_declleft));
 } );
 ( lr_table::NONTERM 5,  ( 
result,  mpc_decls1left,  mpc_decl1right),  rest671);
 } 
;  ( 2,  ( rest671)) => {  my  result = values::MPC_DECLS (fn  _ =  (DECL { prec=>NIL, nonterm=>NULL, term=>NULL, eop=>NIL, control=>NIL,
		   keyword=>NIL, change=>NIL,
		   value=>NIL } ));
 ( 
lr_table::NONTERM 5,  ( result,  default_position,  default_position),  rest671);
 } 
;  ( 3,  ( ( _,  ( values::CONSTR_LIST constr_list1,  _,  constr_list1right)) !  ( _,  ( _,  term1left,  _)) !  rest671)) => {  my  result = values::MPC_DECL (fn  _ =  {  my  (constr_list as 
constr_list1) = constr_list1 ();
 (DECL { prec=>NIL, nonterm=>NULL,
	       term => THE constr_list, eop =>NIL, control=>NIL,
		change=>NIL, keyword=>NIL,
		value=>NIL } );
 } );
 ( lr_table::NONTERM 
4,  ( result,  term1left,  constr_list1right),  rest671);
 } 
;  ( 4,  ( ( _,  ( values::CONSTR_LIST constr_list1,  _,  constr_list1right)) !  ( _,  ( _,  nonterm1left,  _)) !  rest671)) => {  my  result = values::MPC_DECL (fn  _ =  {  my  (constr_list as 
constr_list1) = constr_list1 ();
 (DECL { prec=>NIL, control=>NIL, nonterm=> THE constr_list,
	       term => NULL, eop=>NIL, change=>NIL, keyword=>NIL,
	       value=>NIL } );
 } );
 ( 
lr_table::NONTERM 4,  ( result,  nonterm1left,  constr_list1right),  rest671);
 } 
;  ( 5,  ( ( _,  ( values::ID_LIST id_list1,  _,  id_list1right)) !  ( _,  ( values::PREC prec1,  prec1left,  _)) !  rest671)) => {  my  result = values::MPC_DECL (fn  _ =  {  my  (prec as prec1) = 
prec1 ();
 my  (id_list as id_list1) = id_list1 ();
 (DECL { prec=> [(prec, id_list)], control=>NIL,
	      nonterm=>NULL, term=>NULL, eop=>NIL, change=>NIL,
	      keyword=>NIL, value=>NIL } );
 } )
;
 ( lr_table::NONTERM 4,  ( result,  prec1left,  id_list1right),  rest671);
 } 
;  ( 6,  ( ( _,  ( values::ID id1,  _,  id1right)) !  ( _,  ( _,  start1left,  _)) !  rest671)) => {  my  result = values::MPC_DECL (fn  _ =  {  my  (id as id1) = id1 ();
 (
DECL { prec=>NIL, control => [START_SYM (symbol_make id)], nonterm=>NULL,
	       term => NULL, eop => NIL, change=>NIL, keyword=>NIL,
	       value=>NIL } );
 } );
 ( lr_table::NONTERM 4,  ( result, 
 start1left,  id1right),  rest671);
 } 
;  ( 7,  ( ( _,  ( values::ID_LIST id_list1,  _,  id_list1right)) !  ( _,  ( _,  percent_eop1left,  _)) !  rest671)) => {  my  result = values::MPC_DECL (fn  _ =  {  my  (id_list as id_list1) = 
id_list1 ();
 (DECL { prec=>NIL, control=>NIL, nonterm=>NULL, term=>NULL,
		eop=>id_list, change=>NIL, keyword=>NIL,
	 	value=>NIL } );
 } );
 ( lr_table::NONTERM 4,  ( result,  percent_eop1left,  
id_list1right),  rest671);
 } 
;  ( 8,  ( ( _,  ( values::ID_LIST id_list1,  _,  id_list1right)) !  ( _,  ( _,  keyword1left,  _)) !  rest671)) => {  my  result = values::MPC_DECL (fn  _ =  {  my  (id_list as id_list1) = id_list1
 ();
 (DECL { prec=>NIL, control=>NIL, nonterm=>NULL, term=>NULL, eop=>NIL,
		change=>NIL, keyword=>id_list,
	 	value=>NIL } );
 } );
 ( lr_table::NONTERM 4,  ( result,  keyword1left,  id_list1right)
,  rest671);
 } 
;  ( 9,  ( ( _,  ( values::ID_LIST id_list1,  _,  id_list1right)) !  ( _,  ( _,  prefer1left,  _)) !  rest671)) => {  my  result = values::MPC_DECL (fn  _ =  {  my  (id_list as id_list1) = id_list1 ()
;
 (DECL { prec=>NIL, control=>NIL, nonterm=>NULL, term=>NULL, eop=>NIL,
		    change=>map (fn i = ([], [i])) id_list, keyword=>NIL,
		    value=>NIL } );
 } );
 ( lr_table::NONTERM 4,  ( result,  
prefer1left,  id_list1right),  rest671);
 } 
;  ( 10,  ( ( _,  ( values::CHANGE_DECL change_decl1,  _,  change_decl1right)) !  ( _,  ( _,  change1left,  _)) !  rest671)) => {  my  result = values::MPC_DECL (fn  _ =  {  my  (change_decl as 
change_decl1) = change_decl1 ();
 (DECL { prec=>NIL, control=>NIL, nonterm=>NULL, term=>NULL, eop=>NIL,
		change=>change_decl, keyword=>NIL,
		value=>NIL } );
 } );
 ( lr_table::NONTERM 4,  ( result, 
 change1left,  change_decl1right),  rest671);
 } 
;  ( 11,  ( ( _,  ( values::SUBST_DECL subst_decl1,  _,  subst_decl1right)) !  ( _,  ( _,  subst1left,  _)) !  rest671)) => {  my  result = values::MPC_DECL (fn  _ =  {  my  (subst_decl as subst_decl1
) = subst_decl1 ();
 (DECL { prec=>NIL, control=>NIL, nonterm=>NULL, term=>NULL, eop=>NIL,
		change=>subst_decl, keyword=>NIL,
		value=>NIL } );
 } );
 ( lr_table::NONTERM 4,  ( result,  subst1left,  
subst_decl1right),  rest671);
 } 
;  ( 12,  ( ( _,  ( values::ID_LIST id_list1,  _,  id_list1right)) !  ( _,  ( _,  noshift1left,  _)) !  rest671)) => {  my  result = values::MPC_DECL (fn  _ =  {  my  (id_list as id_list1) = id_list1
 ();
 (DECL { prec=>NIL, control => [NSHIFT id_list], nonterm=>NULL, term=>NULL,
	            eop=>NIL, change=>NIL, keyword=>NIL,
		    value=>NIL } );
 } );
 ( lr_table::NONTERM 4,  ( result,  
noshift1left,  id_list1right),  rest671);
 } 
;  ( 13,  ( ( _,  ( values::PROG prog1,  _,  prog1right)) !  ( _,  ( _,  percent_header1left,  _)) !  rest671)) => {  my  result = values::MPC_DECL (fn  _ =  {  my  (prog as prog1) = prog1 ();
 (
DECL { prec=>NIL, control => [GENERIC prog], nonterm=>NULL, term=>NULL,
	            eop=>NIL, change=>NIL, keyword=>NIL,
		    value=>NIL } );
 } );
 ( lr_table::NONTERM 4,  ( result,  
percent_header1left,  prog1right),  rest671);
 } 
;  ( 14,  ( ( _,  ( values::PROG prog1,  _,  prog1right)) !  ( _,  ( _,  percent_token_sig_info1left,  _)) !  rest671)) => {  my  result = values::MPC_DECL (fn  _ =  {  my  (prog as prog1) = prog1 ()
;
 (DECL { prec=>NIL, control => [TOKEN_SIG_INFO prog],
                    nonterm=>NULL, term=>NULL,
	            eop=>NIL, change=>NIL, keyword=>NIL,
		    value=>NIL } );
 } );
 ( 
lr_table::NONTERM 4,  ( result,  percent_token_sig_info1left,  prog1right),  rest671);
 } 
;  ( 15,  ( ( _,  ( values::ID id1,  _,  id1right)) !  ( _,  ( _,  name1left,  _)) !  rest671)) => {  my  result = values::MPC_DECL (fn  _ =  {  my  (id as id1) = id1 ();
 (
DECL { prec=>NIL, control => [PARSER_NAME (symbol_make id)],
	            nonterm=>NULL, term=>NULL,
		    eop=>NIL, change=>NIL, keyword=>NIL, value=>NIL } );
 } );
 ( lr_table::NONTERM 4,  ( result
,  name1left,  id1right),  rest671);
 } 
;  ( 16,  ( ( _,  ( values::TY ty1,  _,  ty1right)) !  _ !  ( _,  ( values::PROG prog1,  _,  _)) !  ( _,  ( _,  percent_arg1left,  _)) !  rest671)) => {  my  result = values::MPC_DECL (fn  _ =  {  my 
 (prog as prog1) = prog1 ();
 my  (ty as ty1) = ty1 ();
 (
DECL { prec=>NIL, control => [PARSE_ARG (prog, ty)], nonterm=>NULL,
	            term=>NULL, eop=>NIL, change=>NIL, keyword=>NIL,
		     value=>NIL } );
 } );
 ( lr_table::NONTERM 4,  ( result,  
percent_arg1left,  ty1right),  rest671);
 } 
;  ( 17,  ( ( _,  ( _,  verbose1left,  verbose1right)) !  rest671)) => {  my  result = values::MPC_DECL (fn  _ =  (
DECL { prec=>NIL, control => [header::VERBOSE],
	        nonterm=>NULL, term=>NULL, eop=>NIL,
	        change=>NIL, keyword=>NIL,
		value=>NIL } ));
 ( lr_table::NONTERM 4,  ( result,  verbose1left,  
verbose1right),  rest671);
 } 
;  ( 18,  ( ( _,  ( _,  nodefault1left,  nodefault1right)) !  rest671)) => {  my  result = values::MPC_DECL (fn  _ =  (
DECL { prec=>NIL, control => [header::NODEFAULT],
	        nonterm=>NULL, term=>NULL, eop=>NIL,
	        change=>NIL, keyword=>NIL,
		value=>NIL } ));
 ( lr_table::NONTERM 4,  ( result,  
nodefault1left,  nodefault1right),  rest671);
 } 
;  ( 19,  ( ( _,  ( _,  percent_pure1left,  percent_pure1right)) !  rest671)) => {  my  result = values::MPC_DECL (fn  _ =  (
DECL { prec=>NIL, control => [header::PURE],
	        nonterm=>NULL, term=>NULL, eop=>NIL,
	        change=>NIL, keyword=>NIL,
		value=>NIL } ));
 ( lr_table::NONTERM 4,  ( result,  percent_pure1left
,  percent_pure1right),  rest671);
 } 
;  ( 20,  ( ( _,  ( values::TY ty1,  _,  ty1right)) !  ( _,  ( _,  percent_pos1left,  _)) !  rest671)) => {  my  result = values::MPC_DECL (fn  _ =  {  my  (ty as ty1) = ty1 ();
 (
DECL { prec=>NIL, control => [header::POS ty],
	        nonterm=>NULL, term=>NULL, eop=>NIL,
	        change=>NIL, keyword=>NIL,
		value=>NIL } );
 } );
 ( lr_table::NONTERM 4,  ( result,  
percent_pos1left,  ty1right),  rest671);
 } 
;  ( 21,  ( ( _,  ( values::PROG prog1,  _,  prog1right)) !  ( _,  ( values::ID id1,  _,  _)) !  ( _,  ( _,  value1left,  _)) !  rest671)) => {  my  result = values::MPC_DECL (fn  _ =  {  my  (id as 
id1) = id1 ();
 my  (prog as prog1) = prog1 ();
 (
DECL { prec=>NIL, control=>NIL,
	        nonterm=>NULL, term=>NULL, eop=>NIL,
	        change=>NIL, keyword=>NIL,
		value => [(symbol_make id, prog ) ] } );
 } );
 ( lr_table::NONTERM 4,  ( result,  
value1left,  prog1right),  rest671);
 } 
;  ( 22,  ( ( _,  ( values::CHANGE_DECL change_decl1,  _,  change_decl1right)) !  _ !  ( _,  ( values::CHANGE_DEC change_dec1,  change_dec1left,  _)) !  rest671)) => {  my  result = 
values::CHANGE_DECL (fn  _ =  {  my  (change_dec as change_dec1) = change_dec1 ();
 my  (change_decl as change_decl1) = change_decl1 ();
 (change_dec ! change_decl);
 } );
 ( lr_table::NONTERM 14,  ( 
result,  change_dec1left,  change_decl1right),  rest671);
 } 
;  ( 23,  ( ( _,  ( values::CHANGE_DEC change_dec1,  change_dec1left,  change_dec1right)) !  rest671)) => {  my  result = values::CHANGE_DECL (fn  _ =  {  my  (change_dec as change_dec1) = change_dec1
 ();
 ([change_dec]);
 } );
 ( lr_table::NONTERM 14,  ( result,  change_dec1left,  change_dec1right),  rest671);
 } 
;  ( 24,  ( ( _,  ( values::ID_LIST id_list2,  _,  id_list2right)) !  _ !  ( _,  ( values::ID_LIST id_list1,  id_list1left,  _)) !  rest671)) => {  my  result = values::CHANGE_DEC (fn  _ =  {  my  
id_list1 = id_list1 ();
 my  id_list2 = id_list2 ();
 (id_list1, id_list2);
 } );
 ( lr_table::NONTERM 15,  ( result,  id_list1left,  id_list2right),  rest671);
 } 
;  ( 25,  ( ( _,  ( values::SUBST_DECL subst_decl1,  _,  subst_decl1right)) !  _ !  ( _,  ( values::SUBST_DEC subst_dec1,  subst_dec1left,  _)) !  rest671)) => {  my  result = values::SUBST_DECL (fn 
 _ =  {  my  (subst_dec as subst_dec1) = subst_dec1 ();
 my  (subst_decl as subst_decl1) = subst_decl1 ();
 (subst_dec ! subst_decl);
 } );
 ( lr_table::NONTERM 12,  ( result,  subst_dec1left,  
subst_decl1right),  rest671);
 } 
;  ( 26,  ( ( _,  ( values::SUBST_DEC subst_dec1,  subst_dec1left,  subst_dec1right)) !  rest671)) => {  my  result = values::SUBST_DECL (fn  _ =  {  my  (subst_dec as subst_dec1) = subst_dec1 ();
 (
[subst_dec]);
 } );
 ( lr_table::NONTERM 12,  ( result,  subst_dec1left,  subst_dec1right),  rest671);
 } 
;  ( 27,  ( ( _,  ( values::ID id2,  _,  id2right)) !  _ !  ( _,  ( values::ID id1,  id1left,  _)) !  rest671)) => {  my  result = values::SUBST_DEC (fn  _ =  {  my  id1 = id1 ();
 my  id2 = id2 ();

 ([symbol_make id2],[symbol_make id1]);
 } );
 ( lr_table::NONTERM 13,  ( result,  id1left,  id2right),  rest671);
 } 
;  ( 28,  ( ( _,  ( values::TY ty1,  _,  ty1right)) !  _ !  ( _,  ( values::ID id1,  _,  _)) !  _ !  ( _,  ( values::CONSTR_LIST constr_list1,  constr_list1left,  _)) !  rest671)) => {  my  result = 
values::CONSTR_LIST (fn  _ =  {  my  (constr_list as constr_list1) = constr_list1 ();
 my  (id as id1) = id1 ();
 my  (ty as ty1) = ty1 ();
 ((symbol_make id, THE (type_make ty)) ! constr_list);
 } )
;
 ( lr_table::NONTERM 1,  ( result,  constr_list1left,  ty1right),  rest671);
 } 
;  ( 29,  ( ( _,  ( values::ID id1,  _,  id1right)) !  _ !  ( _,  ( values::CONSTR_LIST constr_list1,  constr_list1left,  _)) !  rest671)) => {  my  result = values::CONSTR_LIST (fn  _ =  {  my  (
constr_list as constr_list1) = constr_list1 ();
 my  (id as id1) = id1 ();
 ((symbol_make id, NULL) ! constr_list);
 } );
 ( lr_table::NONTERM 1,  ( result,  constr_list1left,  id1right),  rest671)
;
 } 
;  ( 30,  ( ( _,  ( values::TY ty1,  _,  ty1right)) !  _ !  ( _,  ( values::ID id1,  id1left,  _)) !  rest671)) => {  my  result = values::CONSTR_LIST (fn  _ =  {  my  (id as id1) = id1 ();
 my  (ty
 as ty1) = ty1 ();
 ([(symbol_make id, THE (type_make ty))]);
 } );
 ( lr_table::NONTERM 1,  ( result,  id1left,  ty1right),  rest671);
 } 
;  ( 31,  ( ( _,  ( values::ID id1,  id1left,  id1right)) !  rest671)) => {  my  result = values::CONSTR_LIST (fn  _ =  {  my  (id as id1) = id1 ();
 ([(symbol_make id, NULL)]);
 } );
 ( 
lr_table::NONTERM 1,  ( result,  id1left,  id1right),  rest671);
 } 
;  ( 32,  ( ( _,  ( values::RHS_LIST rhs_list1,  _,  rhs_list1right)) !  _ !  ( _,  ( values::ID id1,  id1left,  _)) !  rest671)) => {  my  result = values::G_RULE (fn  _ =  {  my  (id as id1) = id1
 ();
 my  (rhs_list as rhs_list1) = rhs_list1 ();
 (map (fn { rhs, code, prec } =
    	          header::RULE { lhs=>symbol_make id, rhs,
			       code, prec } )
	 rhs_list);
 } );
 ( 
lr_table::NONTERM 9,  ( result,  id1left,  rhs_list1right),  rest671);
 } 
;  ( 33,  ( ( _,  ( values::G_RULE g_rule1,  _,  g_rule1right)) !  ( _,  ( values::G_RULE_LIST g_rule_list1,  g_rule_list1left,  _)) !  rest671)) => {  my  result = values::G_RULE_LIST (fn  _ =  { 
 my  (g_rule_list as g_rule_list1) = g_rule_list1 ();
 my  (g_rule as g_rule1) = g_rule1 ();
 (g_rule @ g_rule_list);
 } );
 ( lr_table::NONTERM 10,  ( result,  g_rule_list1left,  g_rule1right),  
rest671);
 } 
;  ( 34,  ( ( _,  ( values::G_RULE g_rule1,  g_rule1left,  g_rule1right)) !  rest671)) => {  my  result = values::G_RULE_LIST (fn  _ =  {  my  (g_rule as g_rule1) = g_rule1 ();
 (g_rule);
 } );
 ( 
lr_table::NONTERM 10,  ( result,  g_rule1left,  g_rule1right),  rest671);
 } 
;  ( 35,  ( ( _,  ( values::ID_LIST id_list1,  _,  id_list1right)) !  ( _,  ( values::ID id1,  id1left,  _)) !  rest671)) => {  my  result = values::ID_LIST (fn  _ =  {  my  (id as id1) = id1 ();
 my 
 (id_list as id_list1) = id_list1 ();
 (symbol_make id ! id_list);
 } );
 ( lr_table::NONTERM 2,  ( result,  id1left,  id_list1right),  rest671);
 } 
;  ( 36,  ( rest671)) => {  my  result = values::ID_LIST (fn  _ =  (NIL));
 ( lr_table::NONTERM 2,  ( result,  default_position,  default_position),  rest671);
 } 
;  ( 37,  ( ( _,  ( values::PROG prog1,  _,  prog1right)) !  ( _,  ( values::G_RULE_PREC g_rule_prec1,  _,  _)) !  ( _,  ( values::ID_LIST id_list1,  id_list1left,  _)) !  rest671)) => {  my  result =
 values::RHS_LIST (fn  _ =  {  my  (id_list as id_list1) = id_list1 ();
 my  (g_rule_prec as g_rule_prec1) = g_rule_prec1 ();
 my  (prog as prog1) = prog1 ();
 (
 [ { rhs=>id_list, code=>prog, prec=>g_rule_prec } ] );
 } );
 ( lr_table::NONTERM 8,  ( result,  id_list1left,  prog1right),  rest671);
 } 
;  ( 38,  ( ( _,  ( values::PROG prog1,  _,  prog1right)) !  ( _,  ( values::G_RULE_PREC g_rule_prec1,  _,  _)) !  ( _,  ( values::ID_LIST id_list1,  _,  _)) !  _ !  ( _,  ( values::RHS_LIST rhs_list1
,  rhs_list1left,  _)) !  rest671)) => {  my  result = values::RHS_LIST (fn  _ =  {  my  (rhs_list as rhs_list1) = rhs_list1 ();
 my  (id_list as id_list1) = id_list1 ();
 my  (g_rule_prec as 
g_rule_prec1) = g_rule_prec1 ();
 my  (prog as prog1) = prog1 ();
 ( { rhs=>id_list, code=>prog, prec=>g_rule_prec } ! rhs_list);
 } );
 ( lr_table::NONTERM 8,  ( result,  rhs_list1left,  prog1right)
,  rest671);
 } 
;  ( 39,  ( ( _,  ( values::TYVAR tyvar1,  tyvar1left,  tyvar1right)) !  rest671)) => {  my  result = values::TY (fn  _ =  {  my  (tyvar as tyvar1) = tyvar1 ();
 (tyvar);
 } );
 ( lr_table::NONTERM 16
,  ( result,  tyvar1left,  tyvar1right),  rest671);
 } 
;  ( 40,  ( ( _,  ( _,  _,  rbrace1right)) !  ( _,  ( values::RECORD_LIST record_list1,  _,  _)) !  ( _,  ( _,  lbrace1left,  _)) !  rest671)) => {  my  result = values::TY (fn  _ =  {  my  (
record_list as record_list1) = record_list1 ();
 ("{ " + record_list + " }");
 } );
 ( lr_table::NONTERM 16,  ( result,  lbrace1left,  rbrace1right),  rest671);
 } 
;  ( 41,  ( ( _,  ( _,  _,  rbrace1right)) !  ( _,  ( _,  lbrace1left,  _)) !  rest671)) => {  my  result = values::TY (fn  _ =  ("{ }"));
 ( lr_table::NONTERM 16,  ( result,  lbrace1left,  
rbrace1right),  rest671);
 } 
;  ( 42,  ( ( _,  ( values::PROG prog1,  prog1left,  prog1right)) !  rest671)) => {  my  result = values::TY (fn  _ =  {  my  (prog as prog1) = prog1 ();
 ("(" + prog + ")");
 } );
 ( 
lr_table::NONTERM 16,  ( result,  prog1left,  prog1right),  rest671);
 } 
;  ( 43,  ( ( _,  ( values::TY ty1,  _,  ty1right)) !  ( _,  ( values::QUAL_ID qual_id1,  qual_id1left,  _)) !  rest671)) => {  my  result = values::TY (fn  _ =  {  my  (qual_id as qual_id1) = 
qual_id1 ();
 my  (ty as ty1) = ty1 ();
 (qual_id + " " + ty + " ");
 } );
 ( lr_table::NONTERM 16,  ( result,  qual_id1left,  ty1right),  rest671);
 } 
;  ( 44,  ( ( _,  ( values::QUAL_ID qual_id1,  qual_id1left,  qual_id1right)) !  rest671)) => {  my  result = values::TY (fn  _ =  {  my  (qual_id as qual_id1) = qual_id1 ();
 (qual_id);
 } );
 ( 
lr_table::NONTERM 16,  ( result,  qual_id1left,  qual_id1right),  rest671);
 } 
;  ( 45,  ( ( _,  ( values::TY ty2,  _,  ty2right)) !  _ !  ( _,  ( values::TY ty1,  ty1left,  _)) !  rest671)) => {  my  result = values::TY (fn  _ =  {  my  ty1 = ty1 ();
 my  ty2 = ty2 ();
 (
ty1 + "*" + ty2);
 } );
 ( lr_table::NONTERM 16,  ( result,  ty1left,  ty2right),  rest671);
 } 
;  ( 46,  ( ( _,  ( values::TY ty2,  _,  ty2right)) !  _ !  ( _,  ( values::TY ty1,  ty1left,  _)) !  rest671)) => {  my  result = values::TY (fn  _ =  {  my  ty1 = ty1 ();
 my  ty2 = ty2 ();
 (
ty1 + " -> " + ty2);
 } );
 ( lr_table::NONTERM 16,  ( result,  ty1left,  ty2right),  rest671);
 } 
;  ( 47,  ( ( _,  ( values::TY ty1,  _,  ty1right)) !  _ !  ( _,  ( values::LABEL label1,  _,  _)) !  _ !  ( _,  ( values::RECORD_LIST record_list1,  record_list1left,  _)) !  rest671)) => {  my  
result = values::RECORD_LIST (fn  _ =  {  my  (record_list as record_list1) = record_list1 ();
 my  (label as label1) = label1 ();
 my  (ty as ty1) = ty1 ();
 (record_list + ", " + label + ": " + ty)
;
 } );
 ( lr_table::NONTERM 7,  ( result,  record_list1left,  ty1right),  rest671);
 } 
;  ( 48,  ( ( _,  ( values::TY ty1,  _,  ty1right)) !  _ !  ( _,  ( values::LABEL label1,  label1left,  _)) !  rest671)) => {  my  result = values::RECORD_LIST (fn  _ =  {  my  (label as label1) = 
label1 ();
 my  (ty as ty1) = ty1 ();
 (label + ": " + ty);
 } );
 ( lr_table::NONTERM 7,  ( result,  label1left,  ty1right),  rest671);
 } 
;  ( 49,  ( ( _,  ( values::ID id1,  id1left,  id1right)) !  rest671)) => {  my  result = values::QUAL_ID (fn  _ =  {  my  (id as id1) = id1 ();
 ((fn (a, _) = a) id);
 } );
 ( lr_table::NONTERM 6,  (
 result,  id1left,  id1right),  rest671);
 } 
;  ( 50,  ( ( _,  ( values::QUAL_ID qual_id1,  _,  qual_id1right)) !  ( _,  ( values::IDDOT iddot1,  iddot1left,  _)) !  rest671)) => {  my  result = values::QUAL_ID (fn  _ =  {  my  (iddot as iddot1)
 = iddot1 ();
 my  (qual_id as qual_id1) = qual_id1 ();
 (iddot + qual_id);
 } );
 ( lr_table::NONTERM 6,  ( result,  iddot1left,  qual_id1right),  rest671);
 } 
;  ( 51,  ( ( _,  ( values::ID id1,  id1left,  id1right)) !  rest671)) => {  my  result = values::LABEL (fn  _ =  {  my  (id as id1) = id1 ();
 ((fn (a, _) = a) id);
 } );
 ( lr_table::NONTERM 3,  ( 
result,  id1left,  id1right),  rest671);
 } 
;  ( 52,  ( ( _,  ( values::INT int1,  int1left,  int1right)) !  rest671)) => {  my  result = values::LABEL (fn  _ =  {  my  (int as int1) = int1 ();
 (int);
 } );
 ( lr_table::NONTERM 3,  ( result,  
int1left,  int1right),  rest671);
 } 
;  ( 53,  ( ( _,  ( values::ID id1,  _,  id1right)) !  ( _,  ( _,  prec_tag1left,  _)) !  rest671)) => {  my  result = values::G_RULE_PREC (fn  _ =  {  my  (id as id1) = id1 ();
 (THE (symbol_make id)
);
 } );
 ( lr_table::NONTERM 11,  ( result,  prec_tag1left,  id1right),  rest671);
 } 
;  ( 54,  ( rest671)) => {  my  result = values::G_RULE_PREC (fn  _ =  (NULL));
 ( lr_table::NONTERM 11,  ( result,  default_position,  default_position),  rest671);
 } 
; _ => raise exception (MLY_ACTION i392);
esac;
end;
void = values::TM_VOID;
extract = fn a = (fn values::BEGIN x => x;
 _ => { exception PARSE_INTERNAL;
	 raise exception PARSE_INTERNAL; }; end ) a ();
};
};
package tokens : (weak) Mlyacc_Tokens {
Semantic_Value = parser_data::Semantic_Value;
Token (X,Y) = token::Token(X,Y);
fun arrow (p1, p2) = token::TOKEN (parser_data::lr_table::TERM 0, (parser_data::values::TM_VOID, p1, p2));
fun asterisk (p1, p2) = token::TOKEN (parser_data::lr_table::TERM 1, (parser_data::values::TM_VOID, p1, p2));
fun block (p1, p2) = token::TOKEN (parser_data::lr_table::TERM 2, (parser_data::values::TM_VOID, p1, p2));
fun bar (p1, p2) = token::TOKEN (parser_data::lr_table::TERM 3, (parser_data::values::TM_VOID, p1, p2));
fun change (p1, p2) = token::TOKEN (parser_data::lr_table::TERM 4, (parser_data::values::TM_VOID, p1, p2));
fun colon (p1, p2) = token::TOKEN (parser_data::lr_table::TERM 5, (parser_data::values::TM_VOID, p1, p2));
fun comma (p1, p2) = token::TOKEN (parser_data::lr_table::TERM 6, (parser_data::values::TM_VOID, p1, p2));
fun delimiter (p1, p2) = token::TOKEN (parser_data::lr_table::TERM 7, (parser_data::values::TM_VOID, p1, p2));
fun eof_t (p1, p2) = token::TOKEN (parser_data::lr_table::TERM 8, (parser_data::values::TM_VOID, p1, p2));
fun for_t (p1, p2) = token::TOKEN (parser_data::lr_table::TERM 9, (parser_data::values::TM_VOID, p1, p2));
fun header (i, p1, p2) = token::TOKEN (parser_data::lr_table::TERM 10, (parser_data::values::HEADER (fn () = i), p1, p2));
fun id (i, p1, p2) = token::TOKEN (parser_data::lr_table::TERM 11, (parser_data::values::ID (fn () = i), p1, p2));
fun iddot (i, p1, p2) = token::TOKEN (parser_data::lr_table::TERM 12, (parser_data::values::IDDOT (fn () = i), p1, p2));
fun percent_header (p1, p2) = token::TOKEN (parser_data::lr_table::TERM 13, (parser_data::values::TM_VOID, p1, p2));
fun int (i, p1, p2) = token::TOKEN (parser_data::lr_table::TERM 14, (parser_data::values::INT (fn () = i), p1, p2));
fun keyword (p1, p2) = token::TOKEN (parser_data::lr_table::TERM 15, (parser_data::values::TM_VOID, p1, p2));
fun lbrace (p1, p2) = token::TOKEN (parser_data::lr_table::TERM 16, (parser_data::values::TM_VOID, p1, p2));
fun lparen (p1, p2) = token::TOKEN (parser_data::lr_table::TERM 17, (parser_data::values::TM_VOID, p1, p2));
fun name (p1, p2) = token::TOKEN (parser_data::lr_table::TERM 18, (parser_data::values::TM_VOID, p1, p2));
fun nodefault (p1, p2) = token::TOKEN (parser_data::lr_table::TERM 19, (parser_data::values::TM_VOID, p1, p2));
fun nonterm (p1, p2) = token::TOKEN (parser_data::lr_table::TERM 20, (parser_data::values::TM_VOID, p1, p2));
fun noshift (p1, p2) = token::TOKEN (parser_data::lr_table::TERM 21, (parser_data::values::TM_VOID, p1, p2));
fun of_t (p1, p2) = token::TOKEN (parser_data::lr_table::TERM 22, (parser_data::values::TM_VOID, p1, p2));
fun percent_eop (p1, p2) = token::TOKEN (parser_data::lr_table::TERM 23, (parser_data::values::TM_VOID, p1, p2));
fun percent_pure (p1, p2) = token::TOKEN (parser_data::lr_table::TERM 24, (parser_data::values::TM_VOID, p1, p2));
fun percent_pos (p1, p2) = token::TOKEN (parser_data::lr_table::TERM 25, (parser_data::values::TM_VOID, p1, p2));
fun percent_arg (p1, p2) = token::TOKEN (parser_data::lr_table::TERM 26, (parser_data::values::TM_VOID, p1, p2));
fun percent_token_sig_info (p1, p2) = token::TOKEN (parser_data::lr_table::TERM 27, (parser_data::values::TM_VOID, p1, p2));
fun prec (i, p1, p2) = token::TOKEN (parser_data::lr_table::TERM 28, (parser_data::values::PREC (fn () = i), p1, p2));
fun prec_tag (p1, p2) = token::TOKEN (parser_data::lr_table::TERM 29, (parser_data::values::TM_VOID, p1, p2));
fun prefer (p1, p2) = token::TOKEN (parser_data::lr_table::TERM 30, (parser_data::values::TM_VOID, p1, p2));
fun prog (i, p1, p2) = token::TOKEN (parser_data::lr_table::TERM 31, (parser_data::values::PROG (fn () = i), p1, p2));
fun rbrace (p1, p2) = token::TOKEN (parser_data::lr_table::TERM 32, (parser_data::values::TM_VOID, p1, p2));
fun rparen (p1, p2) = token::TOKEN (parser_data::lr_table::TERM 33, (parser_data::values::TM_VOID, p1, p2));
fun subst (p1, p2) = token::TOKEN (parser_data::lr_table::TERM 34, (parser_data::values::TM_VOID, p1, p2));
fun start (p1, p2) = token::TOKEN (parser_data::lr_table::TERM 35, (parser_data::values::TM_VOID, p1, p2));
fun term (p1, p2) = token::TOKEN (parser_data::lr_table::TERM 36, (parser_data::values::TM_VOID, p1, p2));
fun tyvar (i, p1, p2) = token::TOKEN (parser_data::lr_table::TERM 37, (parser_data::values::TYVAR (fn () = i), p1, p2));
fun verbose (p1, p2) = token::TOKEN (parser_data::lr_table::TERM 38, (parser_data::values::TM_VOID, p1, p2));
fun value (p1, p2) = token::TOKEN (parser_data::lr_table::TERM 39, (parser_data::values::TM_VOID, p1, p2));
fun unknown (i, p1, p2) = token::TOKEN (parser_data::lr_table::TERM 40, (parser_data::values::UNKNOWN (fn () = i), p1, p2));
fun bogus_value (p1, p2) = token::TOKEN (parser_data::lr_table::TERM 41, (parser_data::values::TM_VOID, p1, p2));
};
};
