/**
 * shell
 * CS 341 - Fall 2023
 */
#include "format.h"
#include "shell.h"
#include "vector.h"
#include "sstring.h"
#include "callbacks.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <dirent.h>
#include <time.h>

typedef struct process {
    char *command;
    pid_t pid;
} process;

static vector* process_list;

process* process_create(const char* command, pid_t pid) {
	process* p = calloc(1, sizeof(process));
	p->command = calloc(strlen(command)+1, sizeof(char));
	strcpy(p->command, command);
	p->pid = pid;
	return p;
}

void process_destroy(pid_t pid) {
	for(size_t i = 0; i < vector_size(process_list); i++) {
		process* p = (process*) vector_get(process_list, i);
		if(p->pid == pid) {
			free(p->command);
			free(p);
			vector_erase(process_list, i);
			return;
		}
	}
}

void process_list_destroy() {
	for(size_t i = 0; i < vector_size(process_list); i++) {
		process* p = (process*) vector_get(process_list, i);
                free(p->command);
                free(p);
        }
	vector_destroy(process_list);

}

void kill_stuff() {
	for(size_t i = 0; i < vector_size(process_list); i++) {
                process* p = (process*) vector_get(process_list, i);
                kill(p->pid, SIGKILL);
        }
        process_list_destroy();
}

void sighandler(int sig) {
	pid_t pid;
	switch(sig){
	case SIGINT:
		for (size_t i = 0; i < vector_size(process_list); i++) {
			process* p = (process*) vector_get(process_list, i);
			if(p->pid != getpgid(p->pid)) {
				kill(p->pid, SIGKILL);
				process_destroy(p->pid);
			}
		}
		break;
	case SIGCHLD:
		while ( (pid = waitpid((pid_t)(-1), 0, WNOHANG)) > 0) {
			process_destroy(pid);
		}
		break;
	default:
		break;
	}

}

int execute_command(char* command) {
	if(!strncmp(command, "cd", 2)) {
		int a = chdir(command+3); //lol, chod dir
		if (a < 0) {
			print_no_directory(command+3);
			return 1;
		} else {
			return 0;
		}
	} else {
		fflush(stdout);
		pid_t pid = fork();
		if(pid < 0) {
			print_fork_failed();
			exit(1);
		} else if(pid > 0) {
			process* p = process_create(command, pid);
			vector_push_back(process_list, p);
			if(command[strlen(command)-1] == '&') {
				if(setpgid(pid, pid) == -1) {
					print_setpgid_failed();
					exit(1);
				}
			} else {
				if(setpgid(pid, getpid()) == -1) {
					print_setpgid_failed();
					exit(1);
				}
				int s;
				pid_t rasalk = waitpid(pid, &s, 0);
				if(rasalk != -1) {
					process_destroy(rasalk);
					if(WIFEXITED(s) && WEXITSTATUS(s)) {
						return 1;
					}
				} else {
					print_wait_failed();
					exit(1);
				}
			}
		} else {
			//aight less goo
			if(command[strlen(command)-1] == '&') {
				command[strlen(command)-1] = '\0';
			}
			vector* comv = sstring_split(cstr_to_sstring(command), ' ');
			char* com[vector_size(comv)+1];
			for(size_t i = 0; i < vector_size(comv); i++) {
				com[i] = (char*) vector_get(comv, i);
			}
			if(!strcmp(com[vector_size(comv)-1], "")) {
				com[vector_size(comv)-1] = NULL;
			} else {
				com[vector_size(comv)] = NULL;
			}
			print_command_executed(getpid());
			execvp(com[0], com);
			print_exec_failed(com[0]);
			exit(1);
		}
	}
	return 0;
}

void kull_process(pid_t p) {
	for(size_t i = 0; i < vector_size(process_list); i++) {
		process* pro = (process*) vector_get(process_list, i);
		if(pro->pid == p) {
			kill(pro->pid, SIGKILL);
			print_killed_process(pro->pid, pro->command);
			process_destroy(pro->pid);
			return;
		}
	}
	print_no_process_found(p);
}
void stup_process(pid_t p) {
	for(size_t i = 0; i < vector_size(process_list); i++) {
		process* pro = (process*) vector_get(process_list, i);
		if(pro->pid == p) {
			kill(pro->pid, SIGTSTP);
			print_stopped_process(pro->pid, pro->command);
			return;
		}
	}
	print_no_process_found(p);
}
void cunt_process(pid_t p) {
	for(size_t i = 0; i < vector_size(process_list); i++) {
		process* pro = (process*) vector_get(process_list, i);
		if(pro->pid == p) {
			kill(pro->pid, SIGCONT);
			return;
		}
	}
	print_no_process_found(p);
}

int shell(int argc, char *argv[]) {
    // TODO: This is the entry point for your shell.

	signal(SIGINT, sighandler);
	signal(SIGCHLD, sighandler);
	process_list = shallow_vector_create();

	char* hist = NULL;
	char* com = NULL;

	if(argc != 1 && argc != 3 && argc != 5) {
		print_usage();
		return 0;
	}

	int c;
	while((c = getopt(argc, argv, "h:f:")) != -1) {
		switch(c) {
			case 'h':
				if(!hist && optarg) {
					hist = strdup(optarg);
				} else {
					print_usage();
					return 0;
				}
				break;
			case 'f':
				if(!com && optarg) {
					com = strdup(optarg);
				} else {
					print_usage();
					return 0;
				}
				break;
			default:
				print_usage();
				return 0;

		}
	}

	vector* history = string_vector_create();
	char* hist_path;
	FILE* hist_file;
	if(hist) {
		hist_path = get_full_path(hist);
		hist_file = fopen(hist_path, "r");
		if(!hist_file) {
			print_history_file_error();
		} else {
			char* lin1 = NULL;
			size_t s1 = 0;
			ssize_t b1;
			while(1){
				b1 = getline(&lin1, &s1, hist_file);
				if(b1 == -1){ break;}
				if(b1 > 0){
					if(lin1[b1-1] == '\n') {
						lin1[b1-1] = '\0';
						vector_push_back(history, lin1);
					}
				}
			}
			free(lin1);
			fclose(hist_file);
		}
	}

	FILE* file;
	if(com) {
		file = fopen(com, "r");
		if(!file) {
			print_script_file_error();
			exit(1);
		}
	} else {
		file = stdin;
	}

	//herewegoooo
	char* line1 = NULL;
	size_t shieet = 0;
	ssize_t size = 0;
	int escape = 0;
	while(1) {
		char* path = get_full_path("./");
		print_prompt(path, getpid());
		free(path);
		size = getline(&line1, &shieet, file);
		if(size == -1) {kill_stuff(); break;}
		if(size > 0 && line1[size-1]=='\n') {
			line1[size-1] = '\0';
			if(file != stdin) {print_command(line1);}
		}
		if(!strncmp(line1, "kill", 4)) {
			pid_t next_pid;
			int pid_exist;
			pid_exist = sscanf(line1+4, "%d", &next_pid);
			if(pid_exist != 1) {
				print_invalid_command(line1);
			} else {
				kull_process(next_pid);
			}
		} else if(!strncmp(line1, "stop", 4)) {
			pid_t next_pid;
			int pid_exist;
			pid_exist = sscanf(line1+4, "%d", &next_pid);
			if(pid_exist != 1) {
				print_invalid_command(line1);
			} else {
				stup_process(next_pid);
			}
		} else if(!strncmp(line1, "cont", 4)) {
			pid_t next_pid;
			int pid_exist;
			pid_exist = sscanf(line1+4, "%d", &next_pid);
			if(pid_exist != 1) {
				print_invalid_command(line1);
			} else {
				cunt_process(next_pid);
			}
		}
		else if(!strcmp(line1, "exit")) {
			kill_stuff();
			escape = 1;
			break;}
		else if(!strcmp(line1, "!history")) {
			for(size_t i = 0; i < vector_size(history); i++) {
				print_history_line(i, (char*) vector_get(history, i));
			}
		}
		else if(line1[0] == '!') {
			for(int i = vector_size(history) - 1; i >=0; i--) {
				char* dood = (char*) vector_get(history, i);
				if(line1[1] == '\0' || !strncmp(line1+1, dood, strlen(line1+1))) {
					print_command(dood);
					vector_push_back(history, dood);
					execute_command(dood);
					break;
				}
				if(i==0) {print_no_history_match();}
			}
		}
		else if(line1[0] == '#') {
			size_t ay, index;
			ay = sscanf(line1+1, "%zu", &index);
			if(index > vector_size(history) - 1 || ay==0) {
				print_invalid_index();
			} else {
				char* doo = (char*) vector_get(history, index);
				print_command(doo);
				vector_push_back(history, doo);
				execute_command(doo);
			}
		} else {
		//shiiet
		int fast = 0;
		vector_push_back(history, line1);
		sstring* s = cstr_to_sstring(line1);
		vector* result = sstring_split(s, ' ');
		sstring_destroy(s);
		for(size_t i = 0; i < vector_size(result); i++) {
			char* j = (char*) vector_get(result, i);
			if(!strcmp(j, "||")) {
				char* line2 = strtok(line1, "|");
                                line2[strlen(line2)-1] = '\0';
                                char* line3 = strtok(NULL, "");
                                line3 = line3+2;
                                if(execute_command(line2)) {
                                        execute_command(line2);
                                }
                                fast = 1;
			} else if(!strcmp(j, "&&")) {
				char* line2 = strtok(line1, "&");
				line2[strlen(line2)-1] = '\0';
				char* line3 = strtok(NULL, "");
				line3 = line3+2;
				if(execute_command(line2) == 0) {
					execute_command(line2);
				}
				fast = 1;
			} else if(j[strlen(j)-1] == ';') {
				char* line2 = strtok(line1, ";");
				char* line3 = strtok(NULL, "");
				line3 = line3+1;
				execute_command(line2);
				execute_command(line3);
				fast = 1;
			}
		}
		vector_destroy(result);
		if(fast == 0) {
			execute_command(line1);
		}}

	}

	if(com) {
		fclose(file);
	}
	if(hist) {
		FILE* fh = fopen(hist_path, "w");
		for(size_t i = 0; i < vector_size(history); i++) {
			fprintf(fh, "%s\n", (char*) vector_get(history, i));
		}
		fclose(fh);
		free(hist_path);
	}
	vector_destroy(history);
	if(escape) {exit(0);}
    return 0;
}
