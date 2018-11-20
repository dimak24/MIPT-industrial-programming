#include <string>
#include <variant>
#include <exception>
#include <math.h>
#include <cstdlib>
#include <algorithm>
#include <string.h>



#define L *node.left
#define R *node.right
#define dL derivative(L)
#define dR derivative(R)
#define cL Node(L)
#define cR Node(R)
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
    OP_DIV
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



Node derivative(const Node& node) {
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
        default:
            throw derivative_exception();
    }
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
        else if (*ch == '+' || *ch == '-' || *ch == '*' || *ch == '/') {
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


Node deserialize(std::string expression) {
    return deserialize(expression.c_str());
}



std::string serialize(Node root) {
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
        ans += std::to_string(VALUE(root));
    }
    return ans + " )";
}


char find_nearest_op(const std::string& str, unsigned index) {
    while (index < str.size()) {
        if (str[index] == '+' || str[index] == '-' || str[index] == '*' || str[index] == '/')
            return str[index];
        ++index;
    }
    throw derivative_exception();
}

unsigned find_closing_paranthesys(const std::string& expression, unsigned opening) {
    unsigned balance = 0;
    for (unsigned i = opening; i < expression.size(); ++i) {
        if (expression[i] == '(')
            ++balance;
        else if (expression[i] == ')')
            --balance;
        if (!balance)
            return i;
    }
    throw derivative_exception();
}

std::string reduce_zero(std::string str) {
    unsigned j = str.size() - 1;
    for (; j >= 0; --j)
        if (str[j] == '.')
            break;
    return str.substr(0, j);
}


std::string reduce_parantheses(std::string expression) {
    if (expression[0] == '(') {
        unsigned j = find_closing_paranthesys(expression, 0);
        if (j == expression.size() - 1)
            return reduce_parantheses(expression.substr(1, expression.size() - 2));

        std::string left = reduce_parantheses(expression.substr(1, j - 1));
        char op = find_nearest_op(expression, j);
        for (; j < expression.size(); ++j)
            if (expression[j] == '(')
                break;
            else if (j == expression.size() - 1)
                throw derivative_exception();
        unsigned k = find_closing_paranthesys(expression, j);
        std::string right = reduce_parantheses(expression.substr(j + 1, k - j - 1));
        if (op == '+')
            return left + "+" + right;
        if (op == '-')
            return left + "-(" + right + ")";
        if (op == '*')
            return "(" + left + ")(" + right + ")";
        if (op == '/')
            return "\\frac{" + left + "}{" + right + "}";
        throw derivative_exception();
    }
    if (expression[0] == '0' && EQUAL(std::atof(expression.c_str()), 0))
        return "";
    if (expression[0] == 'x')
        return expression;
    return reduce_zero(expression); 
}


std::string to_latex(std::string expression) {
    expression.erase(std::remove(expression.begin(), expression.end(), ' '), expression.end());
    std::string ans = "\\documentclass{article}\n\\begin{document}\n$$";
    ans += reduce_parantheses(expression);
    return ans + "$$\n\\end{document}";
}
