#ifndef _USER_H
#define _USER_H

#include <string>

using namespace std;

/**
 * show_help - show the helo infomation
*/
void show_help();

/**
 * execute_command - execute the command that user input
 * @input: the string user input
 * 
 * execute the command
*/
void execute_command(const string& input);

/**
 * user_info - main function
*/
void user_info();

#endif