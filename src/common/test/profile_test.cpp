#include "../profile.h"

#include <iostream>
#include <string>
#include <vector>

using namespace std;

int main()
{
    CProfile profile;
    profile.InitCurrentDirectory(); // load colobot.ini file

    string result;
    profile.GetLocalProfileString("test_string", "string_value", result);
    if (result != "Hello world") {
        cout << "GetLocalProfileString failed!" << endl;
        return 1;
    }

    int int_value;
    profile.GetLocalProfileInt("test_int", "int_value", int_value);
    if (int_value != 42) {
        cout << "GetLocalProfileInt failed!" << endl;
        return 1;
    }

    float float_value;
    profile.GetLocalProfileFloat("test_float", "float_value", float_value);
    if (float_value != 1.5) {
        cout << "GetLocalProfileFloat failed!" << endl;
        return 1;
    }

    vector<string> list;
    list = profile.GetLocalProfileSection("test_multi", "entry");
    if (list.size() != 5) {
        cout << "GetLocalProfileSection failed!" << endl;
        return 1;
    }

    return 0;
}
