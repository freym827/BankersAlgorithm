# BankersAlgorithm

This project is an implementation of the bankers algorithm. The bankers algorithm
is used to determine if a system with a number of processes, and resources allocated
to those processes, will be able to complete all of the processes succesfully based
on the needs of the processes and availability of the resources. 
https://en.wikipedia.org/wiki/Banker%27s_algorithm

If the processes can not be completed based on their needs in comparison to the 
available resources, the system will be in deadlock. 

This project takes the state of a system as input and examines the state to deduce whether
the system is in a safe state or not. If the system is in a safe state, the output
will say that and display the safe order of processes completed. If the system is not 
in a safe state, the output will say that and display the processes that could be
completed successfully. 

Example instructions for running are as follows:

Compile with: gcc bankers.c -o bankers

Run with: ./bankers bankersinput.txt

This project takes a text file as input. Example txt files are included in this
project. The required format is as follows:
  - The number of processes and resources are specified at the top of the file
    with a flag for the type of number and an equals sign, followed by the number. 
  - Three matrices are specified by flags: Allocation, Max, and Available. 
    Each flag must be present, with the brackets as shown: an open 
    bracket after the flag on the same line, and a close bracket on a new line 
    after the matrix. 
  - The matrices are layed out with spaces between the digits in a row, and a newline for
    each row. This will make as many rows as there are processes, and as many 
    columns as there are resources for both the Allocation and Max matrices.
    The Available matrix will be 1 x numResources. 
    
Example input and output:

Input 1:

![input1](/images/bankersinput1.png "input1")

Output 1:

![output1](/images/bankersoutput1.png "output1")

Input 2:

![input2](/images/bankersinput2.png "input2")

Output 2:

![output2](/images/bankersoutput2.png "output2")

Example error messages:

We have cases for:
  - No input file specified
  - Input file does not exist
  - A tag for a matrix is spelled incorrectly
  - The Max matrix contains a resource value for a process that is less than it's current allocation.

![error](/images/bankerserror.png "error")
