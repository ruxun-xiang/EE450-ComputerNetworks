CC = g++
CFLAGS = -g -Wall

all : Scheduler.cpp HospitalA.cpp HospitalB.cpp HospitalC.cpp Client.cpp
	$(CC) $(CFLAGS) Scheduler.cpp -o scheduler
	$(CC) $(CFLAGS) HospitalA.cpp -o hospitalA
	$(CC) $(CFLAGS) HospitalB.cpp -o hospitalB
	$(CC) $(CFLAGS) HospitalC.cpp -o hospitalC
	$(CC) $(CFLAGS) Client.cpp -o client
	
scheduler : Scheduler.cpp
	$(CC) $(CFLAGS) scheduler.cpp -o scheduler
hospitalA : HospitalA.cpp
	$(CC) $(CFLAGS) HospitalA.cpp -o hospitalA
hospitalB : HospitalB.cpp
	$(CC) $(CFLAGS) HospitalB.cpp -o hospitalB
hospitalC : HospitalC.cpp
	$(CC) $(CFLAGS) HospitalC.cpp -o hospitalC
client : Client.cpp
	$(CC) $(CFLAGS) Client.cpp -o client




