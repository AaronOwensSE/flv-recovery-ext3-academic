recover: io.o queue.o ext3_partition.o flv_iterator.o flv_recovery.o recover.o
	g++ -o recover io.o queue.o ext3_partition.o flv_iterator.o flv_recovery.o recover.o

recover.o: io.h queue.h ext3_partition.h flv_iterator.h flv_recovery.h recover.c
	g++ -c recover.c

flv_recovery.o: io.h queue.h ext3_partition.h flv_iterator.h flv_recovery.h flv_recovery.c
	g++ -c flv_recovery.c

flv_iterator.o: io.h queue.h ext3_partition.h flv_iterator.h flv_iterator.c
	g++ -c flv_iterator.c

ext3_partition.o: io.h queue.h ext3_partition.h ext3_partition.c
	g++ -c ext3_partition.c

queue.o: queue.h queue.c
	g++ -c queue.c

io.o: io.h io.c
	g++ -c io.c

clean:
	rm -f recovered_file.flv recover recover.o flv_recovery.o flv_iterator.o ext3_partition.o queue.o io.o
