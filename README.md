### Description ###
I built a non-blocking TCP servers for health center, it can accept multiple clients simultaneously without blocking. In this way, there is no specific input sequence for patient 1 client and patient 2 client. 

Because both patient 1 client and patient 2 client have the same TCP client operations, I created a class named "tcpclientclass" to support functions that a TCP client need to use.  

Foe the health center server, I created two class named "tcp_server" and "tcp_data" for two kinds of socket that server need to use, the one is for listen and accept new TCP connections, the other is for transmitting data between client and server.  

For UDP part, because the patient 1 client, patient 2 client and doctor server are all use UDP operations, and since UDP is connectionless protocol, there is no strictly rule that which one is server and which one is client. So I just created a class named "udpclass" to support all the operations that UDP need to use.  

And I just created one file for doctor, by using fork() function to finish multi-threads programming. Basically what I did is to create two child processes and one for doctor 1 and the other for doctor 2.  

### The project has three major phases: ###

1) authenticating into the health center server;

2) requesting available appointments and reserving one of them;

3) sending insurance information to the corresponding doctor to get an estimated cost for the visit.  

In Phases 1 and 2, all communications are through TCP sockets. In phase 3, however, all the communications are over UDP sockets. 

### The main components of this network architecture are: ###

1) one health center server which keeps track of available appointments and it is also in charge of giving out appointments to the requesting patients, 

2) two patients who are going to ask the health center server for available appointments and reserve one of them (the health center server will coordinate the process to avoid any conflicts of the appointments),

3) two doctors that can get requests from the patients and provide estimated costs of the visits


### Files ###

**healthcenterserver.cpp**
Create the TCP server instance by using class tcp_server, then listen to the connections and accept them. When the connections accepted, created TCP data instance by using class tcp_data. To archive non-blocking socket, I used select() to track which socket is active, when found an active socket, process it. Then the health center server can exchange information with patient clients just according to the project description.

**patient1.cpp**
**patient2.cpp**
Basically they are the same code, only the name of patient and name of input file are different.
When create socket instances by using class tcpclientclass and udpclass, then the processes are all according to the project description.

**doctor.cpp**
Two doctors processes, bind port and waiting for new data. When receive requests from the patient client, send cost to them. Because each patient could request to the same doctor, this doctor server is implemented on a loop mode. So the doctor need to be close by Ctrl+C. I used waitpid() to avoid zombie process.

**tcpclientclass.cpp**
Define and the implementation of class tcpclientclass, which are the operations TCP client uses.
Includes create socket, connect, send data, receive data, close socket and so on.

**udpclass.cpp**
Define and the implementation of class udpclass, which are the operations UDP instance uses.
Include create socket, bind, send data, receive data, close socket and so on.

**Makefile**
Just run "make" to compile all the programs.

### Running Order ###

1) Run "make" to compile all the codes.
2) Run "./healthcenterserveroutput" to start TCP server.
3) Run "./doctoroutput" to srart UDP server.
4) Run "./patient1output" and "./patient2output" to start patient process, no specific sequence.
5) Input the index number of time slot in two patient processes, no specific sequence.
6) Type "Ctrl+C" in doctor process to close the doctoroutput.
7) Type "Ctrl+C" in health center process to close the doctoroutput.
