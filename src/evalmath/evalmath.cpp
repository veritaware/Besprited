//This file is licensed under MIT-0
//Copyright 2026 Ribbon-otter
//It is the implementation half of evalmath, a simple math string interpeter
#include <cassert>
#include <expected>
#include <string>
#include <string_view>
#include <optional>
#include <variant>
#include <vector>
#include <charconv>
#include <format>
#include <print>

#include "evalmath.h"

// uncomment to enable print statements and asserts helpful for development only
// #define EVALMATH_DEBUG 
#ifdef EVALMATH_DEBUG
    #define EVALMATH_PRINTLN(...) std::println( __VA_ARGS__)
    #define EVALMATH_PRINT(...) std::print(__VA_ARGS__)
    #define EVALMATH_ASSERT(...) assert( __VA_ARGS__)
    #define EVALMATH_PRINT_STACK(stack,tag) print_stack((stack), (tag))
#else
    #define EVALMATH_PRINTLN(...)
    #define EVALMATH_PRINT(...)
    #define EVALMATH_ASSERT(...)
    #define EVALMATH_PRINT_STACK(stack,tag)
#endif

using size_t = std::size_t;

namespace evalmath {
    namespace details {
        bool static is_digit (char a) {
            return '0' <= a && a <= '9';
        }
        bool static is_whitespace (char a) {
            return ' ' == a;
        }
        bool static is_operator (char a) {
            return a == '+'
                || a == '*'
                || a == '-'
                || a == '/';
        }
        enum struct token_type {
            NUMBER,
            OPERATOR,
            RIGHT_PARENTHESES,
            LEFT_PARENTHESES,
            INVALID,
            END_OF_TEXT

        };
#ifdef EVALMATH_DEBUG
        std::string_view static to_sv(const token_type& a) {
            switch (a) {
                case token_type::NUMBER: return "{num}";
                case token_type::OPERATOR: return "{op}";
                case token_type::RIGHT_PARENTHESES: return "{ ) }";
                case token_type::LEFT_PARENTHESES: return "{ ( }";
                case token_type::INVALID: return "{invalid}";
                case token_type::END_OF_TEXT: return "{EOT}";
                default: return "{unknown token type}";
            }
        }
#endif
        
        struct token {
            token_type type;
            union {
                double value_num;
                char value_char; 
                std::string_view value_sv;
            };
        };
        
        double static parse_num(std::string_view num_str) {
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
#endif
            double value = 0;
            auto [ptr, ec] =
                std::from_chars(num_str.data(), num_str.data() + num_str.size(), value);
            EVALMATH_PRINTLN("\tparsing number '{}' ", num_str);
            assert(ec == std::errc()); //there should never be an error
            assert(ptr == num_str.data() + num_str.size()); //and we use up the entire 
                                                            //token value
            //silence unused value warnings on non-debug builds
            (void)ptr;
            (void)ec;
#ifdef __clang__
#pragma clang diagnostic pop
#endif
            return value;
        }
        
        ///returns a token if there are tokens left
        std::pair<std::optional<token>, size_t> static next_token(std::string_view text) {
            if (text.empty()) return {};
            auto all_start = text.begin(); //text including whitespace
            while (!text.empty() && is_whitespace(text.front())) text.remove_prefix(1);
            if (text.empty()) return {};
            
            auto token_start = text.begin(); //text after skipping whitespace
            char start_char = text.front();
            token_type type;
            text.remove_prefix(1);
            if (start_char == '.' && !text.empty() && is_digit(text.front())) {
                //we are a number starting with .
                type = token_type::NUMBER;
                while (!text.empty() && is_digit(text.front())) {
                    text.remove_prefix(1);
                }
            } else if (is_digit(start_char) 
                || ( start_char == '-' && !text.empty() && is_digit(text.front()) )
                //we will assume we are a negative number in the case of '-'
                //and then in the reducer we will promote negative numbers into
                //'-' operators if needed
                ) { // parse digit
                type = token_type::NUMBER;
                while (!text.empty() && is_digit(text.front())) {
                    text.remove_prefix(1);
                }
                if (!text.empty() && text.front() == '.') {
                    text.remove_prefix(1);
                }
                while (!text.empty() && is_digit(text.front())) {
                    text.remove_prefix(1);
                }
            } else if (is_operator(start_char)) { // parse operator
                type = token_type::OPERATOR;
            } else if ('(' == start_char) { // parse left parentheses
                type = token_type::LEFT_PARENTHESES;
            } else if (')' == start_char) { // parse right parentheses
                type = token_type::RIGHT_PARENTHESES;
            } else {
                type = token_type::INVALID;
                while (!text.empty() && !is_whitespace(text.front())) {
                    text.remove_prefix(1);
                }
            }
            auto end = text.begin();//the end of where we stopped gobbling
            //how much the caller need to advance their head
            long len = end - all_start; 
            switch (type) {
                case token_type::NUMBER:
                    return 
                        {token{.type = type, .value_num = parse_num({token_start,end})}, len};
                case token_type::OPERATOR:
                    return {token{.type = type, .value_char = start_char}, len};
                case token_type::LEFT_PARENTHESES:
                    //we don't care about the value (we just suppress the warning)
                    return {token{type, {0.0}}, len}; 
                case token_type::RIGHT_PARENTHESES:
                    //we don't care about the value (we just suppress the warning)
                    return {token{type, {0.0}}, len};
                case token_type::END_OF_TEXT:
                case token_type::INVALID: {
                    auto sv = std::string_view{token_start, end};
                    return {token{.type = type, .value_sv=sv}, len};
                }
                default:
                    assert(false); //should be unreachable
            }
            assert(false); //should be unreachable
        }
        
        int static op_precedence(char op) {
            if (op == '+' || op == '-') return 1;
            if (op == '*' || op == '/') return 2;
            assert(false);
            return 0;
        }

        int static op_precedence(token op) {
            assert(op.type == token_type::OPERATOR);
            return op_precedence(op.value_char);
        }
 
        double static eval_op(double lhs, char op, double rhs) {
            if (op == '+') {
                return lhs + rhs;
            }
            if (op == '*') {
                return lhs * rhs;
            }
            if (op == '-') {
                return lhs - rhs;
            }
            if (op == '/') {
                return lhs / rhs;
            }
            assert(false); //we shouldn't produce tokens with ops
                           //that aren't handled here
            return 0;
        }

#ifdef EVALMATH_DEBUG
        void static print_stack(std::vector<token> &stack, std::string_view tag) {
            for (const auto& i : stack) {std::print("{},", to_sv(i.type));}
            std::println("<- {}", tag);
        }
#endif
 
        ///the reduce half of the shift reduce parser
        ///returns a value only if an error occurred

        std::optional<std::string> static
        reduce(std::vector<token> &stack) {
            EVALMATH_PRINTLN("----- Begin reduce operation");
            
            //iterations of the inner loop are written assuming that 
            //assuming other patterns don't add tokens to the stack
            //and all patterns have an opportunity to match each 
            //additional token

            //however some patterns do need to add tokens onto the stack
            //(for example, the one to disambiguate 3-4 (with an operator -)
            //and 3 (-4)  (3 next to -4)
            //so queues hold such generated tokens so the other patterns can match
            // on the in between steps
            std::vector<token> queue;
            do {  
                //dribble elements out of the queue if needed.
                if (!queue.empty()) {
                    stack.push_back(queue.back());
                    queue.pop_back();
                }
                while (!stack.empty()) {
                    EVALMATH_PRINT_STACK(stack, "<- stack at start of matching");
                    
                    const auto& a = stack[stack.size() - 1];
                    if (a.type == token_type::INVALID) {
                        return std::format("invalid token: '{}'", a.value_sv);
                    }
                    if (stack.front().type == token_type::OPERATOR) {
                        return std::format("number missing before: '{}'", a.value_char);
                    }
                    if (stack.front().type == token_type::RIGHT_PARENTHESES) {
                        return std::format("extra right parentheses at the beginning");
                    }
                    
                    if (stack.size() < 2) break;
                    const auto& b = stack[stack.size() - 2];
                    if (a.type == token_type::NUMBER && b.type == token_type::NUMBER) {
                        if (a.value_num < 0) {
                            //actually the negative number is a minus operator
                            
                            //NOTE: we have to error on '3(' earlier to prevent 
                            //interpreting 3(-4) as 3-4
                            EVALMATH_PRINT_STACK(stack,
                                "stack before '-' operator transmorg");
                            token pos_val{.type=token_type::NUMBER, 
                                          .value_num= -a.value_num};
                            stack.pop_back();
                            stack.emplace_back(token{.type=token_type::OPERATOR,
                                                     .value_char='-'});
                            //we are making the stack longer,
                            //so we need to use the queue to dribble the elements on it
                            queue.emplace_back(pos_val);
                            EVALMATH_PRINT_STACK(stack, "stack after '-' operator transmorg");
                            continue; //invalidated references, so we start again
                            
                        } else {
                            return std::format("missing operator between {} and {}",
                                b.value_num, a.value_num);
                        }
                    }
                    if (a.type == token_type::END_OF_TEXT && b.type == token_type::OPERATOR) {
                        return std::format("formula ends in binary operator:", b.value_char);
                    }
                    if (a.type == token_type::OPERATOR && b.type == token_type::OPERATOR) {
                        return std::format("missing operand between: {} and {}", b.value_char, a.value_char);
                    }
                    if (a.type == token_type::RIGHT_PARENTHESES
                        && b.type == token_type::LEFT_PARENTHESES) {
                        return std::format("parentheses pair missing contents");
                    }

                    if (a.type == token_type::LEFT_PARENTHESES
                        && b.type == token_type::NUMBER) {
                        return std::format("operator needed between {} and left parentheses", b.value_num);
                    }
                    //This is included for completeness
                    //I am not sure it is possible for this error to be triggered.
                    if (a.type == token_type::NUMBER
                        && b.type == token_type::RIGHT_PARENTHESES) {
                        return std::format("operator needed between right parentheses and {}", a.value_num);
                    }
                    if (stack.size() == 2
                        && a.type == token_type::RIGHT_PARENTHESES
                        && b.type != token_type::LEFT_PARENTHESES) {
                        return std::format("missing left parentheses");
                    }
                        
                    // oh look we are done :D
                    if (stack.size() == 2
                        && a.type == token_type::END_OF_TEXT 
                        && b.type == token_type::NUMBER) {
                        stack.pop_back(); //now the stack is just the number and we are done
                        return {}; //no more reductions possible
                    }
                    if (a.type == token_type::END_OF_TEXT
                        && b.type == token_type::LEFT_PARENTHESES) {
                        return std::format("extra left parentheses on the end");
                    }
                    if (a.type == token_type::OPERATOR
                        && b.type == token_type::LEFT_PARENTHESES) {
                        return std::format("need a value between left parentheses and operator");
                    }
                    if (a.type == token_type::RIGHT_PARENTHESES
                        && b.type == token_type::OPERATOR) {
                        return std::format("need a value between right parentheses and operator");
                    }
                    
                    if (stack.size() < 3) break;
                    const auto& c = stack[stack.size() - 3];
                    if (a.type == token_type::RIGHT_PARENTHESES 
                        && b.type == token_type::NUMBER 
                        && c.type == token_type::LEFT_PARENTHESES) {
                            token result = b;
                            stack.pop_back();
                            stack.pop_back();
                            stack.pop_back();
                            stack.push_back(result);
                    }
                    if (a.type == token_type::END_OF_TEXT
                        && c.type == token_type::LEFT_PARENTHESES) {
                        return std::format("missing right parentheses");
                    }
                    
                    if (stack.size() < 4) break;
                    const auto& d = stack[stack.size() - 4];
                    //no risk that a higher priority operator can show up later
                    //since we are at the end of the text
                    if (
                        ((   a.type == token_type::END_OF_TEXT
                            || a.type == token_type::RIGHT_PARENTHESES
                            )
                        && b.type == token_type::NUMBER
                        && c.type == token_type::OPERATOR
                        && d.type == token_type::NUMBER)
                        
                        ||

                        (a.type == token_type::OPERATOR
                        && b.type == token_type::NUMBER
                        && c.type == token_type::OPERATOR
                        && d.type == token_type::NUMBER
                        && op_precedence(a) <= op_precedence(c))
                        
                        ) {
                        token end = a; //remember the end so we can put it back
                        double value_num = eval_op(d.value_num,c.value_char,b.value_num); 
                        
                        EVALMATH_PRINT("evaluated {} {} {}", 
                            d.value_num, c.value_char, b.value_char);
                        stack.pop_back();
                        stack.pop_back();
                        stack.pop_back();
                        stack.pop_back();
                        stack.emplace_back(
                            token{token_type::NUMBER, {.value_num=value_num} });
                        stack.emplace_back(end);
                        EVALMATH_PRINTLN("as {}", value_num);
                        continue; //reinitialize the a,b,c,d references 
                                  //since they are officially invalid now
                    } 
                    EVALMATH_PRINT_STACK(stack, "stack at end of match");
                    EVALMATH_PRINT_STACK(queue, "queue at end of match");
                    break;
                }
            } while (!queue.empty());
            EVALMATH_PRINTLN("----- reduce operation ended");
            return {};
        }
    }
    using namespace details;
    std::expected<double, std::string> eval(std::string_view formula) {
        //stack reduce parser
        std::vector<token> stack;
        while (true) {
            auto [maybe_token, len] = next_token(formula);
            if (!maybe_token) break;
            token this_token = *maybe_token;
            //advance the parse
            formula.remove_prefix(len);
            stack.push_back(this_token);
            if (auto maybe_error = reduce(stack)) return std::unexpected(*maybe_error);
        }
        //END_OF_TEXT tokens don't use value, but we supply a 0.0 suppress warnings
        stack.push_back(token{token_type::END_OF_TEXT, {.value_num=0.0} }); 
        if (auto maybe_error = reduce(stack)) return std::unexpected(*maybe_error);
        
        ///I should add a particular match to reduce to catch the invalid formula
        ///before these hit, but we shouldn't crash if it somehow happens in production
        EVALMATH_ASSERT(stack.size() == 1);
        EVALMATH_ASSERT(stack.front().type == token_type::NUMBER);
        if (stack.size() != 1 && stack.front().type == token_type::NUMBER) { 
            //one of our better error detecting rules didn't find
            //the specific error inside reduce
            return std::unexpected(std::format("invalid formula '{}'", formula));
        }
        EVALMATH_PRINTLN("eval returns -- {} evaluated as {}", 
            formula, stack.front().value_num);
        return stack.front().value_num;
    }
}

// vim: ts=4 sw=4 expandtab
