TARGET = glc
CC = cc
CFLAGS=-Wall -Wextra -Wno-unused-parameter
OBJS = \
	parser.o \
	listmaker.o \
	ntokenizer.o




$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) $(NODEBUGING) -c $< -o $@
clean:
	rm -f $(TARGET) $(OBJS)

