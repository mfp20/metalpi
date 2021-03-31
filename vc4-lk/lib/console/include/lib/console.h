/*
 * Copyright (c) 2008-2009 Travis Geiselbrecht
 *
 * Use of this source code is governed by a MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT
 */
#pragma once


/* Previously, this file was included to get access to defining a console
 * command. This logic has been moved into the following header, which is
 * what in almost every case what regular code wants to include instead of
 * this file.
 */
#include <lk/console_cmd.h>

/* external api */
int console_init(void);
void console_start(void);
void console_register_commands(cmd_block *block);
int console_run_script(const char *string);
int console_run_script_locked(const char *string); // special case from inside a command
console_cmd console_get_command_handler(const char *command);
void console_abort_script(void);

/* panic shell api */
void panic_shell_start(void);

extern int lastresult;

