#include <bits/stdc++.h>

using namespace std;

int main() {
    fstream in("Values", fstream::in);
    fstream out("Binary", fstream::out | fstream::binary);
    string str;
    while (getline(in, str)) {
        char c[3];
        c[2] = '\0';
        for (int i = 0; i < str.length(); ++i) {
            if ( (str[i] == 'x') && (str[i - 1] == '0') ) {
                c[0] = str[i+1]; c[1] = str[i+2];
                string cc = c;
                stringstream ss;
                ss << hex << cc;
                int num;
                ss >> num;
                cout << cc << ' ' << num << endl;
                unsigned char byte;
                byte = num;
                out.write((const char*)&byte, 1);
            }
        }
    }
}
