
generic package dotgraph_lr_vals_g(
	   #
           package token:  Token;
           package g : Attributed_Subgraph_Tree;
	   #
           graph_info:  Void -> g::Graph_Info;
           node_info :  Void -> g::Node_Info;
           edge_info :  Void -> g::Edge_Info;
         )
         { 
package parser_data{
package header { 
## dot-graph.grammar
## COPYRIGHT (c) 1994 AT&T Bell Laboratories.

# Parser specification for the "dot" style syntax
# defined by GraphViz.  For docs on it see:
#     http://www.graphviz.org/Documentation.php
# (The version specified here may well be a decade
# or so behind the above specifications.)

exception ERROR;

Vertex
  = NODE (String, Null_Or( String )) 
  | SUBGRAPH (g::Attributed_Graph -> g::Attributed_Graph)
  ;

fun ga_fn g = g::set_attr(g::GRAPH_PART g);		# "ga_fn" may be "graph_attribute_function"
fun na_fn n = g::set_attr(g::NODE_PART n);		# "na_fn" may be "node_attribute_function"
fun ea_fn e = g::set_attr(g::EDGE_PART e);		# "ea_fn" may be "edge_attribute_function"

fun pna_fn g = g::set_attr(g::PROTONODE_PART g);	# "pna_fn" may be "proto_node_attribute_function"
fun pea_fn g = g::set_attr(g::PROTOEDGE_PART g);	# "pea_fn" may be "proto_edge_attribute_function"

fun find_subgraph (g, name)
    =
    case (g::find_subgraph (g, name))
	#
        THE sg => sg;
        NULL   => raise exception ERROR;
    esac;

stipulate
    count = REF 0;
herein
    fun anonymous ()
	=
	("_anonymous_" + (int::to_string *count))
        before
	    count := *count + 1;
end;

stipulate

    fun make_port_fn (NULL,     NULL) =>   fn _ =  ();
        make_port_fn (THE tp,   NULL) =>   fn e =   ea_fn e ("tailport",tp);
        make_port_fn (NULL,   THE hp) =>   fn e =   ea_fn e ("headport",hp);
        make_port_fn (THE tp, THE hp) => { fn e = { ea_fn e ("headport",hp);
                                                    ea_fn e ("tailport",tp);
                                                  };
                                         };
    end;

herein

    fun make_edges (vs, attributes) g
	=
        {
	    fun do_edge portfn (t,h)
		=
		{   e = g::add_edge{ graph=>g,hd=>h,tl=>t,info=>NULL };

		    portfn e;
		    apply (ea_fn e) attributes;
		};

	    fun mk_e (tl ! (rest as hd ! l))
		    =>
		    case (tl, hd)
			#
		        (NODE(t,tport), NODE(h,hport))
			    =>
			    {   do_edge (make_port_fn(tport,hport)) (g::get_node(g,t,NULL), g::get_node(g,h,NULL));
			        mk_e rest;
			    };

		        (NODE(name,port), SUBGRAPH mkg)
			    =>
			    {   edgefn = do_edge (make_port_fn(port,NULL));
			        t      = g::get_node(g,name,NULL);
			        subg   = mkg g;					# "subg" might be "subgraph"

			        g::apply_nodes (fn n = edgefn(t,n)) subg;

		                mk_e((SUBGRAPH(fn _ = subg)) ! l);
			    };

		        (SUBGRAPH mkg, NODE(name,port))
			    =>
			    {   edgefn =  do_edge (make_port_fn(NULL, port));
			        h      =  g::get_node(g,name,NULL);

			        g::apply_nodes (fn n = edgefn(n,h)) (mkg g);

			        mk_e rest;
			    };

		        (SUBGRAPH mkg, SUBGRAPH mkg')
			    =>
			    {   edgefn = do_edge (make_port_fn(NULL, NULL));

			        tailg  = mkg  g;
			        headg  = mkg' g;

			        g::apply_nodes (fn h = g::apply_nodes (fn t = edgefn(t,h)) tailg) headg;

				mk_e((SUBGRAPH(fn _ = headg)) ! l);
			    };
		    esac;

		mk_e _ => ();
	    end;


	    mk_e vs;

	    g;
	};
end;


};
package lr_table = token::lr_table;
package token = token;
stipulate include lr_table; herein 
my table={   action_rows =
"\
\\001\000\001\000\009\000\002\000\008\000\000\000\
\\001\000\003\000\028\000\008\000\053\000\012\000\024\000\000\000\
\\001\000\008\000\007\000\000\000\
\\001\000\008\000\048\000\000\000\
\\001\000\008\000\056\000\000\000\
\\001\000\008\000\058\000\014\000\057\000\000\000\
\\001\000\008\000\065\000\000\000\
\\001\000\008\000\069\000\000\000\
\\001\000\008\000\075\000\000\000\
\\001\000\011\000\073\000\000\000\
\\001\000\012\000\010\000\000\000\
\\001\000\013\000\045\000\000\000\
\\001\000\015\000\043\000\000\000\
\\001\000\015\000\064\000\000\000\
\\001\000\015\000\066\000\000\000\
\\001\000\016\000\072\000\000\000\
\\001\000\017\000\076\000\000\000\
\\001\000\018\000\047\000\000\000\
\\001\000\021\000\000\000\000\000\
\\078\000\000\000\
\\079\000\001\000\006\000\002\000\005\000\004\000\004\000\000\000\
\\080\000\000\000\
\\081\000\000\000\
\\082\000\000\000\
\\083\000\000\000\
\\084\000\000\000\
\\085\000\000\000\
\\086\000\000\000\
\\087\000\000\000\
\\088\000\008\000\063\000\000\000\
\\089\000\011\000\071\000\000\000\
\\090\000\000\000\
\\091\000\000\000\
\\092\000\000\000\
\\093\000\000\000\
\\093\000\007\000\032\000\000\000\
\\094\000\013\000\045\000\000\000\
\\095\000\000\000\
\\096\000\001\000\029\000\003\000\028\000\005\000\027\000\006\000\026\000\
\\008\000\025\000\012\000\024\000\000\000\
\\097\000\001\000\029\000\003\000\028\000\005\000\027\000\006\000\026\000\
\\008\000\025\000\012\000\024\000\000\000\
\\098\000\000\000\
\\099\000\000\000\
\\100\000\010\000\041\000\000\000\
\\101\000\000\000\
\\102\000\000\000\
\\103\000\000\000\
\\104\000\000\000\
\\105\000\007\000\032\000\000\000\
\\106\000\000\000\
\\107\000\000\000\
\\108\000\000\000\
\\109\000\000\000\
\\109\000\018\000\047\000\000\000\
\\110\000\009\000\037\000\020\000\036\000\000\000\
\\111\000\020\000\036\000\000\000\
\\112\000\009\000\037\000\000\000\
\\113\000\000\000\
\\114\000\000\000\
\\115\000\000\000\
\\116\000\000\000\
\\117\000\000\000\
\\118\000\000\000\
\\119\000\000\000\
\\120\000\000\000\
\\121\000\007\000\032\000\000\000\
\\122\000\000\000\
\\123\000\007\000\032\000\000\000\
\\124\000\000\000\
\\125\000\000\000\
\\126\000\000\000\
\\127\000\012\000\030\000\000\000\
\\128\000\000\000\
\";
    action_row_numbers =
"\020\000\002\000\000\000\023\000\
\\021\000\010\000\024\000\022\000\
\\039\000\070\000\047\000\045\000\
\\044\000\053\000\035\000\046\000\
\\042\000\040\000\038\000\012\000\
\\049\000\011\000\039\000\052\000\
\\027\000\026\000\003\000\025\000\
\\039\000\034\000\001\000\055\000\
\\054\000\050\000\004\000\005\000\
\\034\000\061\000\036\000\043\000\
\\041\000\019\000\048\000\029\000\
\\013\000\006\000\071\000\014\000\
\\063\000\066\000\064\000\051\000\
\\056\000\057\000\060\000\007\000\
\\058\000\062\000\033\000\030\000\
\\015\000\017\000\069\000\037\000\
\\068\000\067\000\065\000\009\000\
\\029\000\031\000\032\000\008\000\
\\028\000\016\000\059\000\018\000";
   goto_table =
"\
\\001\000\075\000\002\000\001\000\000\000\
\\000\000\
\\000\000\
\\000\000\
\\000\000\
\\000\000\
\\000\000\
\\000\000\
\\003\000\021\000\009\000\020\000\010\000\019\000\011\000\018\000\
\\012\000\017\000\013\000\016\000\014\000\015\000\015\000\014\000\
\\016\000\013\000\020\000\012\000\021\000\011\000\023\000\010\000\
\\024\000\009\000\000\000\
\\000\000\
\\022\000\029\000\000\000\
\\000\000\
\\000\000\
\\017\000\033\000\018\000\032\000\019\000\031\000\000\000\
\\007\000\038\000\008\000\037\000\022\000\036\000\000\000\
\\000\000\
\\000\000\
\\000\000\
\\003\000\021\000\009\000\020\000\012\000\040\000\013\000\016\000\
\\014\000\015\000\015\000\014\000\016\000\013\000\020\000\012\000\
\\021\000\011\000\023\000\010\000\024\000\009\000\000\000\
\\000\000\
\\000\000\
\\006\000\042\000\000\000\
\\003\000\021\000\009\000\020\000\010\000\044\000\011\000\018\000\
\\012\000\017\000\013\000\016\000\014\000\015\000\015\000\014\000\
\\016\000\013\000\020\000\012\000\021\000\011\000\023\000\010\000\
\\024\000\009\000\000\000\
\\000\000\
\\000\000\
\\000\000\
\\000\000\
\\000\000\
\\003\000\021\000\009\000\020\000\010\000\047\000\011\000\018\000\
\\012\000\017\000\013\000\016\000\014\000\015\000\015\000\014\000\
\\016\000\013\000\020\000\012\000\021\000\011\000\023\000\010\000\
\\024\000\009\000\000\000\
\\007\000\038\000\008\000\048\000\000\000\
\\015\000\050\000\016\000\013\000\023\000\049\000\024\000\009\000\000\000\
\\018\000\052\000\000\000\
\\019\000\053\000\000\000\
\\000\000\
\\000\000\
\\000\000\
\\007\000\038\000\008\000\057\000\000\000\
\\000\000\
\\006\000\058\000\000\000\
\\000\000\
\\000\000\
\\000\000\
\\000\000\
\\004\000\060\000\009\000\059\000\000\000\
\\000\000\
\\000\000\
\\000\000\
\\000\000\
\\000\000\
\\022\000\065\000\000\000\
\\022\000\066\000\000\000\
\\000\000\
\\000\000\
\\000\000\
\\000\000\
\\000\000\
\\000\000\
\\000\000\
\\000\000\
\\005\000\068\000\000\000\
\\000\000\
\\000\000\
\\000\000\
\\000\000\
\\000\000\
\\000\000\
\\000\000\
\\000\000\
\\004\000\072\000\009\000\059\000\000\000\
\\000\000\
\\000\000\
\\000\000\
\\000\000\
\\000\000\
\\000\000\
\\000\000\
\";
   numstates = 76;
   numrules = 51;
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
Arg = Void;
package values { 
Semantic_Value = TM_VOID | NT_VOID  Void -> Void | SYMBOL Void ->  (String) | QQ_SUBG_HDR Void ->  (String) | QQ_SUBG_STMT Void ->  ((g::Attributed_Graph -> g::Attributed_Graph))
 | QQ_EDGE_RHS Void ->  (List ( Vertex ) ) | QQ_EDGE_STMT Void ->  ((g::Attributed_Graph -> g::Attributed_Graph)) | QQ_NODE_STMT Void ->  ((g::Attributed_Graph -> g::Attributed_Graph))
 | QQ_PORT_ANGLE Void ->  (String) | QQ_PORT_LOCATION Void ->  (String) | QQ_NODE_PORT Void ->  (Null_Or ( String ) ) | QQ_NODE_NAME Void ->  (String)
 | QQ_NODE_ID Void ->  ((String, Null_Or( String ))) | QQ_ATTR_STMT Void ->  ((g::Attributed_Graph -> g::Attributed_Graph)) | QQ_STMT1 Void ->  ((g::Attributed_Graph -> g::Attributed_Graph))
 | QQ_STMT Void ->  ((g::Attributed_Graph -> g::Attributed_Graph)) | QQ_STMT_LIST1 Void ->  ((g::Attributed_Graph -> g::Attributed_Graph))
 | QQ_STMT_LIST Void ->  ((g::Attributed_Graph -> g::Attributed_Graph)) | QQ_ATTR_SET Void ->  ((String, String)) | QQ_OPT_ATTR_LIST Void ->  (List ( (String, String)) )
 | QQ_REC_ATTR_LIST Void ->  (List ( (String, String)) ) | QQ_ATTR_LIST Void ->  (List ( (String, String)) ) | QQ_INSIDE_ATTR_LIST Void ->  (List ( (String, String)) )
 | QQ_ATTR_ILK Void ->  ((List ((String, String)) -> g::Attributed_Graph -> g::Attributed_Graph)) | QQ_GRAPH_TYPE Void ->  (String) | QQ_FILE Void ->  (Null_Or ( g::Attributed_Graph ) );
};
Semantic_Value = values::Semantic_Value;
Result = Null_Or ( g::Attributed_Graph ) ;
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
fn (TERM 20) => TRUE; _ => FALSE; end;
show_terminal =
fn (TERM 0) => "GRAPH"
; (TERM 1) => "DIGRAPH"
; (TERM 2) => "SUBGRAPH"
; (TERM 3) => "STRICT"
; (TERM 4) => "NODE"
; (TERM 5) => "EDGE"
; (TERM 6) => "EDGEOP"
; (TERM 7) => "SYMBOL"
; (TERM 8) => "COLON"
; (TERM 9) => "SEMICOLON"
; (TERM 10) => "COMMA"
; (TERM 11) => "LBRACE"
; (TERM 12) => "LBRACKET"
; (TERM 13) => "LPAREN"
; (TERM 14) => "RBRACE"
; (TERM 15) => "RBRACKET"
; (TERM 16) => "RPAREN"
; (TERM 17) => "EQUAL"
; (TERM 18) => "DOT"
; (TERM 19) => "AT"
; (TERM 20) => "EOF"
; _ => "bogus-term"; end;
stipulate include header; herein
errtermvalue=
fn _ => values::TM_VOID;
 end; end;
my terms:  List( Terminal ) = NIL
 @@ (TERM 20) @@ (TERM 19) @@ (TERM 18) @@ (TERM 17) @@ (TERM 16) @@ (TERM 15) @@ (TERM 14) @@ (TERM 13) @@ (TERM 12) @@ (TERM 11) @@ (TERM 10) @@ (TERM 9) @@ (TERM 8) @@ (TERM 6) @@ (TERM 5) @@ 
(TERM 4) @@ (TERM 3) @@ (TERM 2) @@ (TERM 1) @@ (TERM 0);
};
package actions {
exception MLY_ACTION Int;
stipulate include header; herein
actions = 
fn (i392, default_position, stack, 
    (()): Arg) = 
case (i392, stack)
  ( 0,  ( ( _,  ( _,  _,  rbrace1right)) !  ( _,  ( values::QQ_STMT_LIST stmt_list1,  _,  _)) !  _ !  ( _,  ( values::SYMBOL symbol1,  _,  _)) !  ( _,  ( values::QQ_GRAPH_TYPE graph_type1,  
graph_type1left,  _)) !  rest671)) => {  my  result = values::QQ_FILE (fn  _ =  {  my  (graph_type as graph_type1) = graph_type1 ();
 my  (symbol as symbol1) = symbol1 ();
 my  (stmt_list as 
stmt_list1) = stmt_list1 ();
 (
 { g = g::make_graph{ name=>symbol, graph_info, edge_info, node_info, info=>NULL};

                         g::set_attr (g::GRAPH_PART g) ("graph_type",graph_type);
                         THE (stmt_list g) ;
                       }
);
 } );
 ( lr_table::NONTERM 0,  ( result,  graph_type1left,  rbrace1right),  rest671);
 } 
;  ( 1,  ( rest671)) => {  my  result = values::QQ_FILE (fn  _ =  (NULL));
 ( lr_table::NONTERM 0,  ( result,  default_position,  default_position),  rest671);
 } 
;  ( 2,  ( ( _,  ( _,  graph1left,  graph1right)) !  rest671)) => {  my  result = values::QQ_GRAPH_TYPE (fn  _ =  ("g"));
 ( lr_table::NONTERM 1,  ( result,  graph1left,  graph1right),  rest671);
 } 
;  ( 3,  ( ( _,  ( _,  _,  graph1right)) !  ( _,  ( _,  strict1left,  _)) !  rest671)) => {  my  result = values::QQ_GRAPH_TYPE (fn  _ =  ("sg"));
 ( lr_table::NONTERM 1,  ( result,  strict1left,  
graph1right),  rest671);
 } 
;  ( 4,  ( ( _,  ( _,  digraph1left,  digraph1right)) !  rest671)) => {  my  result = values::QQ_GRAPH_TYPE (fn  _ =  ("dg"));
 ( lr_table::NONTERM 1,  ( result,  digraph1left,  digraph1right),  
rest671);
 } 
;  ( 5,  ( ( _,  ( _,  _,  digraph1right)) !  ( _,  ( _,  strict1left,  _)) !  rest671)) => {  my  result = values::QQ_GRAPH_TYPE (fn  _ =  ("sdg"));
 ( lr_table::NONTERM 1,  ( result,  strict1left,  
digraph1right),  rest671);
 } 
;  ( 6,  ( ( _,  ( _,  graph1left,  graph1right)) !  rest671)) => {  my  result = values::QQ_ATTR_ILK (fn  _ =  (fn al = fn g = { apply (ga_fn  g) al;  g; }));
 ( lr_table::NONTERM 2,  ( result,  
graph1left,  graph1right),  rest671);
 } 
;  ( 7,  ( ( _,  ( _,  node1left,  node1right)) !  rest671)) => {  my  result = values::QQ_ATTR_ILK (fn  _ =  (fn al = fn g = { apply (pna_fn g) al;  g; }));
 ( lr_table::NONTERM 2,  ( result,  
node1left,  node1right),  rest671);
 } 
;  ( 8,  ( ( _,  ( _,  edge1left,  edge1right)) !  rest671)) => {  my  result = values::QQ_ATTR_ILK (fn  _ =  (fn al = fn g = { apply (pea_fn g) al;  g; }));
 ( lr_table::NONTERM 2,  ( result,  
edge1left,  edge1right),  rest671);
 } 
;  ( 9,  ( ( _,  ( values::QQ_INSIDE_ATTR_LIST inside_attr_list1,  _,  inside_attr_list1right)) !  ( _,  ( values::NT_VOID optcomma1,  _,  _)) !  ( _,  ( values::QQ_ATTR_SET attr_set1,  attr_set1left
,  _)) !  rest671)) => {  my  result = values::QQ_INSIDE_ATTR_LIST (fn  _ =  {  my  (attr_set as attr_set1) = attr_set1 ();
 my  optcomma1 = optcomma1 ();
 my  (inside_attr_list as inside_attr_list1)
 = inside_attr_list1 ();
 (attr_set ! inside_attr_list);
 } );
 ( lr_table::NONTERM 3,  ( result,  attr_set1left,  inside_attr_list1right),  rest671);
 } 
;  ( 10,  ( rest671)) => {  my  result = values::QQ_INSIDE_ATTR_LIST (fn  _ =  ([]));
 ( lr_table::NONTERM 3,  ( result,  default_position,  default_position),  rest671);
 } 
;  ( 11,  ( rest671)) => {  my  result = values::NT_VOID (fn  _ =  ());
 ( lr_table::NONTERM 4,  ( result,  default_position,  default_position),  rest671);
 } 
;  ( 12,  ( ( _,  ( _,  comma1left,  comma1right)) !  rest671)) => {  my  result = values::NT_VOID (fn  _ =  ());
 ( lr_table::NONTERM 4,  ( result,  comma1left,  comma1right),  rest671);
 } 
;  ( 13,  ( ( _,  ( _,  _,  rbracket1right)) !  ( _,  ( values::QQ_INSIDE_ATTR_LIST inside_attr_list1,  _,  _)) !  ( _,  ( _,  lbracket1left,  _)) !  rest671)) => {  my  result = values::QQ_ATTR_LIST
 (fn  _ =  {  my  (inside_attr_list as inside_attr_list1) = inside_attr_list1 ();
 (inside_attr_list);
 } );
 ( lr_table::NONTERM 5,  ( result,  lbracket1left,  rbracket1right),  rest671);
 } 
;  ( 14,  ( ( _,  ( values::QQ_ATTR_LIST attr_list1,  _,  attr_list1right)) !  ( _,  ( values::QQ_REC_ATTR_LIST rec_attr_list1,  rec_attr_list1left,  _)) !  rest671)) => {  my  result = 
values::QQ_REC_ATTR_LIST (fn  _ =  {  my  (rec_attr_list as rec_attr_list1) = rec_attr_list1 ();
 my  (attr_list as attr_list1) = attr_list1 ();
 (rec_attr_list @ attr_list);
 } );
 ( 
lr_table::NONTERM 6,  ( result,  rec_attr_list1left,  attr_list1right),  rest671);
 } 
;  ( 15,  ( rest671)) => {  my  result = values::QQ_REC_ATTR_LIST (fn  _ =  ([]));
 ( lr_table::NONTERM 6,  ( result,  default_position,  default_position),  rest671);
 } 
;  ( 16,  ( ( _,  ( values::QQ_REC_ATTR_LIST rec_attr_list1,  rec_attr_list1left,  rec_attr_list1right)) !  rest671)) => {  my  result = values::QQ_OPT_ATTR_LIST (fn  _ =  {  my  (rec_attr_list as 
rec_attr_list1) = rec_attr_list1 ();
 (rec_attr_list);
 } );
 ( lr_table::NONTERM 7,  ( result,  rec_attr_list1left,  rec_attr_list1right),  rest671);
 } 
;  ( 17,  ( ( _,  ( values::SYMBOL symbol2,  _,  symbol2right)) !  _ !  ( _,  ( values::SYMBOL symbol1,  symbol1left,  _)) !  rest671)) => {  my  result = values::QQ_ATTR_SET (fn  _ =  {  my  symbol1
 = symbol1 ();
 my  symbol2 = symbol2 ();
 ((symbol1, symbol2));
 } );
 ( lr_table::NONTERM 8,  ( result,  symbol1left,  symbol2right),  rest671);
 } 
;  ( 18,  ( ( _,  ( values::QQ_STMT_LIST1 stmt_list11,  stmt_list11left,  stmt_list11right)) !  rest671)) => {  my  result = values::QQ_STMT_LIST (fn  _ =  {  my  (stmt_list1 as stmt_list11) = 
stmt_list11 ();
 (stmt_list1);
 } );
 ( lr_table::NONTERM 9,  ( result,  stmt_list11left,  stmt_list11right),  rest671);
 } 
;  ( 19,  ( rest671)) => {  my  result = values::QQ_STMT_LIST (fn  _ =  (fn g = g));
 ( lr_table::NONTERM 9,  ( result,  default_position,  default_position),  rest671);
 } 
;  ( 20,  ( ( _,  ( values::QQ_STMT stmt1,  stmt1left,  stmt1right)) !  rest671)) => {  my  result = values::QQ_STMT_LIST1 (fn  _ =  {  my  (stmt as stmt1) = stmt1 ();
 (stmt);
 } );
 ( 
lr_table::NONTERM 10,  ( result,  stmt1left,  stmt1right),  rest671);
 } 
;  ( 21,  ( ( _,  ( values::QQ_STMT stmt1,  _,  stmt1right)) !  ( _,  ( values::QQ_STMT_LIST1 stmt_list11,  stmt_list11left,  _)) !  rest671)) => {  my  result = values::QQ_STMT_LIST1 (fn  _ =  {  my 
 (stmt_list1 as stmt_list11) = stmt_list11 ();
 my  (stmt as stmt1) = stmt1 ();
 (stmt o stmt_list1);
 } );
 ( lr_table::NONTERM 10,  ( result,  stmt_list11left,  stmt1right),  rest671);
 } 
;  ( 22,  ( ( _,  ( values::QQ_STMT1 stmt11,  stmt11left,  stmt11right)) !  rest671)) => {  my  result = values::QQ_STMT (fn  _ =  {  my  (stmt1 as stmt11) = stmt11 ();
 (stmt1);
 } );
 ( 
lr_table::NONTERM 11,  ( result,  stmt11left,  stmt11right),  rest671);
 } 
;  ( 23,  ( ( _,  ( _,  _,  semicolon1right)) !  ( _,  ( values::QQ_STMT1 stmt11,  stmt11left,  _)) !  rest671)) => {  my  result = values::QQ_STMT (fn  _ =  {  my  (stmt1 as stmt11) = stmt11 ();
 (
stmt1);
 } );
 ( lr_table::NONTERM 11,  ( result,  stmt11left,  semicolon1right),  rest671);
 } 
;  ( 24,  ( ( _,  ( values::QQ_NODE_STMT node_stmt1,  node_stmt1left,  node_stmt1right)) !  rest671)) => {  my  result = values::QQ_STMT1 (fn  _ =  {  my  (node_stmt as node_stmt1) = node_stmt1 ();
 (
node_stmt);
 } );
 ( lr_table::NONTERM 12,  ( result,  node_stmt1left,  node_stmt1right),  rest671);
 } 
;  ( 25,  ( ( _,  ( values::QQ_EDGE_STMT edge_stmt1,  edge_stmt1left,  edge_stmt1right)) !  rest671)) => {  my  result = values::QQ_STMT1 (fn  _ =  {  my  (edge_stmt as edge_stmt1) = edge_stmt1 ();
 (
edge_stmt);
 } );
 ( lr_table::NONTERM 12,  ( result,  edge_stmt1left,  edge_stmt1right),  rest671);
 } 
;  ( 26,  ( ( _,  ( values::QQ_ATTR_STMT attr_stmt1,  attr_stmt1left,  attr_stmt1right)) !  rest671)) => {  my  result = values::QQ_STMT1 (fn  _ =  {  my  (attr_stmt as attr_stmt1) = attr_stmt1 ();
 (
attr_stmt);
 } );
 ( lr_table::NONTERM 12,  ( result,  attr_stmt1left,  attr_stmt1right),  rest671);
 } 
;  ( 27,  ( ( _,  ( values::QQ_SUBG_STMT subg_stmt1,  subg_stmt1left,  subg_stmt1right)) !  rest671)) => {  my  result = values::QQ_STMT1 (fn  _ =  {  my  (subg_stmt as subg_stmt1) = subg_stmt1 ();
 (
fn g = { subg_stmt g; g; });
 } );
 ( lr_table::NONTERM 12,  ( result,  subg_stmt1left,  subg_stmt1right),  rest671);
 } 
;  ( 28,  ( ( _,  ( values::QQ_ATTR_LIST attr_list1,  _,  attr_list1right)) !  ( _,  ( values::QQ_ATTR_ILK attr_ilk1,  attr_ilk1left,  _)) !  rest671)) => {  my  result = values::QQ_ATTR_STMT (fn  _
 =  {  my  (attr_ilk as attr_ilk1) = attr_ilk1 ();
 my  (attr_list as attr_list1) = attr_list1 ();
 (attr_ilk attr_list);
 } );
 ( lr_table::NONTERM 13,  ( result,  attr_ilk1left,  attr_list1right),  
rest671);
 } 
;  ( 29,  ( ( _,  ( values::QQ_ATTR_SET attr_set1,  attr_set1left,  attr_set1right)) !  rest671)) => {  my  result = values::QQ_ATTR_STMT (fn  _ =  {  my  (attr_set as attr_set1) = attr_set1 ();
 (
fn g = { ga_fn g (#1 attr_set,#2 attr_set); g; });
 } );
 ( lr_table::NONTERM 13,  ( result,  attr_set1left,  attr_set1right),  rest671);
 } 
;  ( 30,  ( ( _,  ( values::QQ_NODE_PORT node_port1,  _,  node_port1right)) !  ( _,  ( values::QQ_NODE_NAME node_name1,  node_name1left,  _)) !  rest671)) => {  my  result = values::QQ_NODE_ID (fn  _
 =  {  my  (node_name as node_name1) = node_name1 ();
 my  (node_port as node_port1) = node_port1 ();
 (node_name, node_port);
 } );
 ( lr_table::NONTERM 14,  ( result,  node_name1left,  
node_port1right),  rest671);
 } 
;  ( 31,  ( ( _,  ( values::SYMBOL symbol1,  symbol1left,  symbol1right)) !  rest671)) => {  my  result = values::QQ_NODE_NAME (fn  _ =  {  my  (symbol as symbol1) = symbol1 ();
 (symbol);
 } );
 ( 
lr_table::NONTERM 15,  ( result,  symbol1left,  symbol1right),  rest671);
 } 
;  ( 32,  ( rest671)) => {  my  result = values::QQ_NODE_PORT (fn  _ =  (NULL));
 ( lr_table::NONTERM 16,  ( result,  default_position,  default_position),  rest671);
 } 
;  ( 33,  ( ( _,  ( values::QQ_PORT_LOCATION port_location1,  port_location1left,  port_location1right)) !  rest671)) => {  my  result = values::QQ_NODE_PORT (fn  _ =  {  my  (port_location as 
port_location1) = port_location1 ();
 (THE port_location);
 } );
 ( lr_table::NONTERM 16,  ( result,  port_location1left,  port_location1right),  rest671);
 } 
;  ( 34,  ( ( _,  ( values::QQ_PORT_ANGLE port_angle1,  port_angle1left,  port_angle1right)) !  rest671)) => {  my  result = values::QQ_NODE_PORT (fn  _ =  {  my  (port_angle as port_angle1) = 
port_angle1 ();
 (THE port_angle);
 } );
 ( lr_table::NONTERM 16,  ( result,  port_angle1left,  port_angle1right),  rest671);
 } 
;  ( 35,  ( ( _,  ( values::QQ_PORT_LOCATION port_location1,  _,  port_location1right)) !  ( _,  ( values::QQ_PORT_ANGLE port_angle1,  port_angle1left,  _)) !  rest671)) => {  my  result = 
values::QQ_NODE_PORT (fn  _ =  {  my  (port_angle as port_angle1) = port_angle1 ();
 my  (port_location as port_location1) = port_location1 ();
 (THE (port_angle + port_location));
 } );
 ( 
lr_table::NONTERM 16,  ( result,  port_angle1left,  port_location1right),  rest671);
 } 
;  ( 36,  ( ( _,  ( values::QQ_PORT_ANGLE port_angle1,  _,  port_angle1right)) !  ( _,  ( values::QQ_PORT_LOCATION port_location1,  port_location1left,  _)) !  rest671)) => {  my  result = 
values::QQ_NODE_PORT (fn  _ =  {  my  (port_location as port_location1) = port_location1 ();
 my  (port_angle as port_angle1) = port_angle1 ();
 (THE (port_location + port_angle));
 } );
 ( 
lr_table::NONTERM 16,  ( result,  port_location1left,  port_angle1right),  rest671);
 } 
;  ( 37,  ( ( _,  ( values::SYMBOL symbol1,  _,  symbol1right)) !  ( _,  ( _,  colon1left,  _)) !  rest671)) => {  my  result = values::QQ_PORT_LOCATION (fn  _ =  {  my  (symbol as symbol1) = symbol1
 ();
 (":" + symbol);
 } );
 ( lr_table::NONTERM 17,  ( result,  colon1left,  symbol1right),  rest671);
 } 
;  ( 38,  ( ( _,  ( _,  _,  rparen1right)) !  ( _,  ( values::SYMBOL symbol2,  _,  _)) !  _ !  ( _,  ( values::SYMBOL symbol1,  _,  _)) !  _ !  ( _,  ( _,  colon1left,  _)) !  rest671)) => {  my  
result = values::QQ_PORT_LOCATION (fn  _ =  {  my  symbol1 = symbol1 ();
 my  symbol2 = symbol2 ();
 (cat [":(",symbol1,",",symbol2,")"]);
 } );
 ( lr_table::NONTERM 17,  ( result,  colon1left,  
rparen1right),  rest671);
 } 
;  ( 39,  ( ( _,  ( values::SYMBOL symbol1,  _,  symbol1right)) !  ( _,  ( _,  at1left,  _)) !  rest671)) => {  my  result = values::QQ_PORT_ANGLE (fn  _ =  {  my  (symbol as symbol1) = symbol1 ();
 (
"@" + symbol);
 } );
 ( lr_table::NONTERM 18,  ( result,  at1left,  symbol1right),  rest671);
 } 
;  ( 40,  ( ( _,  ( values::QQ_OPT_ATTR_LIST opt_attr_list1,  _,  opt_attr_list1right)) !  ( _,  ( values::QQ_NODE_ID node_id1,  node_id1left,  _)) !  rest671)) => {  my  result = values::QQ_NODE_STMT
 (fn  _ =  {  my  (node_id as node_id1) = node_id1 ();
 my  (opt_attr_list as opt_attr_list1) = opt_attr_list1 ();
 (fn g = { apply (na_fn (g::get_node(g, #1 node_id,NULL))) opt_attr_list; g; });
 } )
;
 ( lr_table::NONTERM 19,  ( result,  node_id1left,  opt_attr_list1right),  rest671);
 } 
;  ( 41,  ( ( _,  ( values::QQ_OPT_ATTR_LIST opt_attr_list1,  _,  opt_attr_list1right)) !  ( _,  ( values::QQ_EDGE_RHS edge_rhs1,  _,  _)) !  ( _,  ( values::QQ_NODE_ID node_id1,  node_id1left,  _))
 !  rest671)) => {  my  result = values::QQ_EDGE_STMT (fn  _ =  {  my  (node_id as node_id1) = node_id1 ();
 my  (edge_rhs as edge_rhs1) = edge_rhs1 ();
 my  (opt_attr_list as opt_attr_list1) = 
opt_attr_list1 ();
 (make_edges((NODE node_id) ! edge_rhs, opt_attr_list));
 } );
 ( lr_table::NONTERM 20,  ( result,  node_id1left,  opt_attr_list1right),  rest671);
 } 
;  ( 42,  ( ( _,  ( values::QQ_OPT_ATTR_LIST opt_attr_list1,  _,  opt_attr_list1right)) !  ( _,  ( values::QQ_EDGE_RHS edge_rhs1,  _,  _)) !  ( _,  ( values::QQ_SUBG_STMT subg_stmt1,  subg_stmt1left, 
 _)) !  rest671)) => {  my  result = values::QQ_EDGE_STMT (fn  _ =  {  my  (subg_stmt as subg_stmt1) = subg_stmt1 ();
 my  (edge_rhs as edge_rhs1) = edge_rhs1 ();
 my  (opt_attr_list as opt_attr_list1
) = opt_attr_list1 ();
 (make_edges((SUBGRAPH subg_stmt) ! edge_rhs, opt_attr_list));
 } );
 ( lr_table::NONTERM 20,  ( result,  subg_stmt1left,  opt_attr_list1right),  rest671);
 } 
;  ( 43,  ( ( _,  ( values::QQ_NODE_ID node_id1,  _,  node_id1right)) !  ( _,  ( _,  edgeop1left,  _)) !  rest671)) => {  my  result = values::QQ_EDGE_RHS (fn  _ =  {  my  (node_id as node_id1) = 
node_id1 ();
 ([NODE node_id]);
 } );
 ( lr_table::NONTERM 21,  ( result,  edgeop1left,  node_id1right),  rest671);
 } 
;  ( 44,  ( ( _,  ( values::QQ_EDGE_RHS edge_rhs1,  _,  edge_rhs1right)) !  ( _,  ( values::QQ_NODE_ID node_id1,  _,  _)) !  ( _,  ( _,  edgeop1left,  _)) !  rest671)) => {  my  result = 
values::QQ_EDGE_RHS (fn  _ =  {  my  (node_id as node_id1) = node_id1 ();
 my  (edge_rhs as edge_rhs1) = edge_rhs1 ();
 ((NODE node_id) ! edge_rhs);
 } );
 ( lr_table::NONTERM 21,  ( result,  
edgeop1left,  edge_rhs1right),  rest671);
 } 
;  ( 45,  ( ( _,  ( values::QQ_SUBG_STMT subg_stmt1,  _,  subg_stmt1right)) !  ( _,  ( _,  edgeop1left,  _)) !  rest671)) => {  my  result = values::QQ_EDGE_RHS (fn  _ =  {  my  (subg_stmt as 
subg_stmt1) = subg_stmt1 ();
 ([SUBGRAPH subg_stmt]);
 } );
 ( lr_table::NONTERM 21,  ( result,  edgeop1left,  subg_stmt1right),  rest671);
 } 
;  ( 46,  ( ( _,  ( values::QQ_EDGE_RHS edge_rhs1,  _,  edge_rhs1right)) !  ( _,  ( values::QQ_SUBG_STMT subg_stmt1,  _,  _)) !  ( _,  ( _,  edgeop1left,  _)) !  rest671)) => {  my  result = 
values::QQ_EDGE_RHS (fn  _ =  {  my  (subg_stmt as subg_stmt1) = subg_stmt1 ();
 my  (edge_rhs as edge_rhs1) = edge_rhs1 ();
 ((SUBGRAPH subg_stmt) ! edge_rhs);
 } );
 ( lr_table::NONTERM 21,  ( 
result,  edgeop1left,  edge_rhs1right),  rest671);
 } 
;  ( 47,  ( ( _,  ( _,  _,  rbrace1right)) !  ( _,  ( values::QQ_STMT_LIST stmt_list1,  _,  _)) !  _ !  ( _,  ( values::QQ_SUBG_HDR subg_hdr1,  subg_hdr1left,  _)) !  rest671)) => {  my  result = 
values::QQ_SUBG_STMT (fn  _ =  {  my  (subg_hdr as subg_hdr1) = subg_hdr1 ();
 my  (stmt_list as stmt_list1) = stmt_list1 ();
 (fn g = (stmt_list (g::add_subgraph(g, subg_hdr,NULL))));
 } );
 ( 
lr_table::NONTERM 22,  ( result,  subg_hdr1left,  rbrace1right),  rest671);
 } 
;  ( 48,  ( ( _,  ( _,  _,  rbrace1right)) !  ( _,  ( values::QQ_STMT_LIST stmt_list1,  _,  _)) !  ( _,  ( _,  lbrace1left,  _)) !  rest671)) => {  my  result = values::QQ_SUBG_STMT (fn  _ =  {  my  (
stmt_list as stmt_list1) = stmt_list1 ();
 (fn g = (stmt_list (g::add_subgraph(g,anonymous(),NULL))));
 } );
 ( lr_table::NONTERM 22,  ( result,  lbrace1left,  rbrace1right),  rest671);
 } 
;  ( 49,  ( ( _,  ( values::QQ_SUBG_HDR subg_hdr1,  subg_hdr1left,  subg_hdr1right)) !  rest671)) => {  my  result = values::QQ_SUBG_STMT (fn  _ =  {  my  (subg_hdr as subg_hdr1) = subg_hdr1 ();
 (
fn g = find_subgraph(g, subg_hdr));
 } );
 ( lr_table::NONTERM 22,  ( result,  subg_hdr1left,  subg_hdr1right),  rest671);
 } 
;  ( 50,  ( ( _,  ( values::SYMBOL symbol1,  _,  symbol1right)) !  ( _,  ( _,  subgraph1left,  _)) !  rest671)) => {  my  result = values::QQ_SUBG_HDR (fn  _ =  {  my  (symbol as symbol1) = symbol1 ()
;
 (symbol);
 } );
 ( lr_table::NONTERM 23,  ( result,  subgraph1left,  symbol1right),  rest671);
 } 
; _ => raise exception (MLY_ACTION i392);
esac;
end;
void = values::TM_VOID;
extract = fn a = (fn values::QQ_FILE x => x;
 _ => { exception PARSE_INTERNAL;
	 raise exception PARSE_INTERNAL; }; end ) a ();
};
};
package tokens : (weak) Graph_Tokens {
Semantic_Value = parser_data::Semantic_Value;
Token (X,Y) = token::Token(X,Y);
fun graph (p1, p2) = token::TOKEN (parser_data::lr_table::TERM 0, (parser_data::values::TM_VOID, p1, p2));
fun digraph (p1, p2) = token::TOKEN (parser_data::lr_table::TERM 1, (parser_data::values::TM_VOID, p1, p2));
fun subgraph (p1, p2) = token::TOKEN (parser_data::lr_table::TERM 2, (parser_data::values::TM_VOID, p1, p2));
fun strict (p1, p2) = token::TOKEN (parser_data::lr_table::TERM 3, (parser_data::values::TM_VOID, p1, p2));
fun node (p1, p2) = token::TOKEN (parser_data::lr_table::TERM 4, (parser_data::values::TM_VOID, p1, p2));
fun edge (p1, p2) = token::TOKEN (parser_data::lr_table::TERM 5, (parser_data::values::TM_VOID, p1, p2));
fun edgeop (p1, p2) = token::TOKEN (parser_data::lr_table::TERM 6, (parser_data::values::TM_VOID, p1, p2));
fun symbol (i, p1, p2) = token::TOKEN (parser_data::lr_table::TERM 7, (parser_data::values::SYMBOL (fn () = i), p1, p2));
fun colon (p1, p2) = token::TOKEN (parser_data::lr_table::TERM 8, (parser_data::values::TM_VOID, p1, p2));
fun semicolon (p1, p2) = token::TOKEN (parser_data::lr_table::TERM 9, (parser_data::values::TM_VOID, p1, p2));
fun comma (p1, p2) = token::TOKEN (parser_data::lr_table::TERM 10, (parser_data::values::TM_VOID, p1, p2));
fun lbrace (p1, p2) = token::TOKEN (parser_data::lr_table::TERM 11, (parser_data::values::TM_VOID, p1, p2));
fun lbracket (p1, p2) = token::TOKEN (parser_data::lr_table::TERM 12, (parser_data::values::TM_VOID, p1, p2));
fun lparen (p1, p2) = token::TOKEN (parser_data::lr_table::TERM 13, (parser_data::values::TM_VOID, p1, p2));
fun rbrace (p1, p2) = token::TOKEN (parser_data::lr_table::TERM 14, (parser_data::values::TM_VOID, p1, p2));
fun rbracket (p1, p2) = token::TOKEN (parser_data::lr_table::TERM 15, (parser_data::values::TM_VOID, p1, p2));
fun rparen (p1, p2) = token::TOKEN (parser_data::lr_table::TERM 16, (parser_data::values::TM_VOID, p1, p2));
fun equal (p1, p2) = token::TOKEN (parser_data::lr_table::TERM 17, (parser_data::values::TM_VOID, p1, p2));
fun dot (p1, p2) = token::TOKEN (parser_data::lr_table::TERM 18, (parser_data::values::TM_VOID, p1, p2));
fun at (p1, p2) = token::TOKEN (parser_data::lr_table::TERM 19, (parser_data::values::TM_VOID, p1, p2));
fun eof (p1, p2) = token::TOKEN (parser_data::lr_table::TERM 20, (parser_data::values::TM_VOID, p1, p2));
};
};
