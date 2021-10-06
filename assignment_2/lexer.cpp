#include "lexer.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

void Lexer::AddToken(int line, TokenClass tc, const std::string &lexeme) {
    tks_.push_back({line, tc, lexeme});
}

void Lexer::PrintTokens(char *name) {
    std::cout << "#name \"" << name << "\"" << std::endl;
    for (const auto &tk : tks_) {
        std::cout << "#" << tk.line << " ";
        if (tabs.T2P.find(tk.tc) != tabs.T2P.end()) {
            std::cout << tabs.T2P.at(tk.tc) << " ";
        }
        std::cout << tk.lexeme << std::endl;
    }
}

std::string StrToLower(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return str;
}

std::string StrToUpper(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(),
                   [](unsigned char c) { return std::toupper(c); });
    return str;
}

bool IsBoolConst(const std::string &str) {
    std::string s_low = StrToLower(str);
    return std::islower(str[0]) && (s_low == "true" || s_low == "false");
}

bool IsSpaceSymbol(char lit) {
    return lit == ' ' || lit == '\b' || lit == '\t' || lit == '\013' ||
           lit == '\015' || lit == '\f';
}

void GetErrorSpace(Lexer &lex, char lit, int line) {
    std::string str;
    if (tabs.ASCII.find(lit) != tabs.ASCII.end()) {
        str = tabs.ASCII.at(lit);
    } else {
        str = std::string(1, lit);
    }
    if (lit == '\\') {
        str += lit;
    }
    lex.AddToken(line, TokenClass::ERROR, "\"" + str + "\"");
}

void GetTokenAfEq(Lexer &lex, std::ifstream &file, char lit, int line) {
    if (file.peek() == '>') {
        file.get(lit);
        lex.AddToken(line, TokenClass::DARROW, "DARROW");
    } else {
        lex.AddToken(line, TokenClass::EQ, "'='");
    }
}

void GetTokenAfLt(Lexer &lex, std::ifstream &file, char lit, int line) {
    if (file.peek() == '-') {
        file.get(lit);
        lex.AddToken(line, TokenClass::ASSIGN, "ASSIGN");
    } else if (file.peek() == '=') {
        file.get(lit);
        lex.AddToken(line, TokenClass::LE, "LE");
    } else {
        lex.AddToken(line, TokenClass::LT, "'<'");
    }
}

static bool GetFrstComments(Lexer &lex, std::ifstream &file, char lit,
                            int &line) {
    if (file.peek() == '-') {
        std::string str;
        std::getline(file, str);
        ++line;
    } else {
        lex.AddToken(line, TokenClass::MINUS, "'-'");
    }
    return true;
}

static bool GetScdComments(Lexer &lex, std::ifstream &file, char lit, int &line,
                           int &flag_cmnt) {
    if (lit == '(') {
        if (file.peek() == '*') {
            file.get();
            ++flag_cmnt;
        } else if (flag_cmnt == 0) {
            lex.AddToken(line, TokenClass::LPAREN, "'('");
        }
    } else if (lit == '*') {
        if (file.peek() == ')') {
            file.get(lit);
            if (flag_cmnt > 0) {
                --flag_cmnt;
            } else {
                lex.AddToken(line, TokenClass::ERROR, "\"Unmatched *)\"");
            }
        } else if (flag_cmnt == 0) {
            lex.AddToken(line, TokenClass::MULT, "'*'");
        }
    }
    return true;
}

bool GetComments(Lexer &lex, std::ifstream &file, char lit, int &line,
                 int &flag_cmnt) {
    if (flag_cmnt == 0 && lit == '-') {
        return GetFrstComments(lex, file, lit, line);
    } else if (lit == '(' || lit == '*') {
        return GetScdComments(lex, file, lit, line, flag_cmnt);
    } else if (flag_cmnt > 0) {
        return true;
    }
    return false;
}

static void GetNullToken(Lexer &lex, std::ifstream &file, char lit, int &line) {
    if (file.peek() == 0 && lit == '\\') {
        lex.AddToken(line, TokenClass::ERROR,
                     "\"String contains escaped null character.\"");
    } else {
        lex.AddToken(line, TokenClass::ERROR,
                     "\"String contains null character.\"");
    }
    while (!file.eof() && lit != '\"' && lit != '\n') {
        file.get(lit);
    }
    if (lit == '\n') {
        ++line;
    }
}

static void GetSpecSymbol(Lexer &lex, std::ifstream &file, char &lit, int &line,
                          std::string &str, int &spec_sym, bool &new_line) {
    if (lit == '\\') {
        char next_lit = file.peek();
        if (tabs.ASCII.find(next_lit) != tabs.ASCII.end()) {
            file.get(lit);
            if (lit == '\n' || lit == '\r') {
                ++line;
            }
            str += tabs.ASCII.at(lit);
            new_line = false;
            ++spec_sym;
        } else if (next_lit == 'b' || next_lit == 't' || next_lit == 'n' ||
                   next_lit == 'f' || next_lit == '\\' || next_lit == '\"') {
            file.get(lit);
            str += '\\';
            str += lit;
            new_line = false;
            ++spec_sym;
        } else {
            new_line = true;
        }
    } else {
        str += lit;
    }
}

static void EndStrConst(Lexer &lex, std::ifstream &file, char lit, int &line,
                        std::string &str, int &spec_sym, bool &new_line) {
    if (file.peek() == '\n') {
        if (new_line && (lit == '\\' || lit == '\r')) {
            ++line;
            file.get(lit);
            str += "\\n";
            ++spec_sym;
            GetStrConst(lex, file, lit, line, str, spec_sym);
        } else {
            lex.AddToken(line + 1, TokenClass::ERROR,
                         "\"Unterminated string constant\"");
        }
    } else if (file.peek() == '\"') {
        str += '\"';
        file.get(lit);
        if (str.length() - spec_sym > 1026) {
            lex.AddToken(line, TokenClass::ERROR,
                         "\"String constant too long\"");
        } else {
            lex.AddToken(line, TokenClass::STR_CONST, str);
        }
    } else {
        lex.AddToken(line, TokenClass::ERROR, "\"EOF in string constant\"");
    }
}

void GetStrConst(Lexer &lex, std::ifstream &file, char lit, int &line,
                 std::string str, int spec_sym) {
    bool new_line = false;
    while (file.peek() != '\"' && file.peek() != '\n' && !file.eof()) {
        file.get(lit);

        if (lit == 0 || file.peek() == 0) {
            GetNullToken(lex, file, lit, line);
            return;
        }
        if (tabs.ASCII.find(lit) != tabs.ASCII.end() && lit != '\n') {
            ++spec_sym;
            str += tabs.ASCII.at(lit);
            new_line = false;
        } else if (lit == '\\') {
            GetSpecSymbol(lex, file, lit, line, str, spec_sym, new_line);
        } else {
            str += lit;
        }
    }
    EndStrConst(lex, file, lit, line, str, spec_sym, new_line);
}

void GetNumToken(Lexer &lex, std::ifstream &file, char lit, int line) {
    std::string num(1, lit);
    while (std::isdigit(file.peek())) {
        file.get(lit);
        num += lit;
    }
    lex.AddToken(line, TokenClass::INT_CONST, num);
}

void GetStrToken(Lexer &lex, std::ifstream &file, char lit, int line) {
    std::string str(1, lit);
    while (std::isalpha(file.peek()) || std::isdigit(file.peek()) ||
           file.peek() == '_') {
        file.get(lit);
        str += lit;
    }

    std::string str_low = StrToLower(str), str_up = StrToUpper(str);

    if (tabs.KEYWORDS.find(str_up) != tabs.KEYWORDS.end()) {
        lex.AddToken(line, tabs.KEYWORDS.at(str_up), str_up);
    } else if (IsBoolConst(str)) {
        lex.AddToken(line, TokenClass::BOOL_CONST, str_low);
    } else if (std::isupper(str[0])) {
        lex.AddToken(line, TokenClass::TYPEID, str);
    } else {
        lex.AddToken(line, TokenClass::OBJECTID, str);
    }
}

void GetLexems(Lexer &lex, std::ifstream &file) {
    int line = 1;
    char lit;
    int flag_cmnt = 0;

    while (!file.eof() && file.peek() != -1) {
        file.get(lit);
        if (lit == '\n') {
            ++line;
            continue;
        }

        if (GetComments(lex, file, lit, line, flag_cmnt)) {
            continue;
        }

        if (lit == '\"') {
            GetStrConst(lex, file, lit, line);
        } else if (lit == '=') {
            GetTokenAfEq(lex, file, lit, line);
        } else if (lit == '<') {
            GetTokenAfLt(lex, file, lit, line);
        } else if (tabs.SYMBOLS.find(lit) != tabs.SYMBOLS.end()) {
            std::string str = "'" + std::string(1, lit) + "'";
            lex.AddToken(line, tabs.SYMBOLS.at(lit), str);
        } else if (std::isalpha(lit)) {
            GetStrToken(lex, file, lit, line);
        } else if (std::isdigit(lit)) {
            GetNumToken(lex, file, lit, line);
        } else if (!IsSpaceSymbol(lit)) {
            GetErrorSpace(lex, lit, line);
        }
    }

    if (flag_cmnt > 0) {
        lex.AddToken(line, TokenClass::ERROR, "\"EOF in comment\"");
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        return EXIT_FAILURE;
    }

    Lexer lex;
    std::ifstream file(argv[1]);

    GetLexems(lex, file);
    lex.PrintTokens(argv[1]);

    file.close();
    return 0;
}
