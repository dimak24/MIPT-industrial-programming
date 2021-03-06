
#define PUSH(a) stack.push(a)
#define POP() stack.pop()
#define PUSH_REG(reg) PUSH(registers[reg])
#define POP_REG(reg) registers[reg] = POP()
#define PUSH_MEM(index) PUSH(RAM[index])
#define POP_MEM(index) RAM[index] = POP()
#define PUSH_CALL(current_ip) call_stack.push(current_ip)
#define PUSH_LOCALS_BEGIN(index) locals_begin.push(index)
#define POP_LOCALS_BEGIN() locals_begin.pop()
#define LOCALS_BEGIN() locals_begin.top()
#define POP_CALL() call_stack.pop()
#define let double
#define READ() ({ \
    let a = 0; \
    scanf("%lf", &a); \
    a; \
})
#define WRITE(a) { \
    printf("%lf\n", a); \
}
#define _EPS 1e-6
#define ABS(a) ((a) >= 0 ? (a) : -(a))
#define _EQUAL(a, b) (ABS((a) - (b)) <= _EPS)
#define LESS(a, b) ((a) + _EPS < (b))
#define GRT(a, b) ((b) + _EPS < (a)) 


DEF_CMD(END, 0, {
    return 0;
})

DEF_CMD(PUSH, 2, {
    let reg   = (int)args[1] % (__REGISTERS_NUMBER__ + 1) - 1;
    let shift = (int)args[1] / (__REGISTERS_NUMBER__ + 1);
    args[0] == 0 ? PUSH(args[1]) : 
    args[0] == 1 ? PUSH_REG(args[1]) :
                   PUSH_MEM(registers[reg] + shift);
})

DEF_CMD(POP, 2, {
    let reg = (int)args[1] % (__REGISTERS_NUMBER__ + 1) - 1;
    let shift = (int)args[1] / (__REGISTERS_NUMBER__ + 1);
    args[0] == 0 ? POP() : 
    args[0] == 1 ? POP_REG(args[1]) :
                   POP_MEM(registers[reg] + shift);
})

DEF_CMD(ADD, 0, {
    let a = POP();
    let b = POP();
    PUSH(a + b);
})

DEF_CMD(SUB, 0, {
    let a = POP();
    let b = POP();
    PUSH(b - a);
})

DEF_CMD(MUL, 0, {
    let a = POP();
    let b = POP();
    PUSH(a * b);
})

DEF_CMD(DIV, 0, {
    let a = POP();
    let b = POP();
    PUSH(b / a);
})

DEF_CMD(MOD, 0, {
    let a = POP();
    let b = POP();
    PUSH(fmod(b, a));
})

DEF_CMD(ABS, 0, {
    let a = POP();
    PUSH(ABS(a));
})

DEF_CMD(IN, 0, {
    let a = READ();
    PUSH(a);
})

DEF_CMD(OUT, 0, {
    let a = POP();
    WRITE(a);
})

DEF_CMD(JMP, 1, {
    ip = (int)args[0];
})

DEF_CMD(JA, 1, {
    let a = POP();
    let b = POP();
    if (GRT(b, a)) ip = (int)args[0];
})

DEF_CMD(JAE, 1, {
    let a = POP();
    let b = POP();
    if (!LESS(b, a)) ip = (int)args[0];
})

DEF_CMD(JB, 1, {
    let a = POP();
    let b = POP();
    if (LESS(b, a)) ip = (int)args[0];
})

DEF_CMD(JBE, 1, {
    let a = POP();
    let b = POP();
    if (!GRT(b, a)) ip = (int)args[0];
})

DEF_CMD(JE, 1, {
    let a = POP();
    let b = POP();
    if (_EQUAL(b, a)) ip = (int)args[0];
})

DEF_CMD(JNE, 1, {
    let a = POP();
    let b = POP();
    if (!_EQUAL(b, a)) ip = (int)args[0];
})

DEF_CMD(INC, 0, {
    PUSH(POP() + 1);
})

DEF_CMD(DEC, 0, {
    PUSH(POP() - 1);
})

DEF_CMD(SQRT, 0, {
    PUSH(sqrt(POP()));
})

DEF_CMD(SQR, 0, {
    let a = POP();
    PUSH(a * a);
})

DEF_CMD(POW, 0, {
    let a = POP();
    let b = POP();
    PUSH(pow(b, a));
})

DEF_CMD(SIN, 0, {
    PUSH(sin(POP()));
})

DEF_CMD(COS, 0, {
    PUSH(cos(POP()));
})

DEF_CMD(TG, 0, {
    PUSH(tan(POP()));
})

DEF_CMD(ARCSIN, 0, {
    PUSH(asin(POP()));
})

DEF_CMD(ARCCOS, 0, {
    PUSH(acos(POP()));
})

DEF_CMD(ARCTG, 0, {
    PUSH(atan(POP()));
})

DEF_CMD(SH, 0, {
    PUSH(sinh(POP()));
})

DEF_CMD(CH, 0, {
    PUSH(cosh(POP()));
})

DEF_CMD(TH, 0, {
    PUSH(tanh(POP()));
})

DEF_CMD(ARCSH, 0, {
    PUSH(asinh(POP()));
})

DEF_CMD(ARCCH, 0, {
    PUSH(acosh(POP()));
})

DEF_CMD(ARCTH, 0, {
    PUSH(atanh(POP()));
})

DEF_CMD(LOG, 0, {
    PUSH(log(POP()));
})

DEF_CMD(EXP, 0, {
    PUSH(exp(POP()));
})

DEF_CMD(CALL, 1, {
    PUSH_CALL(ip);
    ip = (int)args[0];
    PUSH_LOCALS_BEGIN(stack.size());

    // nargs
    ip += sizeof(double);
    
    let nlocals = *(double*)(prog + ip);
    for (size_t i = 0; i < nlocals; ++i)
        PUSH(0);
    ip += sizeof(double);
})

DEF_CMD(LEAVE, 1, {
    ip = POP_CALL();
    POP_LOCALS_BEGIN();

    let nargs = *(double*)(prog + (int)args[0]);
    let nlocals = *((double*)(prog + (int)args[0]) + 1);

    for (size_t i = 0; i < nargs + nlocals; ++i)
        POP();
})

DEF_CMD(RET, 1, {
    ip = POP_CALL();
    POP_LOCALS_BEGIN();

    let nargs = *(double*)(prog + (int)args[0]);
    let nlocals = *((double*)(prog + (int)args[0]) + 1);

    let tmp = POP();

    for (size_t i = 0; i < nargs + nlocals; ++i)
        POP();

    PUSH(tmp);
})

DEF_CMD(FD, 3, {
    ip = (int)args[0];
})

DEF_CMD(DRAW, 3, {
    fwritebmp(fopen("proc_picture.bmp", "w"), 
              (int)args[0], (int)args[1], RAM.data(), RAM.data() + (int)args[2]);
})

DEF_CMD(GET_LOCAL, 1, {
    PUSH(stack[LOCALS_BEGIN() + (size_t)args[0]]);
})

DEF_CMD(SET_LOCAL, 1, {
    stack[LOCALS_BEGIN() + (size_t)args[0]] = POP();
})

DEF_CMD(GET_ARG, 1, {
    PUSH(stack[LOCALS_BEGIN() - 1 - (int)args[0]]);
})

DEF_CMD(PASS, 0, {})

DEF_CMD(EQ, 0, {
    let a = POP();
    let b = POP();
    PUSH((double)_EQUAL(a, b));
})

DEF_CMD(NE, 0, {
    let a = POP();
    let b = POP();
    PUSH((double)!_EQUAL(a, b));
})

DEF_CMD(LT, 0, {
    let a = POP();
    let b = POP();
    PUSH((double)LESS(b, a));
})

DEF_CMD(LE, 0, {
    let a = POP();
    let b = POP();
    PUSH((double)!GRT(b, a));
})

DEF_CMD(GT, 0, {
    let a = POP();
    let b = POP();
    PUSH((double)GRT(b, a));
})

DEF_CMD(GE, 0, {
    let a = POP();
    let b = POP();
    PUSH((double)!LESS(b, a));
})

#undef PUSH
#undef POP
#undef PUSH_REG
#undef POP_REG
#undef PUSH_MEM
#undef POP_MEM
#undef POP_CALL
#undef PUSH_CALL
#undef READ
#undef WRITE
#undef let
#undef _EPS
#undef ABS
#undef LESS
#undef _EQUAL
#undef GRT
