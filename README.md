***Welcome to my custom shell project developed for my CS 341 course at UIUC!***

**Project Description**

This repository houses a custom shell program that I developed as part of my computer science coursework. The shell is a command-line interface that allows users to interact with their computer by accepting commands and executing corresponding programs. It was designed to meet specific learning objectives, including understanding the inner workings of a shell, process management, signal handling, and more.

**Features**

*Logical Operators*: The shell supports logical operators such as '&&,' '||,' and ';' to chain commands and control their execution flow.

*Background Processes*: You can run commands in the background by appending '&' to the command, allowing the shell to continue accepting new commands while the previous ones execute.

*Redirection Operators*: The shell enables you to redirect input and output, including '>', '>>,' and '<,' for more advanced command execution and file management.

*Built-in Commands*: It includes several built-in commands like 'cd' for changing directories, 'history' for command history tracking, and 'ps' for process monitoring.

*Signal Commands*: The shell supports sending signals like SIGKILL, SIGSTOP, and SIGCONT to its child processes.

*History and Command Recall*: You can easily recall and re-execute previous commands using the history feature.

**Tools and Technologies**

The shell program was developed using C and leverages the following key concepts and tools:

*C Programming*: The core of the shell is written in C, showcasing strong programming skills.

*Process Control*: The program demonstrates proficiency in process management, including forking, executing, and waiting for child processes.

*Signal Handling*: Signal handling is a fundamental aspect of the shell, allowing for the management of processes and user interruptions.

*Unix-like Environment*: The shell operates in a Unix-like environment, closely resembling the behavior of popular Unix shells.

**Project Structure**

_shell.c_: The source code for the custom shell program.
_format.h_: A customized formatting library for output messages.
README.md: You are here! This document provides an overview of the project.
