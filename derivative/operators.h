#ifdef DEF_MATH_OP
DEF_MATH_OP(PLUS, BINARY, "+", "+", 2)
DEF_MATH_OP(MINUS, BINARY, "-", "-", 2)
DEF_MATH_OP(MUL, BINARY, "*", "*",  2)
DEF_MATH_OP(DIV, BINARY, "/", "/", 2)
DEF_MATH_OP(POW, BINARY, "^", "^", 2)
DEF_MATH_OP(LN, FUNC, "ln", "\\ln", 1)
DEF_MATH_OP(EQ, BINARY, "==", "=", 2)
DEF_MATH_OP(NE, BINARY, "!=", "\\not=", 2)
DEF_MATH_OP(LT, BINARY, "<", "<", 2)
DEF_MATH_OP(LE, BINARY, "<=", "\\le", 2)
DEF_MATH_OP(GT, BINARY, ">", ">", 2)
DEF_MATH_OP(GE, BINARY, ">=", "\\ge", 2)
#endif

#ifdef DEF_ASSIGN_OP
DEF_ASSIGN_OP(ASSIGN, "=")
DEF_ASSIGN_OP(PLUS_ASSIGN, "+=")
DEF_ASSIGN_OP(MINUS_ASSIGN, "-=")
DEF_ASSIGN_OP(MUL_ASSIGN, "*=")
DEF_ASSIGN_OP(DIV_ASSIGN, "/=")
DEF_ASSIGN_OP(POW_ASSIGN, "^=")
#endif

#ifdef DEF_KEYWORD
DEF_KEYWORD(IF, "if")
DEF_KEYWORD(WHILE, "till the cows come home")
DEF_KEYWORD(THEN, "then")
DEF_KEYWORD(REPEAT, "repeat")
DEF_KEYWORD(END, "end")
#endif
