# Microshell

A minimal shell interpreter in C that supports basic command execution with `;`, pipelines `|`, and a built-in `cd` command.

## Description

`microshell` simulates a basic shell environment by executing command-line arguments passed to it. It supports:

- Command chaining with `;`
- Pipelining with `|`
- Built-in `cd` command with one path argument
- Error handling and file descriptor management

No environment variable expansion or wildcard/globbing features are implemented.

## Features

- Executes commands using absolute or relative paths
- Supports command separators (`;`)
- Supports pipes (`|`) with chained commands
- Built-in `cd` implementation:
  - Supports exactly one argument
  - Handles error cases (wrong arg count, chdir failure)

## Usage

Compile:

```bash
gcc -Wall -Wextra -Werror microshell.c -o microshell
```
Execute:
```bash
./microshell /bin/ls
```
