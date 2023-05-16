BIN_FILES = servidor servidor_old
VPATH = lib tests  # path with source files not found in current dir
CC = gcc

CFLAGS = -g -I$(INSTALL_PATH)/include -Wall -fPIC
LDFLAGS = -L$(INSTALL_PATH)/lib/ -Wl,-R$(DIR)/lib/linux '-Wl,-R$$ORIGIN'
LDLIBS = -pthread -lm -lrt


all: $(BIN_FILES)
.PHONY : all

servidor: servidor.o server_impl.o linked_list.o lines.o log.o
	$(CC) $(CFLAGS) $(LDFLAGS) $(LDLIBS) $^ -o $@
	
servidor_old: servidor_old.o server_impl.o linked_list.o lines.o log.o
	$(CC) $(CFLAGS) $(LDFLAGS) $(LDLIBS) $^ -o $@


%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $<


tests: $(TEST_FILES)
.SUFFIXES:
.PHONY : tests


clean:
	rm -f $(BIN_FILES) *.o


.SUFFIXES:
.PHONY : clean