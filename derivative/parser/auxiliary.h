#pragma once

#include <string>


static std::string eat_extra_zeros(const std::string& expression) {
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


template <typename T>
static void* get_address(T new_value) {
static T value;
    
    value = new_value;
    return (void*)&value;
}
