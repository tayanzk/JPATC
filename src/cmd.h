/*
** JPAT C
*/

#ifndef _cmd_h
#define _cmd_h 1

typedef void(*register_cmd_fn)(int, char **);

void register_cmd(const char *cmd, register_cmd_fn fn);
void call_cmd(int c, char **v);

#endif