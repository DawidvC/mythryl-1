api Burg_Tokens {

    Token (X,Y);
    Semantic_Value;
    raw: ((List String ), X, X) -> Token (Semantic_Value,X);
    id: ((String), X, X) -> Token (Semantic_Value,X);
    int: ((Int), X, X) -> Token (Semantic_Value,X);
    ppercent: ((List String ), X, X) -> Token (Semantic_Value,X);
    k_pipe: (X, X) -> Token (Semantic_Value,X);
    k_equal: (X, X) -> Token (Semantic_Value,X);
    k_rparen: (X, X) -> Token (Semantic_Value,X);
    k_lparen: (X, X) -> Token (Semantic_Value,X);
    k_comma: (X, X) -> Token (Semantic_Value,X);
    k_semicolon: (X, X) -> Token (Semantic_Value,X);
    k_colon: (X, X) -> Token (Semantic_Value,X);
    k_sig: (X, X) -> Token (Semantic_Value,X);
    k_ruleprefix: (X, X) -> Token (Semantic_Value,X);
    k_termprefix: (X, X) -> Token (Semantic_Value,X);
    k_start: (X, X) -> Token (Semantic_Value,X);
    k_term: (X, X) -> Token (Semantic_Value,X);
    k_eof: (X, X) -> Token (Semantic_Value,X);
};

api Burg_Lrvals {

    package tokens:  Burg_Tokens;
    package parser_data:Parser_Data;
    sharing parser_data::token::Token == tokens::Token;
    sharing parser_data::Semantic_Value == tokens::Semantic_Value;
};
