#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sstream>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>

using namespace std;

bool running = true;
string lineHeaderText = "May6431//Shell > ";

//Header Related Items
void StrToCommands(string input, char **command1);
void StrToPipeCommands(string input, char **command1, char **command2);
void CmdToFile(char **commands1, char **commands2);
void IssueCommand(char **commands);
void PipeCommand(char **commands1, char **commands2);
bool FindPipe(string input);
bool FindFile(string input);
bool FindExit(string input);
bool FindCD(string input);

int main()
{
	while(running)
	{
		string strIn;
		cout << lineHeaderText;
		getline(cin, strIn);
		char *command1[64];
		char *command2[64];
		
		if(FindExit(strIn))
		{	
			running = false;
			return 1;
		}
		else if (FindPipe(strIn))//handle pipe commands
		{
			int pid;
			cout << "Start Pipe Commands";
			StrToPipeCommands(strIn, command1, command2);
			cout << "Start Fork";
			pid = fork();
			if(pid<0)
			{
				cout << "ERROR(-1): Fork failed\n";
			}
			else if(pid == 0)
			{  
				cout << "Start Pipe";
				PipeCommand(command1, command2);
				cout << "Command Not Found\n";
			}
			else
			{
			   wait(NULL);
			}
		}
		else if (FindFile(strIn))//redirect to file output via piping
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
				CmdToFile(command1, command2);
				cout << "Command Not Found\n";
			}
			else
			{
			   wait(NULL);
			}
		}
		else if (FindCD(strIn))//since cd isnt a program
		{
			StrToCommands(strIn, command1);
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
		else
		{
			StrToCommands(strIn, command1);
			IssueCommand(command1);
		}
	}
	return 0;
}

void StrToCommands(string strIn, char **command1)
{
	char *input;
	input = const_cast<char*>(strIn.c_str());
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
		execvp(*commands1,commands1);
		cout << "Command Not Found\n";
	}
	else//Parent
	{
		dup2(p[1],1);
		close(p[0]);
		execvp("tee",commands2);
		cout << "Command Not Found\n";
   }
}

bool FindPipe(string input) 
{ 
	size_t found = input.find("|");
    return (found != string::npos);
}

bool FindFile(string input) 
{ 
	size_t found = input.find(">");
    return (found != string::npos);
}

bool FindExit(string input) 
{ 
	size_t found = input.find("exit");
    return (found != string::npos);
}

bool FindCD(string input) 
{ 
	size_t found = input.find("cd");
    return (found != string::npos);
}