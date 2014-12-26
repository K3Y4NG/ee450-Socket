all: healthcenterserver patient1 patient2 doctor
.PHONY : all

healthcenterserver: healthcenterserver.cpp
	g++ -o healthcenterserveroutput healthcenterserver.cpp
patient1: patient1.cpp udpclass.cpp tcpclientclass.cpp
	g++ -o patient1output patient1.cpp
patient2: patient2.cpp udpclass.cpp tcpclientclass.cpp
	g++ -o patient2output patient2.cpp
doctor: doctor.cpp udpclass.cpp
	g++ -o doctoroutput doctor.cpp

.PHONY : clean
clean:
	rm -f *output
