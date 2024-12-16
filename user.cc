#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdlib>

#include "fs.h"

using namespace std;

void show_help() {
    cout << "the command be supported:" << endl;
    cout << "  ls                              - list current file and directory" << endl;
    cout << "  touch <filename>                - create a file" << endl;
    cout << "  cat <filename>                  - show the file content" << endl;
    cout << "  read <filename> <filename>      - read the file content" << endl;
    cout << "  echo <filename> <text> <length> - write the content to file (cover)" << endl;
    cout << "  rm <filename>                   - delete a file" << endl;
    cout << "  open <filename>                 - open the file" << endl;
    cout << "  close <filename>                - close the file" << endl;
    cout << "  mkdir <dirname>                 - create a directory" << endl;
    cout << "  rmdir <dirname>                 - delete a directory" << endl;
    cout << "  exit                            - exit the program" << endl;
}

// command parser
void execute_command(const string& input) {
    istringstream iss(input);
    string command;
    iss >> command;

    if (command == "help") {
        show_help();
    } else if (command == "ls") {

        string dirPath;
        iss >> dirPath;
        if (dirPath.empty()) {
            dir("/");
        } else {
            dir(dirPath);
        }
    } else if (command == "touch") {
        string filename;
        iss >> filename;
        int attribute;
        iss >> attribute;
        if (!filename.empty()) {
            create_file(filename, attribute);
        } else {
            cerr << "Use: touch <filename> <attribute>" << endl;
        }
    } else if (command == "cat") {
        string filename;
        iss >> filename;
        if (!filename.empty()) {
            typefile(filename);
        } else {
            cerr << "Use: cat <filename>" << endl;
        }
    } else if (command == "read"){
        string filename;
        iss >> filename;
        int length = 0;
        iss >> length;
        if (!filename.empty() && length != 0) {
            read_file(filename, length);
        } else {
            cerr << "Use: read <filename> <length>" << endl;
        }
    }else if (command == "echo") {
        string filename, text;
        int length;

        iss >> filename;

        std::getline(iss, text);

        if (!text.empty() && text[0] == ' ') {
            text = text.substr(1);
        }

        if (!filename.empty() && !text.empty()) {
            size_t pos = text.find_last_of(' ');
            if (pos != string::npos) {
                length = stoi(text.substr(pos + 1));
                text = text.substr(0, pos);
            } else {
                cerr << "Use: echo <filename> <text> <length>" << endl;
                return;
            }
            write_file(filename, text, length);
        } else {
            cerr << "Use: echo <filename> <text> <length>" << endl;
        }
    } else if (command == "rm") {
        string filename;
        iss >> filename;
        if (!filename.empty()) {
            delete_file(filename);
        } else {
            cerr << "Use: rm <filename>" << endl;
        }
    } else if (command == "open") {
        string filename;
        iss >> filename;
        int flag;
        iss >> flag;
        if (!filename.empty()) {
            open_file(filename, flag);
        } else {
            cerr << "Use: open <filename> <flag>" << endl;
        }
    } else if (command == "close") {
        string filename;
        iss >> filename;
        if (!filename.empty()) {
            close_file(filename);
        } else {
            cerr << "Use: close <filename>" << endl;
        }
    } else if (command == "mkdir") {
        string dirname;
        iss >> dirname;
        if (!dirname.empty()) {
            md(dirname);
        } else {
            cerr << "Use: mkdir <dirname>" << endl;
        }
    } else if (command == "rmdir") {
        string dirname;
        iss >> dirname;
        if (!dirname.empty()) {
            rd(dirname);
        } else {
            cerr << "Use: rmdir <dirname>" << endl;
        }
    } else if (command == "exit") {
        cout << "exit the file system" << endl;
        fs_umount("disk.txt");
        exit(0);
    } else {
        cerr << "unknown command: " << command << endl;
        show_help();
    }
}

void user_info()
{
    cout << "Welcome using File Manage System! input 'help' to check the command." << endl;

    string input;
    while (true) {
        cout << "user@filesystem:~$ ";
        getline(cin, input);
        if (!input.empty()) {
            execute_command(input);
        }
    }
}
