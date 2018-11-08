
#define PUSH(a) stack.push(a)
#define POP() stack.pop()
#define PUSH_REG(reg) PUSH(registers[reg])
#define POP_REG(reg) registers[reg] = POP()
#define PUSH_MEM(index) PUSH(RAM[index])
#define POP_MEM(index) RAM[index] = POP()
#define PUSH_CALL(current_ip) call_stack.push(current_ip)
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
#define EPS 1e-6
#define ABS(a) ((a) >= 0 ? (a) : -(a))
#define EQUAL(a, b) (ABS((a) - (b)) <= EPS)
#define LESS(a, b) ((a) + EPS < (b))
#define GRT(a, b) ((b) + EPS < (a)) 


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
    PUSH(b);
    PUSH(a);
})

DEF_CMD(JAE, 1, {
    let a = POP();
    let b = POP();
    if (!LESS(b, a)) ip = (int)args[0];
    PUSH(b);
    PUSH(a);
})

DEF_CMD(JB, 1, {
    let a = POP();
    let b = POP();
    if (LESS(b, a)) ip = (int)args[0];
    PUSH(b);
    PUSH(a);
})

DEF_CMD(JBE, 1, {
    let a = POP();
    let b = POP();
    if (!GRT(b, a)) ip = (int)args[0];
    PUSH(b);
    PUSH(a);
})

DEF_CMD(JE, 1, {
    let a = POP();
    let b = POP();
    if (EQUAL(b, a)) ip = (int)args[0];
    PUSH(b);
    PUSH(a);
})

DEF_CMD(JNE, 1, {
    let a = POP();
    let b = POP();
    if (!EQUAL(b, a)) ip = (int)args[0];
    PUSH(b);
    PUSH(a);
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

DEF_CMD(SIN, 0, {
    PUSH(sin(POP()));
})

DEF_CMD(COS, 0, {
    PUSH(cos(POP()));
})

DEF_CMD(ARCSIN, 0, {
    PUSH(asin(POP()));
})

DEF_CMD(ARCCOS, 0, {
    PUSH(acos(POP()));
})

DEF_CMD(CALL, 1, {
    PUSH_CALL(ip);
    ip = (int)args[0];
})

DEF_CMD(ENDFUNC, 0, {
    ip = POP_CALL();
})

DEF_CMD(FUNC, 1, {
    ip = (int)args[0];
})

DEF_CMD(DRAW, 3, {
    fwritebmp(fopen("proc_picture.bmp", "w"), 
              (int)args[0], (int)args[1], RAM.data(), RAM.data() + (int)args[2]);
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
