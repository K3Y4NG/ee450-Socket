EE450 Final Project - UNIX C Socket C/S communication 

In this project you will be simulating an online medical appointment system using a hybrid architecture of TCP and UDP sockets. 

The project has three major phases: 

1) authenticating into the health center server;

2) requesting available appointments and reserving one of them;

3) sending insurance information to the corresponding doctor to get an estimated cost for the visit.  

In Phases 1 and 2, all communications are through TCP sockets. In phase 3, however, all the communications are over UDP sockets. 

The main components of this network architecture are: 

1) one health center server which keeps track of available appointments and it is also in charge of giving out appointments to the requesting patients, 

2) two patients who are going to ask the health center server for available appointments and reserve one of them (the health center server will coordinate the process to avoid any conflicts of the appointments),

3) two doctors that can get requests from the patients and provide estimated costs of the visits
