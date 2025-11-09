#include <iostream>
#include <fstream>
#include <string>

using namespace std;

int main(){
    
    ofstream output;

    output.open("C++ Shit\\scores.txt"); //or ofstream output("scores.txt")

    output<<"My name is armaan "<<"I Scored 90";

    output.close();

    cout<<"Done";

    return 0;
}