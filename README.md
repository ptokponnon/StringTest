# stringAlloc
This utilitiy program helps allocate small chuncks of memory dynamically (of variable size) from an initial pool of memory.
When there is no memory left, it can free automatically a configurable percentage of memory, starting by the 
oldest chuck.

That way it will never run out of memory. 
This kind of program is useful for logging kernel execution information for an operating system that does not 
have such advanced tracing features as Linux. The logs can be extracted later and used to debug the system.
