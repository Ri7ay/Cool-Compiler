#pragma once

#include <string>
#include <unordered_map>
#include <vector>

enum class TokenClass {
    TYPEID = 0,
    OBJECTID,

    BOOL_CONST,
    INT_CONST,
    STR_CONST,

    DARROW, /* => */
    ASSIGN, /* <- */
    LE,     /* <= */
    PLUS,   /* + */
    MINUS,  /* - */
    MULT,   /* * */
    DIV,    /* / */
    LPAREN, /* ( */
    RPAREN, /* ) */
    LBRACE, /* { */
    RBRACE, /* } */
    DOT,    /* . */
    COLON,  /* : */
    COMMA,  /* , */
    SEMI,   /* ; */
    EQ,     /* = */
    NEG,    /* ~ */
    LT,     /* < */
    AT,     /* @ */

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

    const std::unordered_map<char, std::string> ASCII{
        {'\000', "\\000"}, {'\001', "\\001"}, {'\002', "\\002"},
        {'\003', "\\003"}, {'\004', "\\004"}, {'\005', "\\005"},
        {'\006', "\\006"}, {'\007', "\\a"},   {'\010', "\\b"},
        {'\011', "\\t"},   {'\012', "\\n"},   {'\013', "\\013"},
        {'\014', "\\f"},   {'\015', "\\015"}, {'\016', "\\016"},
        {'\017', "\\017"}, {'\020', "\\020"}, {'\021', "\\021"},
        {'\022', "\\022"}, {'\023', "\\023"}, {'\024', "\\024"},
        {'\025', "\\024"}, {'\026', "\\026"}, {'\027', "\\027"},
        {'\030', "\\030"}, {'\031', "\\031"}, {'\032', "\\032"},
        {'\e', "\\033"},   {'\034', "\\034"}, {'\035', "\\035"},
        {'\036', "\\036"}, {'\037', "\\037"}, {'\177', "\\177"},
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
    void AddToken(int line, TokenClass tc, const std::string &lexeme);

    void PrintTokens(char *name);
};

std::string StrToLower(std::string s);

std::string StrToUpper(std::string s);

bool IsBoolConst(const std::string &s);

bool IsSpaceSymbol(char lit);

void GetErrorSpace(Lexer &lex, char lit, int line);

void GetTokenAfEq(Lexer &lex, std::ifstream &file, char lit, int line);

void GetTokenAfLt(Lexer &lex, std::ifstream &file, char lit, int line);

bool GetComments(Lexer &lex, std::ifstream &file, char lit, int &line,
                 int &flag_cmnt);

void GetStrConst(Lexer &lex, std::ifstream &file, char lit, int &line,
                 std::string str = "\"", int spec_sym = 0);

void GetNumToken(Lexer &lex, std::ifstream &file, char lit, int line);

void GetStrToken(Lexer &lex, std::ifstream &file, char lit, int line);

void GetLexems(Lexer &lex, std::ifstream &file);
