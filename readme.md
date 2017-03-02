# OS Lab 1 // May, Taylor

## Introduction

> This is based on the requirements set by the Lab 1 PDF provided under the assignment page.

> This program emulates the shell provided in UNIX systems, and runs all commands on a separate thread to prevent main thread failure.

## Features

> This program features a while loop that runs until terminated by the user.  

> When a command is issued by the user the program scans for certain values that require special handling such as ">", "|", "exit", and the cd command.

> Piped commands are separated into two separate commands, then using the C++ api, piped into eachother.

> Since cd is not a program that can be executed, the cd command was created using chdir, and the same command separator the rest of the commands use.
>> That being said I know the PDF said not to create commands but I did not see another way to accomplish making the cd command work.

> The users input string is turned into a char array, which is then either dispatched to a special handler(pipe), or handled by execvp(args[0], args)

> The command to file method also runs the method for the user to see on top of writing it to a file via the use of the tee program.

## Known Issues

> Occasionally when executing certain combinations of commands they will not exit, and you must hit enter again to reset the shell.
>> This is an inconsistent error that is not replicable when repeating issues that caused it.
