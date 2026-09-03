#pragma once

void create_interactive_shell(void);
void elevate_access(void);
void drop_access(void);
void run_command(const char *command, const char *mode);
void clean_binaries(void);