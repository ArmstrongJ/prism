CC = gcc

CFLAGS=-g

OBJS = compress.o db.o fcomp.o md5.o prism.o rfile.o util.o

TGT = prism

%.o: %.c 
	$(CC) $(CFLAGS) -c -o $@ $< 
	
$(TGT): $(OBJS)
	$(CC) -o $(TGT) $(OBJS) -lz

all: $(TGT)

clean:
	rm -f $(TGT) $(OBJS)
