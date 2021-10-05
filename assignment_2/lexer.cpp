#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

enum class TokenClass {
    TYPEID = 0,
    OBJECTID,

    BOOL_CONST,
    INT_CONST,
    STR_CONST,

    DARROW,         /* => */
    ASSIGN,         /* <- */
    LE,             /* <= */
    PLUS,           /* + */
    MINUS,          /* - */
    MULT,           /* * */
    DIV,            /* / */
    LPAREN,         /* ( */         
    RPAREN,         /* ) */
    LBRACE,         /* { */
    RBRACE,         /* } */
    DOT,            /* . */
    COLON,          /* : */
    COMMA,          /* , */
    SEMI,           /* ; */
    EQ,             /* = */
    NEG,            /* ~ */
    LT,             /* < */
    AT,             /* @ */

    CLASS,
    IN,
    INHERITS,
    ISVOID,
    LET,
    NEW,
    OF,
    NOT,
    LOOP,
    POOL,
    CASE,
    ESAC,
    IF,
    THEN,
    ELSE,
    FI,
    WHILE,

    ERROR,
};


struct Tabs {
    const std::unordered_map<char, TokenClass> SYMBOLS = {
        {'+', TokenClass::PLUS},   {'/', TokenClass::DIV},
        {')', TokenClass::RPAREN}, {'{', TokenClass::LBRACE},
        {'}', TokenClass::RBRACE}, {'.', TokenClass::DOT},
        {':', TokenClass::COLON},  {',', TokenClass::COMMA},
        {';', TokenClass::SEMI},   {'~', TokenClass::NEG},
        {'@', TokenClass::AT},
    };
    const std::unordered_map<std::string, TokenClass> KEYWORDS = {
        {"CLASS", TokenClass::CLASS},
        {"IN", TokenClass::IN},
        {"INHERITS", TokenClass::INHERITS},
        {"ISVOID", TokenClass::ISVOID},
        {"LET", TokenClass::LET},
        {"NEW", TokenClass::NEW},
        {"OF", TokenClass::OF},
        {"NOT", TokenClass::NOT},
        {"LOOP", TokenClass::LOOP},
        {"POOL", TokenClass::POOL},
        {"CASE", TokenClass::CASE},
        {"ESAC", TokenClass::ESAC},
        {"IF", TokenClass::IF},
        {"THEN", TokenClass::THEN},
        {"ELSE", TokenClass::ELSE},
        {"FI", TokenClass::FI},
        {"WHILE", TokenClass::WHILE},
    };
    const std::unordered_map<TokenClass, std::string> T2P{
        {TokenClass::OBJECTID, "OBJECTID"},
        {TokenClass::TYPEID, "TYPEID"},
        {TokenClass::BOOL_CONST, "BOOL_CONST"},
        {TokenClass::INT_CONST, "INT_CONST"},
        {TokenClass::STR_CONST, "STR_CONST"},
        {TokenClass::ERROR, "ERROR"},
    };
} tabs;

struct Token {
    int line;
    TokenClass tc;
    std::string lexeme;
};

class Lexer {
    std::vector<Token> tks_{};

   public:
    void addToken(int line, TokenClass tc, const std::string &lexeme) {
        tks_.push_back({line, tc, lexeme});
    }

    void printTokens(char *name) {
        std::cout << "#name \"" << name << "\"" << std::endl; 
        for (const auto& tk : tks_) {
            std::cout << "#" << tk.line << " ";
            if (tabs.T2P.find(tk.tc) != tabs.T2P.end()) {
                std::cout << tabs.T2P.at(tk.tc) << " ";
            }
            std::cout << tk.lexeme << std::endl;
        }
    }

};

std::string str_tolower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return s;
}

std::string str_toupper(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return std::toupper(c); });
    return s;
}


bool is_bool_const(const std::string &s) {
    std::string s_low = str_tolower(s);
    return std::islower(s[0]) && (s_low == "true" || s_low == "false"); 
}


// TODO refactor this!
bool get_comments(Lexer &lex, std::ifstream &file, char lit, int &line, int &flag_cmnt) {
    if (flag_cmnt == 0 && lit == '-') {
        if (file.peek() == '-') {
            std::string str;
            std::getline(file, str);
            ++line;
        } else {
            lex.addToken(line, TokenClass::MINUS, "\'-\'");
        }
        return true;
    } else if (lit == '(') {
        if (file.peek() == '*') {
            file.get();
            ++flag_cmnt;
        } else if (flag_cmnt == 0) {
            lex.addToken(line, TokenClass::LPAREN, "\'(\'");
        }
        return true;
    } else if (lit == '*') {
        if (file.peek() == ')') {
            file.get(lit);
            if (flag_cmnt > 0) {
                --flag_cmnt;
            } else {
                lex.addToken(line, TokenClass::ERROR, "\"Unmatched *)\"");
            }
        } else if (flag_cmnt == 0) {
            lex.addToken(line, TokenClass::MULT, "\'*\'");
        }
        return true;
    } else if (flag_cmnt > 0) {
        return true;
    }
    return false;
}

// TODO refactor this!
void get_str_const(Lexer &lex, std::ifstream &file, char lit, int &line,  std::string str = "\"") {
    bool new_line = false;
    while (file.peek() != '\"' && file.peek() != '\n') {
        file.get(lit);
        if (lit == '\\') {
            char next_lit = file.peek();
            if (next_lit == 'b' || next_lit == 't' || next_lit == 'n' ||
                next_lit == 'f' || next_lit == '\\') {
                file.get(lit);
                str += '\\';
                str += lit;
                new_line = false;
            } else {
                new_line = true;
            }
        } else {
            str += lit;
        }
    }
    if (file.peek() == '\n') {
        if (new_line && lit == '\\') {
            ++line;
            file.get(lit);
            str += "\\n";
            get_str_const(lex, file, lit, line, str);
        } else {
            lex.addToken(line + 1, TokenClass::ERROR, "\"Unterminated string constant\"");
        }
    } else {
        str += '\"';
        file.get(lit);
        lex.addToken(line, TokenClass::STR_CONST, str);
    }
}

void get_num_token(Lexer &lex, std::ifstream &file, char lit, int line) {
    std::string num(1, lit);
    while (std::isdigit(file.peek())) {
        file.get(lit);
        num += lit;
    }
    lex.addToken(line, TokenClass::INT_CONST, num);
}

void get_str_token(Lexer &lex, std::ifstream &file, char lit, int line) {
    std::string str(1, lit);
    while (std::isalpha(file.peek()) || std::isdigit(file.peek()) || lit == '_') {
        file.get(lit);
        str += lit;
    }

    std::string str_low = str_tolower(str), str_up = str_toupper(str);

    if (tabs.KEYWORDS.find(str_up) != tabs.KEYWORDS.end()) {
        lex.addToken(line, tabs.KEYWORDS.at(str_up), str_up);
    } else if (is_bool_const(str)) {
        lex.addToken(line, TokenClass::BOOL_CONST, str_low);
    } else if (std::isupper(str[0])){
        lex.addToken(line, TokenClass::TYPEID, str);
    } else {
        lex.addToken(line, TokenClass::OBJECTID, str);
    }
}

void GetLexems(Lexer &lex, std::ifstream &file) {
    int line = 1;
    char lit;
    int flag_cmnt = 0;
    std::string sub_str;

    while (!file.eof()) {
        file.get(lit);
        if (lit == '\n') {
            ++line;
            continue;
        }

        if (get_comments(lex, file, lit, line, flag_cmnt)) {
            continue;
        }

        if (lit == '\"') {
            get_str_const(lex, file, lit, line);
        } else if (lit == '=') {
            if (file.peek() == '>') {
                file.get(lit);
                lex.addToken(line, TokenClass::DARROW, "DARROW");
            } else {
                lex.addToken(line, TokenClass::EQ, "\'=\'");
            }
        } else if (lit == '<') {
             if (file.peek() == '-') {
                file.get(lit);
                lex.addToken(line, TokenClass::ASSIGN, "ASSIGN");
            } else if (file.peek() == '=') {
                file.get(lit);
                lex.addToken(line, TokenClass::LE, "LE"); 
            } else {
                lex.addToken(line, TokenClass::LT, "\'<\'");
            }
        } else if (tabs.SYMBOLS.find(lit) != tabs.SYMBOLS.end()) {
            std::string str = "\'" + std::string(1, lit) + "\'";
            lex.addToken(line, tabs.SYMBOLS.at(lit), str);
        } else if (std::isalpha(lit)) {
            get_str_token(lex, file, lit, line);
        } else if (std::isdigit(lit)) {
            get_num_token(lex, file, lit, line);
        }
    }

    if (flag_cmnt > 0) {
        lex.addToken(line, TokenClass::ERROR, "\"EOF in comment\"");
    }     
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        return 1;
    }

    Lexer lex;
    std::ifstream file(argv[1]);

    GetLexems(lex, file);
    lex.printTokens(argv[1]);

    file.close();
    return 0;
}
