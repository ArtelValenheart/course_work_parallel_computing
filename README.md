# course_work_parallel_computing 
## About compilation 
In order to compile the program you have to use a compiler with support for c++17 features (filesystem, structured bindings).
### About command line arguments
To run the program you have to use a command line (Windows) or terminal (Linux). To run it in command line/terminal you have to enter the path to executable and after that you have to enter parameters, which are in the next form:  
-t [thread_count] -p [path]  
thread_count - number of threads which the program will use.  
path - path to directory, which contains documents.  
You can index more than one directory:  
-t [thread_count] -p [path1] -p [path2] -p [path3] ...
