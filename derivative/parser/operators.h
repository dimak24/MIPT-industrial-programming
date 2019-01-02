#ifdef DEF_MATH_OP
DEF_MATH_OP(PLUS, BINARY, "+", "+", 2, ADD)
DEF_MATH_OP(MINUS, BINARY, "-", "-", 2, SUB)
DEF_MATH_OP(MUL, BINARY, "*", "*",  2, MUL)
DEF_MATH_OP(DIV, BINARY, "/", "/", 2, DIV)
DEF_MATH_OP(POW, BINARY, "^", "^", 2, POW)
DEF_MATH_OP(LN, FUNC, "ln", "\\ln", 1, LOG)
DEF_MATH_OP(EQ, BINARY, "==", "=", 2, EQ)
DEF_MATH_OP(NE, BINARY, "!=", "\\not=", 2, NE)
DEF_MATH_OP(LT, BINARY, "<", "<", 2, LT)
DEF_MATH_OP(LE, BINARY, "<=", "\\le", 2, LE)
DEF_MATH_OP(GT, BINARY, ">", ">", 2, GT)
DEF_MATH_OP(GE, BINARY, ">=", "\\ge", 2, GE)
DEF_MATH_OP(SQRT, FUNC, "sqrt", "\\sqrt", 1, SQRT)
DEF_MATH_OP(SQR, FUNC, "sqrt", "\\sqrt", 1, SQR)
DEF_MATH_OP(SIN, FUNC, "sin", "\\sin", 1, SIN)
DEF_MATH_OP(COS, FUNC, "cos", "\\cos", 1, COS)
DEF_MATH_OP(PM, BINARY, "+-", "\\pm", 2, POP)
DEF_MATH_OP(MD, BINARY, "*/", "?", 2, POP)
#endif

#ifdef DEF_BUILTIN_FUNC
DEF_BUILTIN_FUNC("print", 1, OUT)
DEF_BUILTIN_FUNC("read", 0, IN)
#endif

#ifdef DEF_ASSIGN_OP
DEF_ASSIGN_OP(ASSIGN, "=", PASS)
DEF_ASSIGN_OP(PLUS_ASSIGN, "+=", ADD)
DEF_ASSIGN_OP(MINUS_ASSIGN, "-=", SUB)
DEF_ASSIGN_OP(MUL_ASSIGN, "*=",  MUL)
DEF_ASSIGN_OP(DIV_ASSIGN, "/=", DIV)
DEF_ASSIGN_OP(POW_ASSIGN, "^=", POW)
#endif

#ifdef DEF_KEYWORD
DEF_KEYWORD(DECLARE, ":=")
DEF_KEYWORD(IF, "if")
DEF_KEYWORD(WHILE, "till the cows come home")
DEF_KEYWORD(THEN, "then")
DEF_KEYWORD(REPEAT, "repeat")
DEF_KEYWORD(END, "end")
#endif
