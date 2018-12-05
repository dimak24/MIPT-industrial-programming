#pragma once

#include <exception>
#include <string>
#include <ctime>

#include "auxiliary.h"
#include "derivative.h"
#include "node.h"



struct latex_exception : public std::exception {
private:
    const char* msg_;

public:
    latex_exception(const char* msg)
        : msg_(msg) {}

    virtual const char* what() noexcept {
        return msg_;
    }
};



std::string to_latex(const Node& root) {
    if (IS_CONST(root))
        return eat_extra_zeros(std::to_string(VALUE(root)));
    if (IS_VAR(root))
        return NAME(root);

    if (OP(root) == OP_DIV)
        return "\\frac{" + to_latex(*root.children[0]) + 
                    "}{" + to_latex(*root.children[1]) + "}";

    if (OP(root) == OP_LN)
        return "\\ln{(" + to_latex(*root.children[0]) + ")}";

    if (OP(root) == OP_POW) {
        if (!IS_OP(*root.children[0]))
            return to_latex(*root.children[0]) + "^{" + 
                   to_latex(*root.children[1]) + "}";
        
        return "(" + to_latex(*root.children[0]) + ")^{" + 
                     to_latex(*root.children[1]) + "}";
    }

    switch(OP(root)) {
#define DEF_OP(name, type, mnemonic, arg_num, f) \
        case OP_##name: { \
            if (IS_UNARY(OP_##name)) \
                return std::string(mnemonic) + to_latex(*root.children[0]); \
            if (IS_BINARY(OP_##name)) { \
                auto ans = to_latex(*root.children[0]) + mnemonic + to_latex(*root.children[1]); \
                if (OP_##name == OP_PLUS || OP_##name == OP_MINUS) \
                    ans = "\\left(" + ans + "\\right)"; \
                return ans; \
            } \
            std::string ans = mnemonic "\\left("; \
            for (auto child : root.children) \
                ans += to_latex(*child); \
            return ans +"\\right)"; \
        }
#include "operators.h"
#undef DEF_OP
        default:
            throw latex_exception("cannot convert to latex");
    }
}   


std::string make_latex_math_expression(const std::string& latex_math_code) {
    return "$" + latex_math_code + "$\n";
}


std::string make_latex_document(const std::string& latex_code) {
    std::string ans = "\\documentclass{article}\n"
                      "\\usepackage[T1, T2A]{fontenc}\n"
                      "\\usepackage[utf8]{inputenc}\n"
                      "\\usepackage[russian]{babel}\n"
                      "\\usepackage{mathtools}\n"
                      "\\title{Derivative poem}\n"
                      "\\begin{document}\n"
                      "\\maketitle\n";
    ans += latex_code + "\n";
    return ans + "\\begin{thebibliography}{9}\n"
                 "\\bibitem{onegin} A.\\,S.\\,Pushkin, \\emph{Eugeniy Onegin} 217--304\n"
                 "\\bibitem{clique} B.\\,Bollob\\'{a}s, P.\\,Erd\\\"{o}s," 
                                   "\\emph{Cliques in random graphs}, "
                                   "Mathematical Proceedings of the Cambridge Philosophical Society. "
                                   "{\\bf 80} (1976)~419--427"
                 "\\end{thebibliography}"
                 "\\end{document}";
}


namespace poem {

const std::vector<std::string> linking_words = {
    "Therefore",
    "Easy to see that",
    "Obviously",
    "You may not believe, but",
    "As a simple consequence of the foregoing",
    "Notice, that",
    "Using the basic rules of differentiation",
    "We have",
    "1488 ЛОГАРИФМЫ  БРАТ НЕ   БРОСИМ!!!!",
    "Until suddenly everything ends",
    "Do you really have nothing to do?",
    "lolkekcheburek",
    "see \\cite{clique}",
    "see \\cite{onegin}",
    "deri-deri-derivative"
};


void append_poem(std::string& poem, const Node& node, const Node& ans) {
    poem += linking_words[rand() % linking_words.size()] + 
            "\n\n$\\frac{d}{dx}\\left(" + to_latex(node) + "\\right) = " + 
            to_latex(ans) + "$\n\n";
}


Node derivative_poem_step(const Node& node, std::string& poem, const char* var = "x") {
#define ARG(n) *node.children[n]
#define L ARG(0)
#define R ARG(1)
#define dARG(n) *node,children[n]
#define dL derivative_poem_step(L, poem, var)
#define dR derivative_poem_step(R, poem, var)
#define cL Node(L)
#define cR Node(R)

    if (IS_CONST(node))
        return MAKE_CONST(0);
    if (IS_VAR(node))
        return MAKE_CONST(!strcmp(var, NAME(node)));
    
    Node ans;
    switch (OP(node)) {
        case Operator::OP_PLUS:
            ans = dL + dR;
            break;
        case Operator::OP_MINUS:
            ans = dL - dR;
            break;
        case Operator::OP_MUL:
            ans = dL * cR + cL * dR;
            break;
        case Operator::OP_DIV:
            ans = (dL * cR - cL * dR) / (cR * cR);
            break;
        case Operator::OP_POW:
            ans = IS_CONST(R) ? cR * (cL ^ MAKE_CONST(VALUE(R) - 1)) * dL
                                   : (cL ^ cR) * (dR * ln(cL) + (cR * dL) / cL);
            break;
        case Operator::OP_LN:
            ans = dL / cL;
            break;
        default:
            throw derivative_exception("unknown command");
    }
    append_poem(poem, node, ans);
    return ans;

#undef cR
#undef cL
#undef dR
#undef dL
#undef dARG
#undef R
#undef L
#undef ARG
}


std::string how_do_I_calculate_the_derivative_poem(const std::string& all_parantheses_expression) {
    srand(time(0));
    Node tree = deserialize(all_parantheses_expression);
    std::string poem = "We are to differentiate:\n" + make_latex_math_expression(to_latex(tree)) + "\n";
    derivative_poem_step(tree, poem);
    return make_latex_document(poem);
}

} // namespace poem
