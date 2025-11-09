#include <iostream>
#include <fstream>
#include <string>

using namespace std;

int main(){

    ifstream input;

    input.open("scores.txt");

    if(input.fail()){
        cout<<"File does not exist";
    }
    
    string fn;
    string mn;
    string ln;
    int s;

    input>>fn>>mn>>ln>>s;

    cout<<fn<<" "<<mn<<" "<<ln<<" "<<s;

    input.close();

    return 0;
}