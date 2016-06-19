Name: Zheng Luo

I finished the lab before the due time, but I forgot to write the read me file. So I finish writting the readme file in the next morning and commit it after the due time (October 5 23:59). I hope some hours of latency of the readme file will not affect the grade.



How long did the lab take?:

50 Hours in total, lab part1 and part2 in total.



Does your lab work as expected?

Almost as I expected.



Are there any issues?

Yes,there are some concepts that I do not know it very clearly,but I had tried to figure it out.

1.When we use the command & , the background process will become zombie after finish executing.

2.Most of the time, Ctrl + C works pretty well as I expected.It will terminate the forground processes. I had use the sleep command to test it. But sometimes there are problems with the Ctrl + C. Can you help me figure out the problems.



What design choices did you make and why?

lab1:
1.I just design the program using the method that you mention in class. Seprating the command into two categories, the inter commands and the external commands.

2.Since it is a lab, I use array to store the command, the size of the array is defined as MAXTOKEN, I can change it into the number we need.
     

lab2:
1.I use a 2D array to store the pipe in the parent process. Every time when executing a new command I will fork a new child process which is the child of the main parent processes.The executing result will be redirecting into the array of pipes. 

2. I use sigaction to implement the Ctrl + c,then I write act.sa_handler = signal_handler. I implement the signal_handler to process the Ctrl + c signal in the parent.
In each child process, since it will inherit the the sigacion, so I use the int sigignore(int sig) to ingore the Ctrl + c command in the background child processes.

