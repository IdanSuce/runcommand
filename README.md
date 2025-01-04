# runcommand
My Engrish is err ...so poor...

runcommand is a C++ based simple make-like tool.

There is just one source file `runcommand.cpp`. It's written under the compile-argument `g++ runcommand.cpp -o runcommand.exe -Wall -Werror -std=c++98 -O0`, gcc version 13.2.0 (Rev6, Built by MSYS2 project).


```python
# Usage of .\runcommand.exe
# Version: 5.0
# Pass paths of text files where there are option to be done by args. If no file is given, the program will read 'runcommand.txt' by default.
# In the text, one line for one command.
# For comment, start this line with '#'
# If you want to stop when some command goes error, start this line with '!'
# If you want to skip some command when the previous one succeeds, start this line with '|'
# If you want to skip some command when the previous one fails, start this line with '&'
# If you want to set a variable, start this line with '@'. The name of the variable should closely follow '@'. After the name there should be a space(note that not '\t'), from the first non-space character to the end of line is the value of this variable, including spaces.
# To use a variable, mention its name with '@' in a command line.
# There are a few built-in variables. They are switch options. They work when they are set and their value is not "false" (all small letters) 
# Following is a simple example.
@QUIT_WHEN_DONE
# when "@QUIT_WHEN_DONE" is set, you needn't to press enter to continue when a file ends.
@QUIT_WHEN_ERROR
# when "@QUIT_WHEN_ERROR" is set, the rest commands after a failed one with the mark '!' are skipped.
@HELP
# when "@HELP" is set, add this help message to the end of file.
command1
# command1 will be run.
! command2
# command2 will be run, regardless of whether command1 succeed. If it failed, the program will pause to ask whether this file should continue.
# If @QUIT_WHEN_ERROR is set, the rest commands will be skipped once command2 failed, without asking.
|! command3
# command3 is skipped if command2 succeeds. If command3 is not skipped and it failed, the program will pause as it did on command2.
command4
# command4 will be run.
& command5
# command5 is skipped if command4 fails.
@var1 a simple variable
# define a variable named "var1", with its value "a simple variable"
cd @var1 .txt
# use variable @var1. this command will be pre-processed into "cd a simple variable.txt". Note that a space is missed.# variable @@ will be pre-processed into @. A single @ will also be pre-processed into @, nothing changed.+Label with Space
# mark the beginning of a label.
command6
command7
# command6 and command7 will be run, with the value of var1 "a simple variable"
-Label with Space
# mark the end of a label.
@var1 myVar
# reset var1
:Label with Space
# run commands in the label. Here command6 and command7 will be run, with the value of var1 "myVar".
|:Label with Space
# all commands in the label will be run only when the previous command(here the second reach of command7) failed.


```
