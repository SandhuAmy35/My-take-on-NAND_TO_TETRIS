#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib> // Defines general utility functions like exit(1)
#include <map>
#include <bitset> // Used for converting decimal addresses to 15-bit binary

using namespace std;

// --- Helper Functions ---

bool isposdecimal(const string& str) {
    bool b = true;
    if (str.empty()) return false;
    for (char c : str) {
        if (!isdigit(c)) {
            b = false;
            break;
        }
    }
    if (b) {
        try {
            int a = stoi(str);
            // Hack machine addresses are 0 to 32767 (15 bits)
            if (a < 0 || a > 32767) {
                b = false;
            }
        } catch (const out_of_range&) {
            b = false;
        }
    }
    return b;
}

bool issymbol(const string& str) {
    if (str.empty() || (!isalpha(str[0]) && str[0] != '_' && str[0] != '.' && str[0] != '$')) {
        return false;
    }
    for (int i = 1; i < str.length(); ++i) {
        if (!isalnum(str[i]) && str[i] != '_' && str[i] != '.' && str[i] != '$') {
            return false;
        }
    }
    return true;
}

// Command Types
enum { A_COMMAND = 0, C_COMMAND = 1, L_COMMAND = 2, NO_COMMAND = -1 };

// --- Table Class (Symbol Table) ---

class table {
public:
    map<string, string> symtab;
    int next_ram_address = 16; // Start of general-purpose RAM variables

    table() {
        // Predefined symbols
        symtab["SP"] = "0"; symtab["LCL"] = "1"; symtab["ARG"] = "2"; symtab["THIS"] = "3"; symtab["THAT"] = "4";
        for (int i = 0; i <= 15; ++i) {
            symtab["R" + to_string(i)] = to_string(i);
        }
        symtab["SCREEN"] = "16384"; symtab["KBD"] = "24576";
    }

    // Add a label or symbol to the table
    void addEntry(const string& key, const string& value) {
        if (symtab.find(key) == symtab.end()) {
            symtab[key] = value;
        }
    }

    // Check if symbol is in the table
    bool contains(const string& key) const {
        return symtab.count(key) > 0;
    }

    // Get address for a symbol
    string getAddress(const string& key) const {
        auto it = symtab.find(key);
        if (it != symtab.end()) {
            return it->second;
        }
        return ""; // Symbol not found
    }

    // Get the next available RAM address for a variable
    string getNewVariableAddress(const string& symbol) {
        string address = to_string(next_ram_address);
        symtab[symbol] = address;
        next_ram_address++;
        return address;
    }
};

// --- Parser Class ---

class parser {
public:
    string line;
    ifstream i;

    parser(const string& fname) {
        i.open(fname);
        if (i.fail()) {
            cerr << "File doesn't exist: " << fname << endl;
            exit(1);
        }
    }

    ~parser() {
        if (i.is_open()) {
            i.close();
        }
    }

    void reset() {
        i.clear(); // Clear any error flags
        i.seekg(0, ios::beg); // Rewind to the beginning of the file
    }

    // Understand and learn whatever shit is this: This function reads a line,
    // removes comments (//...), trims whitespace, and skips empty/comment-only lines.
    // It returns true if a valid instruction/label line is found, false otherwise.
    bool advance() { 
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

    int commandType() const {
        if (line.empty()) {
            return NO_COMMAND;
        }
        if (line[0] == '@') {
            return A_COMMAND;
        }
        if (line[0] == '(') {
            return L_COMMAND; // Label pseudo-command
        }
        return C_COMMAND;
    }

    string symbol() const { // Used for A-command or L-command (address or label)
        if (commandType() == A_COMMAND) {
            string a = line.substr(1);
            if (isposdecimal(a) || issymbol(a)) {
                return a;
            } else {
                cerr << "Error: Invalid A-Instruction address/symbol: " << a << endl;
                return "";
            }
        }
        if (commandType() == L_COMMAND) {
            size_t end_paren = line.find(')');
            if (end_paren != string::npos && end_paren > 1) {
                return line.substr(1, end_paren - 1); // Extract label between ( and )
            }
            cerr << "Error: Invalid L-Command syntax: " << line << endl;
            return "";
        }
        return "";
    }

    // For C-Command: dest=comp;jump
    string dest() const {
        size_t eq_pos = line.find('=');
        if (eq_pos != string::npos) {
            return line.substr(0, eq_pos);
        }
        return ""; // No destination part
    }

    string comp() const {
        size_t eq_pos = line.find('=');
        size_t sc_pos = line.find(';');
        
        size_t start = (eq_pos == string::npos) ? 0 : eq_pos + 1;
        size_t end = (sc_pos == string::npos) ? line.length() : sc_pos;

        if (start >= end) return "";

        return line.substr(start, end - start);
    }

    string jump() const {
        size_t sc_pos = line.find(';');
        if (sc_pos != string::npos) {
            return line.substr(sc_pos + 1);
        }
        return ""; // No jump part
    }
};

// --- Code Class (Binary Translation) ---

class code {
public:
    map<string, string> cc; // Comp
    map<string, string> dd; // Dest
    map<string, string> jj; // Jump

    code() {
        // Comp mnemonics
        cc["0"] = "0101010"; cc["1"] = "0111111"; cc["-1"] = "0111010";
        cc["D"] = "0001100"; cc["A"] = "0110000"; cc["M"] = "1110000";
        cc["!D"] = "0001101"; cc["!A"] = "0110001"; cc["!M"] = "1110001";
        cc["-D"] = "0001111"; cc["-A"] = "0110011"; cc["-M"] = "1110011";
        cc["D+1"] = "0011111"; cc["A+1"] = "0110111"; cc["M+1"] = "1110111";
        cc["D-1"] = "0001110"; cc["A-1"] = "0110010"; cc["M-1"] = "1110010";
        cc["D+A"] = "0000010"; cc["D+M"] = "1000010";
        cc["D-A"] = "0010011"; cc["D-M"] = "1010011";
        cc["A-D"] = "0000111"; cc["M-D"] = "1000111";
        cc["D&A"] = "0000000"; cc["D&M"] = "1000000";
        cc["D|A"] = "0010101"; cc["D|M"] = "1010101";
        
        // Dest mnemonics
        dd[""] = "000"; dd["M"] = "001"; dd["D"] = "010"; dd["MD"] = "011";
        dd["A"] = "100"; dd["AM"] = "101"; dd["AD"] = "110"; dd["AMD"] = "111";

        // Jump mnemonics
        jj[""] = "000"; jj["JGT"] = "001"; jj["JEQ"] = "010"; jj["JGE"] = "011";
        jj["JLT"] = "100"; jj["JNE"] = "101"; jj["JLE"] = "110"; jj["JMP"] = "111";
    }

    string comp(const string& c) const {
        auto it = cc.find(c);
        if (it == cc.end()) {
            cerr << "Error: Invalid Computation Statement: " << c << endl;
            return "";
        }
        return it->second;
    }

    string dest(const string& d) const {
        auto it = dd.find(d);
        if (it == dd.end()) {
            cerr << "Error: Invalid Destination Statement: " << d << endl;
            return "";
        }
        return it->second;
    }

    string jump(const string& j) const {
        auto it = jj.find(j);
        if (it == jj.end()) {
            cerr << "Error: Invalid Jump Statement: " << j << endl;
            return "";
        }
        return it->second;
    }

    // Converts a decimal address string to a 15-bit binary string
    string address(const string& a) const {
        try {
            int ad = stoi(a);
            // bitset<15> creates a 15-bit binary representation.
            // to_string() converts it to "000...000" string.
            bitset<15> aa(ad);
            return aa.to_string();
        } catch (const exception& e) {
            cerr << "Error converting address to integer: " << a << " (" << e.what() << ")" << endl;
            return "";
        }
    }
};

// --- Main Program ---

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "Error: Invalid arguments." << endl;
        cerr << "Usage: " << argv[0] << " <filename>" << endl;
        return 1;
    }
    string input_fname = argv[1];
    parser p(input_fname);
    table symtab;
    code c;
    
    // Determine output file name (.hack)
    string output_fname;
    size_t dot_pos = input_fname.find_last_of('.');
    if (dot_pos != string::npos) {
        output_fname = input_fname.substr(0, dot_pos) + ".hack";
    } else {
        output_fname = input_fname + ".hack";
    }
    
    // --- Pass 1: Build Symbol Table (Handle L-Commands) ---
    int rom_address_counter = 0;
    while (p.advance()) {
        int command_type = p.commandType();
        if (command_type == A_COMMAND || command_type == C_COMMAND) {
            rom_address_counter++;
        } else if (command_type == L_COMMAND) {
            string label = p.symbol();
            if (!label.empty()) {
                // Add (LABEL) with the ROM address of the *next* instruction
                symtab.addEntry(label, to_string(rom_address_counter));
            }
        }
    }

    // Reset parser for Pass 2
    p.reset();

    // --- Pass 2: Generate Code (Handle A- and C-Commands) ---
    ofstream o(output_fname);
    if (!o.is_open()) {
        cerr << "Error: Could not open output file: " << output_fname << endl;
        return 1;
    }

    while (p.advance()) {
        string final_instruction = "";
        int command_type = p.commandType();

        if (command_type == C_COMMAND) {
            string comp_str = p.comp();
            string dest_str = p.dest();
            string jump_str = p.jump();

            string comp_bin = c.comp(comp_str);
            string dest_bin = c.dest(dest_str);
            string jump_bin = c.jump(jump_str);

            if (comp_bin.empty() || dest_bin.empty() || jump_bin.empty()) {
                cerr << "Error in C-Command translation: " << p.line << endl;
                continue; // Skip this line
            }
            // C-instruction format: 111 a ccc ccc ddd jjj
            final_instruction = "111" + comp_bin + dest_bin + jump_bin;

        } else if (command_type == A_COMMAND) {
            string symbol_or_address = p.symbol();
            string address_decimal_str = "";

            if (isposdecimal(symbol_or_address)) {
                // Case 1: Decimal literal, e.g., @100
                address_decimal_str = symbol_or_address;
            } else if (issymbol(symbol_or_address)) {
                // Case 2: Symbol (Label or Variable)
                if (symtab.contains(symbol_or_address)) {
                    // Symbol is a Label or Predefined
                    address_decimal_str = symtab.getAddress(symbol_or_address);
                } else {
                    // Symbol is a new Variable -> assign new RAM address (starting from R16)
                    address_decimal_str = symtab.getNewVariableAddress(symbol_or_address);
                }
            } else {
                continue; 
            }

            if (!address_decimal_str.empty()) {
                string address_bin = c.address(address_decimal_str);
                // A-instruction format: 0 followed by 15-bit address
                final_instruction = "0" + address_bin;
            }
        }
        
        // Write the binary instruction to the output file
        if (!final_instruction.empty() && final_instruction.length() == 16) {
            o << final_instruction << endl;
        }
    }

    o.close();
    cout << "Assembly complete. Output written to: " << output_fname << endl;
    return 0;
}