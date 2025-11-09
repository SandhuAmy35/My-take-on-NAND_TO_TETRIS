#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib> //find out what this is
#include <map>
#include <bitset> //what out all of its functionalities

using namespace std;
bool isposdecimal(string str)
{
    bool b; 
    if (str.empty()) return false;
    for (char c : str) {
        if (!isdigit(c)) {
            b = false;
            break;
        }
    }
    try{ 
        float a=stod(str);
        if(a>=0 && a<=32767)
        {
            b=1;
        }
        else
        {
            b=0;
        }
    }
    catch(const invalid_argument){
        b=0;
    }
    return b;
}

bool issymbol(string str)
{
    bool b;
    if(isalpha(str[1]))
    {
        b=1;
    }
    else
    {
        b=0;
    }
    return b;
}

class parser{
public:
    string fname;
    string line;
    int type;
    string c;
    string d;
    string j;
    string a;
    string l;
    ifstream i;
    bool des;
    bool jum;


    parser(string name){
        fname=name;
        i.open(fname);
        if(i.fail()){
            cerr<<"File doesn't exist"<<endl;
            exit(1);
        }
        
    }

    ~parser() {
        if (i.is_open()) {
            i.close();
        }
    }

    bool hasMoreLines() {
        i.open(fname);
        return i.eof();
    }

    bool advance() { //Understand and learn whatever shit is this
    string raw_line;
    while (getline(i, raw_line)) {
        size_t comment_pos = raw_line.find("//");
        if (comment_pos != string::npos) {
            raw_line = raw_line.substr(0, comment_pos);
        }

        size_t first_char = raw_line.find_first_not_of(" \t\n\r");
        size_t last_char = raw_line.find_last_not_of(" \t\n\r");

        if (string::npos == first_char) {
            continue; // Skip empty or whitespace-only lines
        }

        line = raw_line.substr(first_char, last_char - first_char + 1);
        return true; // A valid, clean line was found
    }
    line = ""; // No more lines left
    return false;
    }

    int commandtype(){
        if (line.empty()) {
        return -1; 
        }  
        if(line[0]=='@'){
            
            type=0;
        }
        else if(line=="")
        {
            type=-1;
        }
        else if(line[1]=='(')
        {
            type=2;
        }
        else
        {
            type=1;
        }
        return type;
    }

    string comp(){
        c="";
        des=0;
       // line=advance();
        for(int i=0;i<line.length();i++){
            if(line[i]=='='){
                des=1;
                break;
            }
            else if(line[i]=='\n')
            {
                des=0;
                break;
            }
        }
        for(int i=0;i<line.length();i++){
            if((line[i]=='=' || des==0)){
                for(int j=i;j<line.length();j++){
                    if(line[j]==';'||line[j]=='\n'){
                        break;
                    }
                    if(line[j]=='='){continue;}
                    c+=line[j];
                }
                break;
            }
        }
        return c;
    }

    string dest(){
        d="";
        if(des==1){
        for(int i=0;i<line.length();i++){
            if((line[i]=='=')){
                break;
            }
            if(des){
                d+=line[i];
            }
        }
        }
        else{
            d="";
        }
        return d;
    }

    string jump(){
        j="";
        for(int i=0;i<line.length();i++){
            if(line[i]==';'){
                for(int k=i;k<line.length();k++){
                    if(line[k]=='\n'){
                        break;
                    }
                    if(line[k]==';'){continue;}
                    j+=line[k];
                }
                break;
            }
        }
        return j;
    }

    string address(){
        a="";
        for(int i=1;i<line.length();i++)
        {
            a+=line[i];
        }
        if(isposdecimal(a) || issymbol(a))
        {
            return a;
        }
        else
        {
            cerr<<"Not a valid A-Instruction"<<endl; 
            return "";
        }
    }

    string label(){
        l="";
        int i=0;
        if(line[i]=='(')
        {
            while(line[i]!=')')
            {
                l=l+line[i];
                i++;
            }
        }
        return l;
    }
    void inscounter(){
        int count;
        if(commandtype()==1 || commandtype()==0)
        {
            count+=1;
        }
        else if(commandtype()==2)
        {
            table t(label(),to_string(count));
        }
    }
    void reset(){}
};

class code{
    public:
    map<string,string> cc;
    map<string,string> dd;
    map<string,string> jj;
    map<string,string> bis;
    string key;
    string value;
    
    code(){
        cc["0"]="0101010"; cc["1"]="0111111"; cc["-1"]="0111010"; cc["D"]="0001100";
        cc["A"]="0110000"; cc["M"]="1110000"; cc["!D"]="0001101"; cc["!A"]="0110001";
        cc["!M"]="1110001"; cc["-D"]="0001111"; cc["-A"]="0110011"; cc["-M"]="1110011";
        cc["D+1"]="0011111"; cc["A+1"]="0110111"; cc["M+1"]="10110111"; cc["D-1"]="0001110";
        cc["A-1"]="0110010"; cc["M-1"]="1110010"; cc["D+A"]="0000010"; cc["D+M"]="1000010";
        cc["D-A"]="0010011"; cc["D-M"]="1010011"; cc["A-D"]="0000111"; cc["M-D"]="1000111";
        cc["D&A"]="0000000"; cc["D&M"]="1000000"; cc["D|A"]="0010101"; cc["D|M"]="1010101";
        dd[""]="000"; dd["M"]="001"; dd["D"]="010"; dd["MD"]="011"; dd["A"]="100"; dd["AM"]="101"; dd["AD"]="110"; dd["AMD"]="111";
        jj[""]="000"; jj["JGT"]="001"; jj["JEQ"]="010"; jj["JGE"]="011"; jj["JLT"]="100"; jj["JNE"]="101"; jj["JLE"]="110"; jj["JMP"]="111";
        for(int i = 0;i<=15;i++)
        {
            string key="R"+to_string(i);
            bis[key]=to_string(i);
        }
        bis["SCREEN"]="16384"; bis["KBD"]="24576";
        bis["SP"]="0"; bis["LCL"]="1"; bis["ARG"]=2; bis["THIS"]="3"; bis["THAT"]="4";
    }

    string comp(string c){
        if(cc.find(c)==cc.end())
        {
            cout<<"Invalid Computation Statement"<<endl;
            return "";
            
        }
        else
        {
            return cc[c];
        }
    }

    string dest(string d){
        if(dd.find(d)==dd.end())
        {
            cout<<"Invalid Destination Statement"<<endl;
            return "";
        }
        else
        {
           return dd[d];
        }
    }

    string jump(string j){
        if(jj.find(j)==jj.end())
        {
            cout<<"Invalid Jump Statement"<<endl;
            return "";
        }
        else
        {
            
            return jj[j];
        }
    }

    string address(string a){
        int ad=stoi(a);
        bitset<15> aa(ad);
        return aa.to_string();
        if(bis.find(j)==jj.end())
        {
            cout<<"Invalid Jump Statement"<<endl;
            return "";
        }
        else
        {
            
            return jj[j];
        }
    }

    string label(){
        t.symboltable();
    }
};

class table{
    public:
    map<string ,string> bis;
    map<string,string> symtab;
    table(string key,string value){
        for(int i = 0;i<=15;i++)
        {
            string key="R"+to_string(i);
            bis[key]=to_string(i);
        }
        bis["SCREEN"]="16384"; bis["KBD"]="24576";
        bis["SP"]="0"; bis["LCL"]="1"; bis["ARG"]=2; bis["THIS"]="3"; bis["THAT"]="4";
        symboltable(key,value);
    }
    void symboltable(string key,string value){
        if(symtab.find(key)==symtab.end())
        {
            symtab[key]=value;
        }
    }
    string pair(string key){
        if(!(symtab.find(key)==symtab.end()))
        {
            return symtab[key];
        }
        else if (!(bis.find(key)==bis.end()))
        {
            return bis[key];
        }
    }
};

int main(int argc, char *argv[]){
    if(argc !=2){
        cerr << "Error: Invalid arguments." << std::endl;
        cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
        return 1;
    }
    string name;
    name=argv[1];
    parser p(name);
    code c; 
    ofstream o;
    string na;
    na = name.substr(0, name.find_last_of('.')) + ".hack";
    o.open(na); 
    string x,y,z,xx,yy,zz,final;
    int a,i=0;
    while (p.advance())
    {
    a=0;
    x="";
    y="";
    z="";
    xx="";
    yy="";
    zz="";
    final=""; 
    a=p.commandtype();
    if(a==1){
        x=p.comp();
        y=p.dest();
        z=p.jump();
        xx=c.comp(x);
        yy=c.dest(y);
        zz=c.jump(z);
        if(xx==""||yy==""||zz=="")
        {
        return -1;
        }
        final="111"+xx+yy+zz;
        i++;
    }
    else if(a==0){
        x=p.address();
             try {
                int ad = stoi(x);
                xx = c.address(x); 
                final = "0" + xx;
            } catch (const std::invalid_argument& e) {
                cerr << "Error: Invalid A-instruction address: " << x << endl;
                continue; 
            }
    }
    if(p.line=="")break;
    bitset<16> instruction_bits(final);
    o<<instruction_bits<<endl;
    o.close();
    return 0;
}
}