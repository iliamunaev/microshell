#include "unistd.h"
#include "stdlib.h"
#include "string.h"
#include "sys/wait.h"

// Print error message to STDERR (with optional argument string)
void ft_puterror(char *msg, char *cmd)
{
	while(*msg)
		write(2, msg++, 1);
	if (cmd)
	{
		while(*cmd)
			write(2, cmd++, 1);
	}
	write(2, "\n", 1);
}

// Execute a command by replacing the current process with execve
// av[i] is set to NULL to mark the end of the arguments
void ft_execute(char **av, int i, int tmp_fd, char **envp)
{
	av[i] = NULL;
	dup2(tmp_fd, 0);         // Redirect stdin to tmp_fd
	close(tmp_fd);           // Close original tmp_fd
	execve(av[0], av, envp); // Try to execute the command
	ft_puterror("error: cannot execute ", av[0]); // If execve fails
	exit(1);
}

// Built-in "cd" command handler
// Only works with exactly one argument (e.g. cd dir)
void change_dir(char **av, int i)
{
	if (i != 2)
		ft_puterror("error: cd: bad arguments", NULL);
	else if (chdir(av[1]) != 0)
		ft_puterror("error: cd: cannot change directory to ", av[1]);
}

// Execute a standalone command (terminated by ";" or end of input)
void exec_in_current(char **av, int i, int *tmp_fd, char **envp)
{
	if (fork() == 0)
		ft_execute(av, i, *tmp_fd, envp); // Child: execute
	else
	{
		close(*tmp_fd);                       // Parent: close previous input fd
		while(waitpid(-1, NULL, 2) != -1);    // Wait for child to finish
		*tmp_fd = dup(0);                     // Restore stdin for next command
	}
}

// Execute a piped command and set up pipe redirection
void exec_in_pipe(char **av, int i, int *tmp_fd, char **envp, int fd[])
{
	pipe(fd); // Create a pipe

	if (fork() == 0)
	{
		dup2(fd[1], 1);       // Redirect stdout to pipe write end
		close(fd[0]);         // Close read end of pipe
		close(fd[1]);         // Close write end (already duplicated)
		ft_execute(av, i, *tmp_fd, envp); // Execute command with stdin from tmp_fd
	}
	else
	{
		close(fd[1]);         // Parent: close write end
		close(*tmp_fd);       // Close old input
		*tmp_fd = fd[0];      // Set new input from pipe read end
	}
}

// Entry point
int main (int ac, char **av, char **envp)
{
	(void)ac;
	av++; // Skip program name

	int fd[2];               // Pipe file descriptors
	int tmp_fd = dup(0);     // Duplicate original stdin

	while (*av)
	{
		int i = 0;

		// Find the length of the current command
		while (av[i] && strcmp(av[i], ";") != 0 && strcmp(av[i], "|") != 0)
			i++;

		// Handle built-in cd
		if (i > 0 && strcmp(av[0], "cd") == 0)
			change_dir(av, i);

		// Execute a command ending with ";" or end of input
		else if (i > 0 && (!av[i] || strcmp(av[i], ";") == 0))
			exec_in_current(av, i, &tmp_fd, envp);

		// Execute a command followed by a pipe
		else if (i > 0 && av[i] && strcmp(av[i], "|") == 0)
			exec_in_pipe(av, i, &tmp_fd, envp, fd);

		// Move to the next command (skip separator if present)
		if (av[i])
			av = &av[i + 1];
		else
			break;
	}

	close(tmp_fd); // Clean up
	return 0;
}
