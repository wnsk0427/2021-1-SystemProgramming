#define MAX_CMD_ARG 64

#define SHNAME "simplesh"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Call this to exit from a fatal error
static void fatal(const char *str)
{
	perror(str);
	exit(1);
}

/*
 * makelist() will split the input string to list array
 * and return the number of items in the list array.
 */
static int makelist(char *input, const char *delimiters, char **list, int max)
{
	int tokens = 0;

	// XXX
	// Split input to list with strtok()
	// tokens++;

	if (strlen(input) < 1) {
		return tokens;
	}

	list[tokens] = strtok(input, delimiters);
	tokens++;

	while (list[tokens] = strtok(NULL, delimiters)){
		tokens++;
	}

	return tokens;
}

/*
 * Check /proc/self/fd for file-descriptor leaks.
 * If this function prints something, it means some file-descriptors are left opened.
 * This function runs on exit().
 */
static void check_fd(void)
{
	struct dirent *p;
	pid_t pid;
	char pidpath[32], path[PATH_MAX], buf[PATH_MAX];
	DIR *d;

	pid = getpid();
	sprintf(pidpath, "/proc/%d/fd", pid);
	d = opendir(pidpath);

	if (!d)
		return;

	while ((p = readdir(d))) {
		// Skip the names "." and ".." as we don't want to recurse on them
		if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, ".."))
			continue;

		// Get real path
		sprintf(path, "%s/%s", pidpath, p->d_name);

		// Skip stdin, stdout, stderr
		if (strcmp(p->d_name, "0") == 0)
			continue;
		if (strcmp(p->d_name, "1") == 0)
			continue;
		if (strcmp(p->d_name, "2") == 0)
			continue;

		// All /proc/*/fd files are supposed to be symlinks
		if (p->d_type != DT_LNK)
			continue;

		// Initialize buf as readlink doesn't set EOF
		memset(buf, 0, PATH_MAX);
		readlink(path, buf, PATH_MAX);
		// Skip printing when this fd is check_fd() itself
		if (strcmp(buf, pidpath))
			printf("fd %s: %s\n", p->d_name, buf);
	}

	closedir(d);
}

int main(int argc, char **argv)
{
	char input[BUFSIZ]; // Input from the user
	char *input_arr[MAX_CMD_ARG]; // input split to string array
	char cwd[PATH_MAX]; // Current working directory

	int i, tokens;
	int flags; // open() flags
	bool eof;

	// Ignore Ctrl+Z (stop process)
	signal(SIGTSTP, SIG_IGN);
	// Allow forked processes to write to the terminal
	signal(SIGTTOU, SIG_IGN);
	signal(SIGTTIN, SIG_IGN);
	// Re-ape child processes automatically (zombie processes)
	signal(SIGCHLD, SIG_IGN);

	// Check for file-descriptor leaks on exit()
	atexit(check_fd);

	while (1) {
		// XXX
		// Set cwd here
		printf(SHNAME ":%s$ ", getcwd(cwd, PATH_MAX));

		// Initialize input by inserting NULL(EOF) to input[0]
		input[0] = '\0';
		while (!(eof = feof(stdin)) && !fgets(input, BUFSIZ, stdin));
		if (eof) {
			// Ctrl+D EOF
			printf("\n");
			exit(0);
		}
		input[strlen(input) - 1] = '\0'; // Replace '\n' at the end with NULL(EOF)

		// Parse input
		tokens = makelist(input, " \t", input_arr, MAX_CMD_ARG);
		
		if (tokens == 0) {
			continue;
		}

		int fd;

		// XXX
		// Implement built-in cd
		if (strstr("cd", input_arr[0])) {
			if (tokens > 2) {
				printf(SHNAME ": %s: too many arguments\n", input_arr[0]);
			}
			else if (tokens < 2) {
				printf(SHNAME ": %s: no arguments\n", input_arr[0]);
			}
			else {
				if (chdir(input_arr[1])) {
					if (access(input_arr[1], X_OK)) {
						if (access(input_arr[1], F_OK)) {
							printf(SHNAME ": %s: %s: No such file or directory\n", input_arr[0], input_arr[1]);
						}
						else {
							printf(SHNAME ": %s: %s: Permission denied\n", input_arr[0], input_arr[1]);
						}
					}
				}
			}
		}

		// XXX
		// Implement built-in exit
		else if (strstr("exit", input_arr[0])) {
			if (tokens > 2) {
				printf(SHNAME ": %s: too many arguments\n", input_arr[0]);
			}
			else {
				if (tokens > 1) {
					exit(atoi(input_arr[1]));
				}
				else {
					exit(0);
				}
			}
		}

		// XXX
		// Implement built-in pwd
		else if (strstr("pwd", input_arr[0])) {
			if (tokens > 1) {
				printf(SHNAME ": %s: too many arguments\n", input_arr[0]);
			}
			else {
				printf("%s\n", getcwd(cwd, PATH_MAX));
			}
		}

		/*
		 * Implement command execution.
		 * Implement redirection.
		 */
		else {
			int redi = 0;
			char *exec_cmd[MAX_CMD_ARG];
			int pid = fork();

			for (int n = 0; n < tokens; n++) {
				if (strstr(input_arr[n], ">") != 0) {
					redi = 1;

					if (strstr(input_arr[n], ">>") != 0) {
						redi = 2;
					}

					break;
				}

				exec_cmd[n] = input_arr[n];
			}

			if (redi == 2) {
				fd = open(input_arr[tokens - 1], O_WRONLY | O_CREAT | O_APPEND, 777);

				if ((fd < 0)) {
					perror("Failed to open file for stdout redirection");
					continue;
				}

				dup2(fd, STDOUT_FILENO);
				close(fd);
			}
			else if (redi == 1){
				fd = open(input_arr[tokens - 1], O_WRONLY | O_CREAT | O_TRUNC, 777);

				if ((fd < 0)) {
					perror("Failed to open file for stdout redirection");
					continue;
				}

				dup2(fd, STDOUT_FILENO);
				close(fd);
			}

			if (pid == 0) {
				execvp(exec_cmd[0], exec_cmd);
				perror("Failed to exec");
			}
			else {
				wait(NULL);
			}

			// redirection
		}

		// XXX
	}

	return 0;
}
