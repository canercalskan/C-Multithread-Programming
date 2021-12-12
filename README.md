# C-Multithread-Programming
Multithread Programming with GNU Library
The multi-threaded application in this project will create a log file and monitor the progress of this activity in parallel.
The main thread running the “main()” function of the application spawns two concurrent threads:
i) the “create” thread that reads lines from stdin and writes them to a log file; and
ii) the “list” thread that tracks records added to the log file and displays them on a second pseudo-terminal dev/pts/1.

