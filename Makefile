CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -I.

SRC = main.c Bike.c Car.c rental_Bike.c rental_Car.c rental_core.c
OBJ = $(SRC:.c=.o)
TARGET = main

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean
