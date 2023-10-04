

#include "parserInt.h"
#include "lex.h"
using namespace std;

map<string, bool> defVar;
map<string, Token> SymTable;
map<string, Value> TempsResults; //Container of temporary locations of Value objects for results of expressions, variables values and constants
queue <Value>* ValQue; //declare a pointer variable to a queue of Value objects

namespace Parser {
    bool pushed_back = false;
    LexItem    pushed_token;

    static LexItem GetNextToken(istream& in, int& line) {
        if (pushed_back) {
            pushed_back = false;
            return pushed_token;
        }
        return getNextToken(in, line);
    }

    static void PushBackToken(LexItem& t) {
        if (pushed_back) {
            abort();
        }
        pushed_back = true;
        pushed_token = t;
    }

}

static int error_count = 0;

int ErrCount()
{
    return error_count;
}

void ParseError(int line, string msg)
{
    ++error_count;
    cout << error_count << ". Line # " << line << ": " << msg << endl;
}

bool Prog(istream& in, int& line)
{
    bool status = StmtList(in, line);
    if (!status)
    {
        ParseError(line, "Missing Program");
        return false;
    }
    cout << endl;
    cout << "(DONE)" << std::endl;
    return true;
}


bool StmtList(istream& in, int& line)
{
    bool status = Stmt(in, line);
    if (!status){
        ParseError(line, "Missing Statement");
        return false;
    }

    LexItem tok = Parser::GetNextToken(in, line);
    if (tok.GetToken() != SEMICOL){
        ParseError(line, "Missing SemiColon");
        return false;
    }

    tok = Parser::GetNextToken(in, line);
    if (tok.GetToken() == RBRACES){
        Parser::PushBackToken(tok);
        return true;
    }
    if (tok.GetToken() != DONE){
        Parser::PushBackToken(tok);
        return StmtList(in, line);
    }

    return true;
}




bool Stmt(istream& in, int& line){
    LexItem tok = Parser::GetNextToken(in, line);
    bool status = false;

    if (tok.GetToken() == WRITELN){
        status = WritelnStmt(in, line);
        if (!status){
            ParseError(line, "WRITELN ERR");
            return false;
        }
    }
    else if (tok.GetToken() == IF){
        status = IfStmt(in, line);
        if (!status){
            ParseError(line, "IFSTMT ERR");
            return false;
        }
    }
    else{
        Parser::PushBackToken(tok);
        status = AssignStmt(in, line);
        if (!status)
        {
            ParseError(line, "ASSIGNSTMT ERR");
            return false;
        }
    }
    return true;
}


//WritelnStmt:= WRITELN (ExpreList)
bool WritelnStmt(istream& in, int& line) {
    LexItem tok;
    ValQue = new queue<Value>;

    tok = Parser::GetNextToken(in, line);
    if (tok != LPAREN) {

        ParseError(line, "Missing Left Parenthesis of Writeln Statement");
        return false;
    }

    bool status = ExprList(in, line);

    if (!status) {
        ParseError(line, "Missing expression list after Print");
        while (!(*ValQue).empty())
        {
            ValQue->pop();
        }
        delete ValQue;
        return false;
    }

    //Evaluate: writeln by printing out the list of expressions' values
    while (!(*ValQue).empty())
    {
        Value nextVal = (*ValQue).front();
        cout << nextVal;
        ValQue->pop();
    }
    cout << endl;

    tok = Parser::GetNextToken(in, line);
    if (tok != RPAREN) {

        ParseError(line, "Missing Right Parenthesis of Writeln Statement");
        return false;
    }
    return true;
}//End of WritelnStmt

bool IfStmt(istream& in, int& line)
{
    LexItem tok;
    Value leftVal;

    tok = Parser::GetNextToken(in, line);
    if (tok.GetToken() != LPAREN)
    {
        ParseError(line, "Missing Left Parenthesis");
        return false;
    }
    bool status = Expr(in, line, leftVal);
    if (!status)
    {
        ParseError(line, "Missing expression");
        return false;
    }
    tok = Parser::GetNextToken(in, line);
    if (tok.GetToken() != RPAREN)
    {
        ParseError(line, "Missing Right Parenthesis");
        return false;
    }
    if (!leftVal.IsBool())
    {
        ParseError(line, "Not a boolean");
        return false;
    }
    tok = Parser::GetNextToken(in, line);
    if (tok.GetToken() != LBRACES)
    {
        ParseError(line, "Missing Left Brace");
        return false;
    }
    if (leftVal.GetBool())
    {
        status = StmtList(in, line);
        if (!status)
        {
            ParseError(line, "Missing statementlist");
            return false;
        }
        tok = Parser::GetNextToken(in, line);
        if (tok.GetToken() != RBRACES)
        {
            ParseError(line, "Missing Right Brace");
            return false;
        }
        tok = Parser::GetNextToken(in, line);
        if (tok.GetToken() == ELSE)
        {
            tok = Parser::GetNextToken(in, line);
            if (tok.GetToken() != LBRACES)
            {
                ParseError(line, "Missing Left brace");
                return false;
            }

            /*bool elseSLStatus = StmtList(in, line);
            if (!elseSLStatus)
            {
                ParseError(line, "Missing statementlist");
                return false;
            }*/

            tok = Parser::GetNextToken(in, line);
            while (tok.GetToken() != RBRACES)
            {
                if (tok.GetToken() == DONE)
                {
                    ParseError(line, "If statement Syntex Error: Missing right brace");
                    return false;
                }
                tok = Parser::GetNextToken(in, line);
            }
        }
        else
        {
            Parser::PushBackToken(tok);
        }
    }
    else
    {
        while (tok.GetToken() != RBRACES)
        {
            if (tok.GetToken() == DONE || tok.GetToken() == ELSE)
            {
                ParseError(line, "Syntax Error");
                return false;
            }
            tok = Parser::GetNextToken(in, line);
        }
        tok = Parser::GetNextToken(in, line);
        if (tok.GetToken() == ELSE)
        {
            tok = Parser::GetNextToken(in, line);
            if (tok.GetToken() != LBRACES)
            {
                ParseError(line, "If Statement Syntax Error: Missing left brace");
                return false;
            }
            status = StmtList(in, line);
            if (!status)
            {
                ParseError(line, "Missing Statement for Else-Clause");
                return false;
            }
            tok = Parser::GetNextToken(in, line);
            if (tok.GetToken() != RBRACES)
            {
                ParseError(line, "If Statement Syntax Error: Missing right brace. ");
                return false;
            }
        }
        else
        {
            Parser::PushBackToken(tok);
        }
    }
    return true;
}

bool AssignStmt(istream& in, int& line)
{
    string str = "";
    Value leftVal;

    LexItem identToken = Parser::GetNextToken(in, line);
    bool status = Var(in, line,identToken);
    
    if (!status)
    {
        ParseError(line, "Missing variable");
        return false;
    }
    
    str = identToken.GetLexeme();
    LexItem tok = Parser::GetNextToken(in, line);
    if (tok.GetToken() != ASSOP)
    {
        defVar[str] = false;
        Parser::PushBackToken(tok);
        return false;
    }
    status = Expr(in, line, leftVal);
    if (!status)
    {
        ParseError(line, "Missing Expression");
        return false;
    }
    switch (identToken.GetToken())
    {
    case NIDENT:
    {
        if (!leftVal.IsInt() && !leftVal.IsReal())
        {
            ParseError(line, "error");
            return false;
        }
        break;
    }
    case SIDENT:
    {
        if (!leftVal.IsInt() && !leftVal.IsReal() && !leftVal.IsString())
        {
            ParseError(line, "error");
            return false;
        }
        break;
    }
    default:
        break;
    }
    TempsResults[str] = leftVal;
    defVar[str] = true;
    return true;
}

bool Var(istream& in, int& line, LexItem& idtok)
{
    //idtok = Parser::GetNextToken(in, line);
    if (idtok.GetToken() == NIDENT || idtok.GetToken() == SIDENT)
    {
        return true;
    }
    else
    {
        return false;
    }
}
//ExprList:= Expr {,Expr}
bool ExprList(istream& in, int& line) {
    bool status = false;
    Value retVal;

    status = Expr(in, line, retVal);
    if (!status) {
        ParseError(line, "Missing Expression");
        return false;
    }
    ValQue->push(retVal);
    LexItem tok = Parser::GetNextToken(in, line);

    if (tok == COMMA) {
        status = ExprList(in, line);
    }
    else if (tok.GetToken() == ERR) {
        ParseError(line, "Unrecognized Input Pattern");
        cout << "(" << tok.GetLexeme() << ")" << endl;
        return false;
    }
    else {
        Parser::PushBackToken(tok);
        return true;
    }
    return status;
}//End of ExprList


bool Expr(istream& in, int& line, Value& retVal)
{
    bool status = RelExpr(in, line, retVal);
    
    if (!status)
    {
        return false;
    }
    
    LexItem tok = Parser::GetNextToken(in, line);

    if (tok.GetToken() == SEQ || tok.GetToken() == NEQ)
    {
        Value leftVal = retVal;
        status = RelExpr(in, line, retVal);
        if (!status)
        {
            ParseError(line, "Missing RelExpr");
            return false;
        }
        if (tok.GetToken() == SEQ)
        {
            retVal = leftVal.SEqual(retVal);
        }
        else if (tok.GetToken() == NEQ)
        {
            retVal = leftVal.operator==(retVal);
        }
        if (retVal.GetType() == VERR)
        {
            ParseError(line, "Illegal operand type for the operator.");
            return false;
        }
    }
    else
    {
        Parser::PushBackToken(tok);
    }
    return status;
}

bool RelExpr(istream& in, int& line, Value& retVal)
{
    bool status = AddExpr(in, line, retVal);
    if (!status)
    {
        return false;
    }

    LexItem tok = Parser::GetNextToken(in, line);
    if (tok.GetToken() == SLTHAN || tok.GetToken() == SGTHAN || tok.GetToken() == NLTHAN || tok.GetToken() == NGTHAN)
    {
        Value leftVal = retVal;

        status = AddExpr(in, line, retVal);
        if (!status)
        {
            ParseError(line, "Missing RelExpr");
            return false;
        }
        switch (tok.GetToken())
        {
        case SLTHAN:
        {
            retVal = leftVal.SLthan(retVal);
            break;
        }
        case SGTHAN:
        {
            retVal = leftVal.SGthan(retVal);
            break;
        }
        case NLTHAN:
        {
            retVal = leftVal.operator<(retVal);
            break;
        }
        case NGTHAN:
        {
            retVal = leftVal.operator>(retVal);
            break;
        }
        default:
            break;
        }
        if (retVal.GetType() == VERR)
        {
            ParseError(line, "Illegal operand type for the operation");
            return false;
        }
    }
    else
    {
        Parser::PushBackToken(tok);
    }
    return status;
}

bool AddExpr(istream& in, int& line, Value& retVal)
{
    bool status = MultExpr(in, line, retVal);
    if (!status)
    {
        return false;
    }

    LexItem tok = Parser::GetNextToken(in, line);
    while (tok.GetToken() == PLUS || tok.GetToken() == MINUS || tok.GetToken() == CAT)
    {
        Value leftVal = retVal;

        status = MultExpr(in, line,retVal);
        if (!status)
        {
            ParseError(line, "Missing MultExpr");
            return false;
        }

        switch (tok.GetToken())
        {
        case PLUS:
        {
            retVal = leftVal.operator+(retVal);
            break;
        }
        case MINUS:
        {
            retVal = leftVal.operator-(retVal);
            break;
        }
        case CAT:
        {
            retVal = leftVal.Catenate(retVal);
            break;
        }
        default:
            if (retVal.GetType() == VERR)
            {
                ParseError(line, "Illegal operand type for the operation.");
                return false;
            }
            break;
        }

        tok = Parser::GetNextToken(in, line);
    }
    Parser::PushBackToken(tok);
    return status;
}

bool MultExpr(istream& in, int& line, Value& retVal)
{
    bool status = ExponExpr(in, line, retVal);
    if (!status)
    {
        return false;
    }

    LexItem tok = Parser::GetNextToken(in, line);
    while (tok.GetToken() == MULT || tok.GetToken() == DIV || tok.GetToken() == SREPEAT)
    {
        Value leftVal = retVal;

        status = ExponExpr(in, line, retVal);
        if (!status)
        {
            ParseError(line, "Missing ExponExpr");
            return false;
        }

        switch (tok.GetToken())
        {
        case MULT:
        {
            retVal = leftVal.operator*(retVal);
            break;
        }
        case DIV:
        {
            retVal = leftVal.operator/(retVal);
            break;
        }
        case SREPEAT:
        {
            retVal = leftVal.Repeat(retVal);
            break;
        }
        default:
            if (retVal.GetType() == VERR)
            {
                ParseError(line, "Illegal operand type for the operation.");
                return false;
            }
            break;
        }
        tok = Parser::GetNextToken(in, line);
    }
    Parser::PushBackToken(tok);
    return status;
}

bool ExponExpr(istream& in, int& line, Value& retVal)
{
    bool status = UnaryExpr(in, line, retVal);
    if (!status)
    {
        return false;
    }

    LexItem tok = Parser::GetNextToken(in, line);
    while (tok.GetToken() == EXPONENT)
    {
        Value leftVal = retVal;
        status = ExponExpr(in, line, retVal);
        if (!status)
        {
            ParseError(line, "Missing UnaryExpr");
            return false;
        }
        if (tok.GetToken() == EXPONENT)
        {
            retVal = leftVal.operator^(retVal);
        }
        if (retVal.GetType() == VERR)
        {
            ParseError(line, "Illegal operand type for the operation.");
            return false;
        }
        tok = Parser::GetNextToken(in, line);
    }

    Parser::PushBackToken(tok);
    return status;
}

bool UnaryExpr(istream& in, int& line, Value& retVal)
{
    int sign = 0;
    LexItem tok = Parser::GetNextToken(in, line);

    if (tok.GetToken() == PLUS)
    {
        sign = 1;
    }
    else if (tok.GetToken() == MINUS)
    {
        sign = -1;
    }
    else
    {
        Parser::PushBackToken(tok);
    }
    bool status = PrimaryExpr(in, line, sign, retVal);

    return status;
}

bool PrimaryExpr(istream& in, int& line, int sign, Value& retVal)
{
    LexItem tok = Parser::GetNextToken(in, line);
    string str = tok.GetLexeme();

    if (tok.GetToken() == IDENT || tok.GetToken() == RCONST || tok.GetToken() == ICONST || tok.GetToken() == SCONST)
    {
        switch (tok.GetToken())
        {
        case RCONST: case ICONST:
        {
            if (str[str.length() - 1] == '.')
            {
                int val = stoi(str);
                retVal = Value(val);
            }
            else
            {
                double val = stod(str);
                retVal = Value(val);
            }
            break;
        }
        case SCONST:
        {
            if (sign != 0)
            {
                ParseError(line, "SIGN ERROR");
                return false;
            }
            retVal = Value(str);
            break;
        }
        default:
            break;
        }
        return true;
    }
    else if (tok.GetToken() == NIDENT || tok.GetToken() == SIDENT)
    {
        bool status = defVar[tok.GetLexeme()];
        if (!status)
        {
            ParseError(line, "Incorrect Var");
            return false;
        }
        if (sign == -1)
        {
            retVal = TempsResults[str] * -1;
        }
        else
        {
            retVal = TempsResults[str];
        }
        return true;
    }
    else if (tok.GetToken() == LPAREN)
    {
        bool exprstatus = Expr(in, line, retVal);
        if (!exprstatus)
        {
            ParseError(line, "Missing Expr");
            return false;
        }
        tok = Parser::GetNextToken(in, line);
        if (tok.GetToken() != RPAREN)
        {
            ParseError(line, " Missing RPAREN");
            return false;
        }
        else
        {
            return exprstatus;
        }
    }
    return false;
}

