#include <string>
#include <variant>
#include <exception>
#include <math.h>
#include <cstdlib>
#include <algorithm>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <vector>



#define IS_OP(node) ((node).data.type == NodeType::NODE_OP)
#define IS_VAR(node) ((node).data.type == NodeType::NODE_VAR)
#define IS_CONST(node) ((node).data.type == NodeType::NODE_CONST)
#define OP(node) (std::get<Operator>((node).data.value))
#define VAR(node) (std::get<Variable>((node).data.value))
#define VALUE(node) (std::get<double>((node).data.value))
#define EPS 1e-5
#define EQUAL(value1, value2) (fabs((value1) - (value2)) < EPS)


struct derivative_exception : public std::exception {

};


enum Variable {
    VAR_X,
    VAR_Y,
    VAR_Z
};


enum Operator {
    OP_PLUS,
    OP_MINUS,
    OP_MUL,
    OP_DIV,
    OP_POW
};


enum NodeType {
    NODE_VAR,
    NODE_OP,
    NODE_CONST
};


struct NodeData {
    NodeType type;
    std::variant<Variable, Operator, double> value;
};


struct Node {
    Node* left;
    NodeData data;
    Node* right;

    Node* parent;


    Node(Node* left, NodeData data, Node* right)
        : left(left), data(data), right(right), parent(nullptr) {
            if (left)
                left->parent = this;
            if (right)
                right->parent = this;
        }

    Node(const Node& another)
        : left(another.left ? new Node(*another.left) : nullptr),
          data(another.data),
          right(another.right ? new Node(*another.right) : nullptr),
          parent(nullptr) {}


    Node(Node* parent) : left(nullptr), right(nullptr), parent(parent) {}


    ~Node() {
        if (left)
            delete left;
        if (right)
            delete right;
    }
};


Node operator+(const Node& left, const Node& right) {
    if (IS_CONST(left) && IS_CONST(right))
        return Node(nullptr, NodeData{NodeType::NODE_CONST, VALUE(left) + VALUE(right)}, nullptr);

    if (IS_CONST(left) && EQUAL(VALUE(left), 0))
        return Node(right);

    if (IS_CONST(right) && EQUAL(VALUE(right), 0))
        return Node(left);
    
    return Node(new Node(left), NodeData{NodeType::NODE_OP, Operator::OP_PLUS}, new Node(right));
}


Node operator-(const Node& left, const Node& right) {
    if (IS_CONST(left) && IS_CONST(right))
        return Node(nullptr, NodeData{NodeType::NODE_CONST, VALUE(left) - VALUE(right)}, nullptr);

    if (IS_CONST(right) && EQUAL(VALUE(right), 0))
        return Node(left);
    
    return Node(new Node(left), NodeData{NodeType::NODE_OP, Operator::OP_MINUS}, new Node(right));
}


Node operator*(const Node& left, const Node& right) {
    if (IS_CONST(left) && IS_CONST(right))
        return Node(nullptr, NodeData{NodeType::NODE_CONST, VALUE(left) * VALUE(right)}, nullptr);

    if ((IS_CONST(left) && EQUAL(VALUE(left), 0)) || (IS_CONST(right) && EQUAL(VALUE(right), 0)))
        return Node(nullptr, {NodeType::NODE_CONST, 0}, nullptr);

    if (IS_CONST(left) && EQUAL(VALUE(left), 1))
        return Node(right);

    if (IS_CONST(right) && EQUAL(VALUE(right), 1))
        return Node(left);
    
    return Node(new Node(left), NodeData{NodeType::NODE_OP, Operator::OP_MUL}, new Node(right));
}


Node operator/(const Node& left, const Node& right) {
    if (IS_CONST(left) && IS_CONST(right))
        return Node(nullptr, NodeData{NodeType::NODE_CONST, VALUE(left) / VALUE(right)}, nullptr);

    if (IS_CONST(left) && EQUAL(VALUE(left), 0))
        return Node(nullptr, {NodeType::NODE_CONST, 0}, nullptr);

    if (IS_CONST(right) && EQUAL(VALUE(right), 1))
        return Node(left);
    
    return Node(new Node(left), NodeData{NodeType::NODE_OP, Operator::OP_DIV}, new Node(right));
}

Node operator^(const Node& left, const Node& right) {
    if (IS_CONST(left) && IS_CONST(right))
        return Node(nullptr, NodeData{NodeType::NODE_CONST, pow(VALUE(left), VALUE(right))}, nullptr);

    if (IS_CONST(left) && EQUAL(VALUE(left), 0))
        return Node(nullptr, {NodeType::NODE_CONST, 0}, nullptr);

    if ((IS_CONST(right) && EQUAL(VALUE(right), 0)) || (IS_CONST(left) && EQUAL(VALUE(left), 1)))
        return Node(nullptr, {NodeType::NODE_CONST, 1}, nullptr);

    return Node(new Node(left), NodeData{NodeType::NODE_OP, Operator::OP_POW}, new Node(right));

}



Node derivative(const Node& node) {
#define L *node.left
#define R *node.right
#define dL derivative(L)
#define dR derivative(R)
#define cL Node(L)
#define cR Node(R)

    if (IS_CONST(node))
        return Node(nullptr, NodeData{NodeType::NODE_CONST, 0}, nullptr);
    if (IS_VAR(node))
        return Node(nullptr, NodeData{NodeType::NODE_CONST, 1}, nullptr);                                       // TODO
    switch (OP(node)) {
        case Operator::OP_PLUS:
            return dL + dR;
        case Operator::OP_MINUS:
            return dL - dR;
        case Operator::OP_MUL:
            return dL * cR + cL * dR;
        case Operator::OP_DIV:
            return (dL * cR - cL * dR) / (cR * cR);
        case Operator::OP_POW:
            if (IS_CONST(R))
                return R * (L ^ Node(nullptr, {NodeType::NODE_CONST, VALUE(R) - 1}, nullptr));
            else /* coming soon */
                throw derivative_exception();
        default:
            throw derivative_exception();
    }

#undef cR
#undef cL
#undef dR
#undef dL
#undef R
#undef L
}


void append_dump(Node* root, std::string& ans, unsigned indent = 0) {
    if (!root)
        return;

    std::string value = "";
    if (IS_OP(*root)) {
        switch (OP(*root)) {
            case OP_PLUS:
                value = "+";
                break;
            case OP_MINUS:
                value = "-";
                break;
            case OP_MUL:
                value = "*";
                break;
            case OP_DIV:
                value = "/";
                break;
            case OP_POW:
                value = "^";
                break;
            default:
                throw derivative_exception();
        }
    }
    else if (IS_CONST(*root))
        value = std::to_string(VALUE(*root));
    else {
        switch (VAR(*root)) {
            case VAR_X:
                value = "x";
                break;
            case VAR_Y:
                value = "y";
                break;
            case VAR_Z:
                value = "z";
                break;
            default:
                throw derivative_exception();
        }
    }

    std::string indent_str(indent, ' ');

    ans += indent_str + std::to_string((size_t)root) + " [label=\"" + value + "\"];\n";
    ans += indent_str + std::to_string((size_t)root) + "-> {";
    
    unsigned argnum = (root->left != nullptr) + (root->right != nullptr);
    if (argnum == 2)
        ans += std::to_string((size_t)root->left) + ", " + std::to_string((size_t)root->right);
    else if (argnum)
        ans += std::to_string((size_t)(root->left ? root->left : root->right));
    
    ans +=  "};\n";

    append_dump(root->left, ans, indent);
    append_dump(root->right, ans, indent);
}


void dump(Node* root) {
    std::string to_dot;
    to_dot += "digraph G {";
    append_dump(root, to_dot, 4);
    to_dot += "}";
    printf("%s\n", to_dot.c_str());
}


Node deserialize(const char* expression, char var = 'x') {
    Node* root = new Node(nullptr);
    Node* node = root;

    const char* end = expression + strlen(expression);

    for (const char* ch = expression; ch != end; ++ch) {
        if (*ch == ' ')
            continue;
        if (*ch == '(') {
            if (!node->left) {
                node->left = new Node(node);
                node = node->left;
            }
            else {
                node->right = new Node(node);
                node = node->right;
            }
        }
        else if (*ch == ')') {
            node = node->parent;
        }
        else if (*ch == var /* is var */)                                                                           // TODO
            node->data = NodeData{NodeType::NODE_VAR, Variable::VAR_X};
        else if (*ch == '+' || *ch == '-' || *ch == '*' || *ch == '/' || *ch == '^') {
            switch (*ch) {
                case '+':
                    node->data = NodeData{NodeType::NODE_OP, Operator::OP_PLUS};
                    break;
                case '-':
                    node->data = NodeData{NodeType::NODE_OP, Operator::OP_MINUS};
                    break;
                case '*':
                    node->data = NodeData{NodeType::NODE_OP, Operator::OP_MUL};
                    break;
                case '/':
                    node->data = NodeData{NodeType::NODE_OP, Operator::OP_DIV};
                    break;
                case '^':
                    node->data = NodeData{NodeType::NODE_OP, Operator::OP_POW};
                    break;
                default:
                    throw derivative_exception();
            }
        }
        else {
            char* double_end = nullptr;
            double value = strtod(ch, &double_end);

            node->data = NodeData{NodeType::NODE_CONST, value};

            ch = --double_end;
        }
    }

    return *root->left;
}


Node deserialize(std::string expression, char var = 'x') {
    return deserialize(expression.c_str(), var);
}

std::string eat_extra_zeros(const std::string& expression) {
    auto point = expression.find('.');
    if (point == std::string::npos)
        return expression;
    unsigned end = expression.size() - 1;
    for (; end >= point; --end)
        if (expression[end] != '0')
            break;
    if (expression[end] == '.')
        --end;
    return expression.substr(0, end + 1);
}



std::string serialize(const Node& root) {
    std::string ans = "( ";
    if (root.left) {
        ans += serialize(*root.left);
        switch(OP(root)) {
            case Operator::OP_PLUS:
                ans += " + ";
                break;
            case Operator::OP_MINUS:
                ans += " - ";
                break;
            case Operator::OP_MUL:
                ans += " * ";
                break;
            case Operator::OP_DIV:
                ans += " / ";
                break;
            case Operator::OP_POW:
                ans += " ^ ";
                break;
            default:
                throw derivative_exception();
        }
        ans += serialize(*root.right);
    }
    else if (IS_VAR(root)) {
        switch(VAR(root)) {
            case Variable::VAR_X:
                ans += 'x';
                break;
            case Variable::VAR_Y:
                ans += 'y';
                break;
            case Variable::VAR_Z:
                ans += 'z';
                break;
            default:
                throw derivative_exception();
        }
    }
    else /* IS_CONST(root) */ { 
        ans += eat_extra_zeros(std::to_string(VALUE(root)));
    }
    return ans + " )";
}


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


    if (OP(root) == Operator::OP_POW) {
        if (!IS_OP(*root.left))
            return left + "^{" + right + "}";
        return "(" + left + ")^{" + right + "}";
    }

    std::string ans = "";


    if (IS_OP(*root.left) && OP(root) != Operator::OP_PLUS && OP(root) != Operator::OP_MINUS &&
        OP(*root.left) != Operator::OP_DIV)
        ans += "\\left(" + left + "\\right)";
    else if (IS_CONST(*root.left) && EQUAL(VALUE(*root.left), 0));
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

    if (IS_OP(*root.right) && OP(root) != Operator::OP_PLUS)
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
                      "\\begin{document}\n";
    ans += latex_code + "\n";
    return ans + "\\end{document}";
}




// POEM



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
    "Do you really have nothing to do?"
};


Node derivative_poem_step(const Node& node, std::string& poem) {

#define L *node.left
#define R *node.right
#define dL derivative_poem_step(L, poem)
#define dR derivative_poem_step(R, poem)
#define cL Node(L)
#define cR Node(R)

    if (IS_CONST(node))
        return Node(nullptr, NodeData{NodeType::NODE_CONST, 0}, nullptr);
    if (IS_VAR(node))
        return Node(nullptr, NodeData{NodeType::NODE_CONST, 1}, nullptr);                                      // TODO
    
    switch (OP(node)) {
        case Operator::OP_PLUS: {
            auto ans = dL + dR;
            poem += linking_words[rand() % linking_words.size()] + 
                    "\n\n$\\frac{d}{dx}\\left(" + to_latex(node) + "\\right) = " + 
                    to_latex(ans) + "$\n\n";
            return ans;
        }
        case Operator::OP_MINUS: {
            auto ans = dL - dR;
            poem += linking_words[rand() % linking_words.size()] + 
                    "\n\n$\\frac{d}{dx}\\left(" + to_latex(node) + "\\right) = " + 
                    to_latex(ans) + "$\n\n";
            return ans;
        }
        case Operator::OP_MUL: {
            auto ans = dL * cR + cL * dR;
            poem += linking_words[rand() % linking_words.size()] + 
                    "\n\n$\\frac{d}{dx}\\left(" + to_latex(node) + "\\right) = " + 
                    to_latex(ans) + "$\n\n";
            return ans;
        }
        case Operator::OP_DIV: {
            auto ans = (dL * cR - cL * dR) / (cR * cR);
            poem += linking_words[rand() % linking_words.size()] + 
                    "\n\n$\\frac{d}{dx}\\left(" + to_latex(node) + "\\right) = " + 
                    to_latex(ans) + "$\n\n";
            return ans;
        }

        case Operator::OP_POW: {
            if (!IS_CONST(R)) /* coming soon */
                throw derivative_exception();
            
            auto ans = R * (L ^ Node(nullptr, {NodeType::NODE_CONST, VALUE(R) - 1}, nullptr));
            poem += linking_words[rand() % linking_words.size()] + 
                    "\n\n$\\frac{d}{dx}\\left(" + to_latex(node) + "\\right) = " + 
                    to_latex(ans) + "$\n\n";
            return ans;
        }
        default:
            throw derivative_exception();
    }

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
