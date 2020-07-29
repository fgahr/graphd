#include <graphd/input/token.hpp>

#include <cctype>
#include <sstream>
#include <stdexcept>
#include <vector>

static std::string downcase(std::string s) {
    std::ostringstream buffer;
    for (auto c : s) {
        if (std::isupper(c)) {
            buffer << (char)std::tolower(c);
        } else {
            buffer << c;
        }
    }
    return buffer.str();
}

static std::vector<std::string> supported_keywords{"graph", "strict"};

static bool is_supported_keyword(std::string s) {
    s = downcase(s);
    for (auto kw : supported_keywords) {
        if (s == kw) {
            return true;
        }
    }
    return false;
}

static std::vector<std::string> unsupported_keywords{"node", "edge", "digraph",
                                                     "subgraph"};

static bool is_unsupported_keyword(std::string s) {
    s = downcase(s);
    for (auto kw : unsupported_keywords) {
        if (s == kw) {
            throw std::runtime_error{"unsupported keyword: " + s};
        }
    }

    return false;
}

static const char fixed_tokens[] = ";,{}[]=";

static bool is_fixed_token(int c) {
    for (char tok : fixed_tokens) {
        if (c == tok)
            return true;
    }
    return false;
}

namespace graphd::input {

bool Token::is_identifier() {
    switch (this->type) {
    case TokenType::NAME:
    case TokenType::NUMERAL:
        return true;
    default:
        return false;
    }
}

Token Token::from(char c) {
    switch (c) {
    case ';':
        return Token{TokenType::SEMICOLON, ""};
    case ',':
        return Token{TokenType::COMMA, ""};
    case '{':
        return Token{TokenType::OPENING_BRACE, ""};
    case '}':
        return Token{TokenType::CLOSING_BRACE, ""};
    case '[':
        return Token{TokenType::OPENING_SQUARE_BRACKET, ""};
    case ']':
        return Token{TokenType::CLOSING_SQUARE_BRACKET, ""};
    case '=':
        return Token{TokenType::EQUAL_SIGN, ""};
    default:
        throw std::logic_error{"not a valid token " + std::to_string(c)};
    }
}

Token Token::from(std::string s) {
    if (s.size() == 1 && is_fixed_token(s.front())) {
        return Token::from(s.front());
    }

    if (is_supported_keyword(s)) {
        return Token{TokenType::KEYWORD, downcase(s)};
    } else if (is_unsupported_keyword(s)) {
        throw std::runtime_error{"unsupported keyword: " + s};
    } else {
        return Token{TokenType::NAME, s};
    }
}

Tokenizer::Tokenizer(std::istream &in) : in{in} {}

std::string Tokenizer::read_string() {
    std::ostringstream buffer;
    std::string str{""};

    while (true) {
        int c = in.get();
        switch (c) {
        case EOF:
            throw std::runtime_error{"encountered EOF while parsing string"};
        case '\\':
            buffer << (char)in.get();
            continue;
        case '"':
            str = buffer.str();
            if (str.empty()) {
                throw std::runtime_error{"empty string"};
            }
            return str;
        default:
            buffer << (char)c;
        }
    }
}

std::string Tokenizer::read_name() {
    std::ostringstream buffer;

    while (true) {
        int c = in.get();
        if (std::isalnum(c)) {
            buffer << (char)c;
        } else if (c == EOF) {
            return buffer.str();
        } else {
            // semicolon, comma, etc.
            in.putback(c);
            std::string name = buffer.str();
            if (name.empty()) {
                throw std::logic_error{"attempting to parse empty name"};
            }
            return name;
        }
    }
}

std::string Tokenizer::read_numeral() {
    std::ostringstream buffer;

    bool has_integer_part = false;
    bool seen_decimal_point = false;

    while (true) {
        int c = in.get();
        if (c == '.') {
            if (seen_decimal_point) {
                throw std::runtime_error{"invalid numeral: " + buffer.str() +
                                         "."};
            }
            seen_decimal_point = true;
            buffer << '.';
        } else if (std::isdigit(c)) {
            if (!seen_decimal_point) {
                has_integer_part = true;
            }
            buffer << (char)c;
        } else {
            if (!(seen_decimal_point || has_integer_part)) {
                std::logic_error{"attempting to parse empty numeral"};
            }
            in.putback(c);
            return buffer.str();
        }
    }
}

Token Tokenizer::next_token() {
    // NOTE: Error handling not bulletproof. In particular, badbit needs to be
    // checked after putback.
    int c;
    while ((c = in.get()) != EOF) {
        if (std::isspace(c))
            continue;
        if (is_fixed_token(c))
            return Token::from((char)c);
        if (c == '"')
            return Token{TokenType::NAME, read_string()};
        if (c == '-') {
            c = in.get();
            if (c == EOF) {
                throw std::runtime_error{"unexpected end of input"};
            } else if (c == '-') {
                return Token{TokenType::UNDIRECTED_EDGE, ""};
            } else if (c == '>') {
                throw std::runtime_error{
                    "unexpected token: -> (only undirected graphs "
                    "are supported at this time)"};
                // return Token{TokenType::DIRECTED_EDGE, ""};
            } else {
                in.putback(c);
                return Token{TokenType::NUMERAL, "-" + read_numeral()};
            }
        }

        if (std::isalpha(c) || c == '_') {
            in.putback(c);
            std::string name = read_name();
            return Token::from(name);
        } else if (c == '.' || std::isdigit(c)) {
            in.putback(c);
            return Token{TokenType::NUMERAL, read_numeral()};
        } else {
            throw std::runtime_error{"unexpected input byte: " +
                                     std::to_string((char)c)};
        }
    }

    return Token{TokenType::EOI, ""};
}
} // namespace graphd::input
