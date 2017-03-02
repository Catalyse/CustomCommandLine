#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sstream>
#include <string>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

using namespace std;

bool running = true;
string lineHeaderText = "May6431//Shell > ";

//Header Related Items
void StrToCommands(char *line, char **argv);
void StrToPipeCommands(string input, char **command1, char **command2);
void StrToFileCommands(string input, char **command1, char **command2);
void StrFromFileCommands(string input, char **command1, char **command2);
void CmdToFile(char **commands1, char **commands2);
void CmdFromFile(char **commands1, char **commands2);
void IssueCommand(char **commands);
void PipeCommand(char **commands1, char **commands2);
bool IsPipe(string input);
bool IsFileOut(string input);
bool IsFileIn(string input);
bool IsCdCommand(string input);
bool IsExit(string input);

int main()
{
	while(running)
	{
		string strIn;
		char *input;
		cout << lineHeaderText;
		getline(cin, strIn);
		input = const_cast<char*>(strIn.c_str());
		char *command1[64];
		char *command2[64];
		
		if(IsExit(strIn))
		{	
			running = false;
			return 1;
		}
		else if (IsPipe(strIn))//handle pipe commands
		{
			int pid;
			StrToPipeCommands(strIn, command1, command2);
			pid = fork();
			if(pid<0)
			{
				cout << "ERROR(-1): Fork failed\n";
			}
			else if(pid == 0)
			{  
				PipeCommand(command1, command2);
				cout << "Command Not Found\n";
			}
			else
			{
			   wait(NULL);
			}
		}
		else if (IsFileOut(strIn))//redirect to file output via piping
		{
			int pid;
			StrToFileCommands(strIn, command1, command2);
			pid = fork();
			if(pid<0)
			{
				cout << "ERROR(-1): Fork failed\n";
			}
			else if(pid == 0)
			{  
				CmdToFile(command1, command2);
				cout << "Command Not Found\n";
			}
			else
			{
			   wait(NULL);
			}
		}
		else if (IsCdCommand(strIn))//since cd isnt a program
		{
			StrToCommands(input, command1);
			if(command1[1] == NULL)
			{
				chdir( getenv("HOME") );
			}
			else
			{
				string two = command1[1];
				if(two == "~" || two == " " || two == "")
				{
					char* home = getenv("HOME");
					chdir(home);
				}
				else 
					chdir(command1[1]);
			 }
		}
		else if (IsFileIn(strIn))
		{
			int pid;
			StrFromFileCommands(strIn, command1, command2);
			CmdFromFile(command1, command2);
		}
		else
		{
			StrToCommands(input, command1);
			IssueCommand(command1);
		}
	}
	return 0;
}

void StrToCommands(char *input, char **command1)
{
     while (*input != '\0') 
	 {
          while (*input == ' ' || *input == '\t' || *input == '\n')
               *input++ = '\0';
          *command1++ = input;
          while (*input != '\0' && *input != ' ' && *input != '\t' && *input != '\n') 
               input++;
     }
     *command1 = '\0';
}

void StrToPipeCommands(string input, char **command1, char **command2)
{
    int splitLoc = input.find(" | ");
	string s1 = input.substr(0,splitLoc);
	string s2 = input.substr(splitLoc+2, (input.length() - splitLoc+2));
	
	char *input1;
	char *input2;
	input1 = const_cast<char*>(s1.c_str());
	input2 = const_cast<char*>(s2.c_str());
	
	StrToCommands(input1, command1);
	StrToCommands(input2, command2);
}

void StrToFileCommands(string input, char **command1, char **command2)
{
    int splitLoc = input.find(" > ");
	string s1 = input.substr(0,splitLoc);
	string s2 = input.substr(splitLoc+2, (input.length() - splitLoc+2));
	
	char *input1;
	char *input2;
	input1 = const_cast<char*>(s1.c_str());
	input2 = const_cast<char*>(s2.c_str());
	
	StrToCommands(input1, command1);
	StrToCommands(input2, command2);
}

void StrFromFileCommands(string input, char **command1, char **command2)
{
    int splitLoc = input.find(" < ");
	string s1 = input.substr(0,splitLoc);
	string s2 = input.substr(splitLoc+2, (input.length() - splitLoc+2));
	
	char *input1;
	char *input2;
	input1 = const_cast<char*>(s1.c_str());
	input2 = const_cast<char*>(s2.c_str());
	
	StrToCommands(input1, command1);
	StrToCommands(input2, command2);
}

void IssueCommand(char **commands)//nonpipe
{
	pid_t pid;
	pid = fork();
	if(pid<0)
	{
		cout << "ERROR(-1): Fork failed\n";
		return;
	}
	else if(pid == 0)
	{  
		execvp(*commands,commands);
		cout << "Command Not Found\n";
	}
	else
	{
	   wait(NULL);
   }
}

void PipeCommand(char **commands1, char **commands2)//Pipe
{
	int p[2];
	pipe(p);
	
	pid_t pid;
	pid = fork();
	if(pid<0)
	{
		cout << "ERROR(-1): Fork failed\n";
		return;
	}
	else if(pid == 0)//child
	{  
		dup2(p[0],0);
		close(p[1]);
		execvp(*commands2,commands2);
		cout << "Command Not Found\n";
	}
	else//Parent
	{
		dup2(p[1],1);
		close(p[0]);
		execvp(*commands1,commands1);
		cout << "Command Not Found\n";
   }
}

void CmdToFile(char **commands1, char **commands2)//Pipe file write
{
	int p[2];
	pipe(p);
	
	pid_t pid;
	pid = fork();
	if(pid<0)
	{
		cout << "ERROR(-1): Fork failed\n";
		return;
	}
	else if(pid == 0)//child
	{  
		dup2(p[0],0);
		close(p[1]);
		execlp("tee","tee",commands2[0], (char *)NULL);
		cout << "Command Not Found\n";
	}
	else//Parent
	{
		dup2(p[1],1);
		close(p[0]);
		execvp(*commands1,commands1);
		cout << "Command Not Found\n";
   }
}

void CmdFromFile(char **commands1, char **commands2)//Pipe file write
{
	pid_t pid;
	pid = fork();
	if(pid<0)
	{
		cout << "ERROR(-1): Fork failed\n";
		return;
	}
	else if(pid == 0)//child
	{  
		int in = open(commands2[0], O_RDONLY);
        dup2(in, 0);
		close(in);
		execvp(*commands1, commands1);
	}
	else//Parent
	{
		wait(NULL);
   }
}
bool IsPipe(string input) 
{ 
	size_t found = input.find("|");
    return (found != string::npos);
}
 
bool IsFileOut(string input) 
{ 
	size_t found = input.find(">");
    return (found != string::npos);
}

bool IsFileIn(string input) 
{ 
	size_t found = input.find("<");
    return (found != string::npos);
}

bool IsCdCommand(string input)
{
	size_t found = input.find("cd");
    return (found != string::npos);
}

bool IsExit(string input)
{
	size_t found = input.find("exit");
    return (found != string::npos);
}