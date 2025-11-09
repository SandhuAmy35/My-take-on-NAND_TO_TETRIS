#include <iostream>
#include <fstream>
#include <string>
#include <unordered_set>
#include <unordered_map>

using namespace std;

unordered_set<string> ALC={"add","sub", "neg", "eq", "gt", "lt", "and", "or", "not"};

unordered_map<string,string> arithmetic={
    {"add","@SP\nAM=M-1\nD=M\nA=A-1\nM=M+D\n"},
    {"sub","@SP\nAM=M-1\nD=M\nA=A-1\nM=M-D\n"},
    {"and","@SP\nAM=M-1\nD=M\nA=A-1\nM=M&D\n"},
    {"or","@SP\nAM=M-1\nD=M\nA=A-1\nM=M|D\n"},
    {"not","@SP\nA=M-1\nM=!M\n"},
    {"neg","@SP\nA=M-1\nM=-M\n"},
};

unordered_map<string,string> segments={
    {"local","LCL"},
    {"argument","ARG"},
    {"this","THIS"},
    {"that","THAT"},
};

enum{
    C_ARITHMETIC,
    C_PUSH,C_POP,
    C_LABEL,C_GOTO,C_IF,
    C_FUNCTION,C_CALL,C_RETURN
    };

class parser{
    
    ifstream i;
    string line;

    public:

    parser(string& fname){
        i.open(fname);
        if(i.fail()){
            cerr<<"File doesnt exist "<<fname<<endl;
            exit(1);
        }
    }

    ~parser(){
        if(i.is_open()){
            i.close();
        }
    }

    bool Advance(){
        string raw_line;
        while(getline(i,raw_line)){
            //This Section below literally only considers the
            //string only before // so if no string before
            //thats an empty line
            size_t comment_pos=raw_line.find("//");
            if(comment_pos != string::npos){
                raw_line=raw_line.substr(0,comment_pos);
            }
            //This section helps clear all the white space 
            size_t first_char=raw_line.find_first_not_of(" \t\n\r");
            size_t last_char=raw_line.find_last_not_of(" \t\n\r");
            //Ignores empty lines
            if(string::npos==first_char){
                continue;
            }
            line=raw_line.substr(first_char,last_char-first_char+1);
            return true;
        }
        //returning false cause we at the end of the file
        line="";
        return false;
    }

    int command_type(){
        string type;
        for(char c:line){
            if(c ==' '){
                break;
            }
            type+=c;
        }
        if (type == "push") {
            return C_PUSH;
        } 
        else if (type == "pop") {
            return C_POP;
        }
        else if (type == "label") {
            return C_LABEL;
        }
        else if (type == "goto") {
            return C_GOTO;
        }
        else if (type == "if-goto") {
            return C_IF;
        }
        else if (type == "function") {
            return C_FUNCTION;
        }   
        else if (type == "return") {
            return C_RETURN;
        }
        else if (type == "call") {
            return C_CALL;
        }
        else if (ALC.count(type)) {
            return C_ARITHMETIC;
        }
        return -1;
    }

    string Arg1(int cmd){
        string arg1="";
        if(cmd==C_ARITHMETIC){
            for(char c:line)
            {
                if(c==' ') break;
                arg1+=c;
            }
        }
        else{
            size_t first_space=line.find(' ');
            string temp=line.substr(first_space+1);
            for(char c:temp)
            {
                if(c==' ') break;
                arg1+=c;
            }
        }
        return arg1;
    }

    int Arg2(int cmd){
        int arg2;
        if(cmd!=C_ARITHMETIC){
        size_t last_space=line.rfind(" ");
        string num=line.substr(last_space+1);
        arg2=stoi(num);
        return arg2;
        }
        else
        {
            return -1;
        }
    }

};

class CodeWrite{
    ofstream o;
    int jump_counter=0;

    public:

    CodeWrite(string ofname){
        o.open(ofname);
    }

    void WriteArithmetic(string command){
        if(arithmetic.count(command)){
            o<<arithmetic.at(command);
        }
        else if(command=="eq"){
            string id = to_string(jump_counter++);
            o<<"@SP\nM=M-1\nA=M\nD=M\n@SP\nM=M-1\nA=M\nD=M-D\n"
             <<"@EQ_if_true"<<id<<"\nD;JEQ\n@SP\nA=M\nM=0\n@END"<<id<<"\n0;JMP\n"
             <<"(EQ_if_true"<<id<<")\n@SP\nA=M\nM=-1\n@END"<<id<<"\n0;JMP\n"
             <<"(END"<<id<<")\n@SP\nM=M+1\n";
        }   
        else if(command=="gt"){
            string id = to_string(jump_counter++);
            o<<"@SP\nM=M-1\nA=M\nD=M\n@SP\nM=M-1\nA=M\nD=M-D\n"
             <<"@GT_if_true"<<id<<"\nD;JGT\n@SP\nA=M\nM=0\n@END"<<id<<"\n0;JMP\n"
             <<"(GT_if_true"<<id<<")\n@SP\nA=M\nM=-1\n@END"<<id<<"\n0;JMP\n"
             <<"(END"<<id<<")\n@SP\nM=M+1\n";
        }
        else if(command=="lt"){
            string id = to_string(jump_counter++);
            o<<"@SP\nM=M-1\nA=M\nD=M\n@SP\nM=M-1\nA=M\nD=M-D\n"
             <<"@LT_if_true"<<id<<"\nD;JLT\n@SP\nA=M\nM=0\n@END"<<id<<"\n0;JMP\n"
             <<"(LT_if_true"<<id<<")\n@SP\nA=M\nM=-1\n@END"<<id<<"\n0;JMP\n"
             <<"(END"<<id<<")\n@SP\nM=M+1\n";
        }
    }

    void WritePushPop(int command,string segment,int i,string xxx){
        string foo=xxx+"."+to_string(i);
        switch(command){
            case C_PUSH:
                if(segments.count(segment)){
                    string seg=segments.at(segment);
                    o<<"@"<<seg<<"\nD=M\n@"<<i
                     <<"\nD=D+A\nA=D\nD=M\n@SP\nA=M\nM=D\n"
                     <<"@SP\nM=M+1\n";
                }
                else if(segment=="constant"){
                    o<<"@"<<i<<"\nD=A\n@SP\nA=M\nM=D\n@SP\n"
                     <<"M=M+1\n";
                }
                else if(segment=="static"){
                    o<<"@"<<foo<<"\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n";
                }
                else if(segment=="temp"){
                    o<<"@5\nD=A\n@"<<i<<"\nA=A+D\nD=M\n@SP\nA=M\nM=D\n"
                     <<"@SP\nM=M+1\n";
                }
                else if(segment=="pointer"){
                    if(i == 0){ 
                        o<<"@THIS\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n";
                    }else{ 
                        o<<"@THAT\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n";
                    }
                }
                break;
            case C_POP:
                if(segments.count(segment)){
                    string seg=segments.at(segment);
                    o<<"@"<<seg<<"\nD=M\n@"<<i<<"\nD=D+A\n@R13\n"
                     <<"M=D\n@SP\nM=M-1\nA=M\nD=M\n@R13\nA=M\n"
                     <<"M=D\n";
                }
                else if(segment=="static"){
                    o<<"@SP\nM=M-1\nA=M\nD=M\n@"<<foo<<"\nM=D\n";
                }
                else if(segment=="temp"){
                    o<<"@5\nD=A\n@"<<i<<"\nA=A+D\nD=A\n@R13\n"
                     <<"M=D\n@SP\nM=M-1\nA=M\nD=M\n@R13\nA=M"
                     <<"\nM=D\n";
                }
                else if(segment=="pointer"){
                    if(i == 0){ 
                        o<<"@SP\nAM=M-1\nD=M\n@THIS\nM=D\n";
                    }else{
                        o<<"@SP\nAM=M-1\nD=M\n@THAT\nM=D\n";
                    }
                }
                break;
        }
    }

};

int main(int argc,char* argv[]){
    if(argc != 2){
        cerr<<"Invalid Number of Arguments";
        cout<<"Usage: "<<argv[0]<<"<filename>"<<endl;
    }
    string fname = argv[1];
    parser p(fname);
    size_t name = fname.find_first_of('.');
    size_t dot_pos = fname.find_last_of('.');
    string ofname;
    size_t foo = fname.find_last_of('/')+1;
    string jname="";
    if (dot_pos != string::npos) {
        jname = fname.substr(foo,foo-dot_pos+1);
        ofname = fname.substr(0, dot_pos) + ".asm";
    } else {
        jname = fname.substr(foo);
        ofname = fname + ".asm";
    }
    CodeWrite c(ofname);
    int command;
    while(p.Advance()){
        int command=-1;
        string arg1="";
        int arg2=-1;
        command=p.command_type();
        if(command==C_ARITHMETIC){
            arg1=p.Arg1(command);
            c.WriteArithmetic(arg1);
        }
        else
        {
            arg1=p.Arg1(command);
            arg2=p.Arg2(command);
            c.WritePushPop(command,arg1,arg2,jname);
        }

    }
    return 0;
}