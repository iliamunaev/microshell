#include "unistd.h"
#include "stdlib.h"
#include "string.h"
#include "sys/wait.h"

// Writes an error message to STDERR
void ft_error(char *msg, char *cmd)
{
	while (*msg)
		write(2, msg++, 1);
	if (cmd)
		while (*cmd)
			write(2, cmd++, 1);
	write(2, "\n", 1);
}

// Built-in 'cd' command handler
void change_dir(char **av, int i)
{
	if (i != 2)
		ft_error("error: cd: bad arguments", NULL); // Must have exactly one argument
	else if (chdir(av[1]) == -1)
		ft_error("error: cd: cannot change directory to ", av[1]); // If fails
}

// Executes a command in the current process
void execute(char **av, int i, int tmp_fd, char **envp)
{
	av[i] = NULL;               // Null-terminate the command args
	dup2(tmp_fd, 0);
	close(tmp_fd);
	execve(av[0], av, envp);    // Execute the command
	ft_error("error: cannot execute ", av[0]);
	exit(1);
}

// Execute a command normally (not piped)
void exec_in_current(char **av, int i, int *tmp_fd, char **envp)
{
	if (fork() == 0) // In child process
		execute(av, i, *tmp_fd, envp);
	else // In parent process
	{
		close(*tmp_fd);
		while (waitpid(-1, NULL, 2) != -1); // Wait for all child processes
		*tmp_fd = dup(0);                   // Reset tmp_fd to stdin
	}
}

// Execute a command in a pipeline
void exec_in_pipe(char **av, int i, int *tmp_fd, char **envp, int fd[])
{
	pipe(fd); // Create a pipe

	if (fork() == 0) // In child process
	{
		dup2(fd[1], 1);
		close(fd[1]);
		close(fd[0]);
		execute(av, i, *tmp_fd, envp);
	}
	else // In parent process
	{
		close(fd[1]);
		close(*tmp_fd);
		*tmp_fd = fd[0];            // Set next input to read end of pipe
	}
}

// Entry point
int main(int ac, char **av, char **envp)
{
	(void)ac;
	av++; // Skip program name

	int fd[2];                   // Pipe file descriptors
	int tmp_fd = dup(0);         // Duplicate stdin for redirection

	while (*av)
	{
		int i = 0;

		// Find next delimiter ';' or '|'
		while (av[i] && strcmp(av[i], ";") != 0 && strcmp(av[i], "|") != 0)
			i++;

		// Handle 'cd' command
		if (i > 0 && strcmp(av[0], "cd") == 0)
			change_dir(av, i);

		// If next token is ';' or end of input, execute normally
		else if (i > 0 && (!av[i] || strcmp(av[i], ";") == 0))
			exec_in_current(av, i, &tmp_fd, envp);

		// If next token is '|', execute as part of a pipeline
		else if (i > 0 && strcmp(av[i], "|") == 0)
			exec_in_pipe(av, i, &tmp_fd, envp, fd);

		// Move pointer past the current command and delimiter
		if (av[i])
			av = &av[i + 1];
		else
			break;
	}

	close(tmp_fd);
	return 0;
}
