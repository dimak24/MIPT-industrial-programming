#pragma once

#include <string>

#include "node.h"
#include "auxiliary.h"



std::string to_latex(const Node& root) {
    if (IS_CONST(root))
        return eat_extra_zeros(std::to_string(VALUE(root)));
    if (IS_VAR(root))
        switch (VAR(root)) {
            case Variable::VAR_X:
                return "x";
            case Variable::VAR_Y:
                return "y";
            case Variable::VAR_Z:
                return "z";
            default:
                throw derivative_exception(); 
        }

    std::string left  = root.left ? to_latex(*root.left) : "",
                right = root.right ? to_latex(*root.right) : "";


    if (OP(root) == Operator::OP_DIV)
        return "\\frac{" + left + "}{" + right + "}";

    if (OP(root) == Operator::OP_LN)
        return "\\ln{(" + right + ")}";

    if (OP(root) == Operator::OP_POW) {
        if (root.left && !IS_OP(*root.left))
            return left + "^{" + right + "}";
        return "(" + left + ")^{" + right + "}";
    }

    std::string ans = "";


    if (root.left && IS_OP(*root.left) && 
        OP(root) != Operator::OP_PLUS && OP(root) != Operator::OP_MINUS &&
        OP(*root.left) != Operator::OP_DIV)
            ans += "\\left(" + left + "\\right)";
    else if (root.left && IS_CONST(*root.left) && EQUAL(VALUE(*root.left), 0));
        // replace "0-" with "-"
    else
        ans += left;

    switch (OP(root)) {
        case Operator::OP_PLUS:
            ans += "+";
            break;
        case Operator::OP_MINUS:
            ans += "-";
            break;
        case Operator::OP_MUL:
            ans += "*";
            break;
        default:
            throw derivative_exception();
    }

    if (root.right && IS_OP(*root.right) && OP(root) != Operator::OP_PLUS)
        ans += "\\left(" + right + "\\right)";
    else
        ans += right;

    return ans;
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


Node derivative_poem_step(const Node& node, std::string& poem) {

#define L *node.left
#define R *node.right
#define dL derivative_poem_step(L, poem)
#define dR derivative_poem_step(R, poem)
#define cL Node(L)
#define cR Node(R)

    if (IS_CONST(node))
        return MAKE_CONST(0);
    if (IS_VAR(node))
        return MAKE_CONST(1);                                  // TODO
    
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
            ans = dR / cR;
            break;
        default:
            throw derivative_exception();
    }
    append_poem(poem, node, ans);
    return ans;

#undef cR
#undef cL
#undef dR
#undef dL
#undef R
#undef L
}


std::string how_do_I_calculate_the_derivative_poem(const std::string& all_parantheses_expression) {
    srand(time(0));
    Node tree = deserialize(all_parantheses_expression);
    std::string poem = "We are to differentiate:\n" + make_latex_math_expression(to_latex(tree)) + "\n";
    derivative_poem_step(tree, poem);
    return make_latex_document(poem);
}

} // namespace poem
