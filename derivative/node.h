#pragma once



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
    OP_POW,
    OP_LN
};


enum NodeType {
    NODE_UNDEFINED,
    NODE_VAR,
    NODE_OP,
    NODE_CONST
};


struct NodeData {
    NodeType type;
    std::variant<Variable, Operator, double> value = 0;
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

    Node& operator=(const Node& another) {
        left = another.left ? new Node(*another.left) : nullptr;
        data = another.data;
        right = another.right ? new Node(*another.right) : nullptr;
        parent = nullptr;

        return *this;
    }

    Node(Node* parent) 
        : left(nullptr), 
          data({NODE_UNDEFINED}),
          right(nullptr), 
          parent(parent) {}

    Node() : Node(nullptr) {}


    ~Node() {
        if (left)
            delete left;
        if (right)
            delete right;
    }
};



#define IS_OP(node) ((node).data.type == NodeType::NODE_OP)
#define IS_VAR(node) ((node).data.type == NodeType::NODE_VAR)
#define IS_CONST(node) ((node).data.type == NodeType::NODE_CONST)
#define IS_UNDEFINED(node) ((node).data.type == NodeType::NODE_UNDEFINED)
#define MAKE_CONST(value) Node(nullptr, {NodeType::NODE_CONST, value}, nullptr)
// binary
auto MAKE_OP(const Node& left, Operator op, const Node& right) {
    return Node(new Node(left), {NodeType::NODE_OP, op}, new Node(right));
}
//unary
auto MAKE_OP(Operator op, const Node& right) {
    return Node(nullptr, {NodeType::NODE_OP, op}, new Node(right));
}
#define OP(node) (std::get<Operator>((node).data.value))
#define VAR(node) (std::get<Variable>((node).data.value))
#define VALUE(node) (std::get<double>((node).data.value))
#define EPS 1e-5
#define EQUAL(value1, value2) (fabs((value1) - (value2)) < EPS)



Node operator+(const Node& left, const Node& right) {
    if (IS_CONST(left) && IS_CONST(right))
        return MAKE_CONST(VALUE(left) + VALUE(right));

    if (IS_CONST(left) && EQUAL(VALUE(left), 0))
        return Node(right);

    if (IS_CONST(right) && EQUAL(VALUE(right), 0))
        return Node(left);
    
    return MAKE_OP(left, Operator::OP_PLUS, right);
}


Node operator-(const Node& left, const Node& right) {
    if (IS_CONST(left) && IS_CONST(right))
        return MAKE_CONST(VALUE(left) - VALUE(right));

    if (IS_CONST(right) && EQUAL(VALUE(right), 0))
        return Node(left);
    
    return MAKE_OP(left, Operator::OP_MINUS, right);
}


Node operator*(const Node& left, const Node& right) {
    if (IS_CONST(left) && IS_CONST(right))
        return MAKE_CONST(VALUE(left) * VALUE(right));

    if ((IS_CONST(left) && EQUAL(VALUE(left), 0)) || (IS_CONST(right) && EQUAL(VALUE(right), 0)))
        return MAKE_CONST(0);

    if (IS_CONST(left) && EQUAL(VALUE(left), 1))
        return Node(right);

    if (IS_CONST(right) && EQUAL(VALUE(right), 1))
        return Node(left);
    
    return MAKE_OP(left, Operator::OP_MUL, right);
}


Node operator/(const Node& left, const Node& right) {
    if (IS_CONST(left) && IS_CONST(right))
        return MAKE_CONST(VALUE(left) / VALUE(right));

    if (IS_CONST(left) && EQUAL(VALUE(left), 0))
        return MAKE_CONST(0);

    if (IS_CONST(right) && EQUAL(VALUE(right), 1))
        return Node(left);

    if (IS_VAR(left) && IS_VAR(right) && VAR(left) == VAR(right))
        return MAKE_CONST(1);
    
    return MAKE_OP(left, Operator::OP_DIV, right);
}


Node operator^(const Node& left, const Node& right) {
    if (IS_CONST(left) && IS_CONST(right))
        return MAKE_CONST(pow(VALUE(left), VALUE(right)));

    if (IS_CONST(left) && EQUAL(VALUE(left), 0))
        return MAKE_CONST(0);

    if ((IS_CONST(right) && EQUAL(VALUE(right), 0)) || (IS_CONST(left) && EQUAL(VALUE(left), 1)))
        return MAKE_CONST(1);

    return MAKE_OP(left, Operator::OP_POW, right);
}


Node ln(const Node& node) {
    if (IS_CONST(node))
        return MAKE_CONST(log(VALUE(node)));

    return MAKE_OP(Operator::OP_LN, node);
}


Node& operator+=(Node& left, const Node& right) {
    return left = left + right;
}

Node& operator-=(Node& left, const Node& right) {
    return left = left - right;
}

Node& operator*=(Node& left, const Node& right) {
    return left = left * right;
}

Node& operator/=(Node& left, const Node& right) {
    return left = left / right;
}

Node& operator^=(Node& left, const Node& right) {
    return left = left ^ right;
}
