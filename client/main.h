#ifndef _chatting_
#define _chatting_

#include <string>

void gotoxy(int x, int y);
int wherex();
int wherey();
void Clear();
void Print();
void PushPrintText(std::string string);
void main();

#endif