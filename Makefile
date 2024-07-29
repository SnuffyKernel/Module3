CC = gcc
CFLAGS = -Wall -Werror -Wextra

SRC1 = task1/task1.c
TARGET1 = t1

SRC2 = task2/task2.c
SRC2_SUM = task2/sum.c
SRC2_CONCAT = task2/concat.c
TARGET2 = t2
TARGET2_SUM = sum
TARGET2_CONCAT = concat

SRC4 = task4/task4.c
TARGET4 = t4

SRC5 = task5/task5.c
TARGET5 = t5

TARGET6 = task6
SRC6_SENDER = task6/sender.c
TARGET6_SENDER = sender
SRC6_RECEIVER = task6/receiver.c
TARGET6_RECEIVER = receiver

TARGET7 = t6

all: $(TARGET1) $(TARGET2) $(TARGET4) $(TARGET5) $(TARGET6)

$(TARGET1):
	$(CC) $(CFLAGS) $(SRC1) -o $(TARGET1)

$(TARGET2):
	$(CC) $(CFLAGS) $(SRC2) -o $(TARGET2)
	$(CC) $(CFLAGS) $(SRC2_SUM) -o $(TARGET2_SUM)
	$(CC) $(CFLAGS) $(SRC2_CONCAT) -o $(TARGET2_CONCAT)

$(TARGET4):
	$(CC) $(CFLAGS) $(SRC4) -o $(TARGET4)

$(TARGET5):
	$(CC)  $(SRC5) -o $(TARGET5)

$(TARGET6):
	$(CC) $(CFLAGS) $(SRC6_SENDER) -o $(TARGET6_SENDER)
	$(CC) $(CFLAGS) $(SRC6_RECEIVER) -o $(TARGET6_RECEIVER)

clean:
	rm -f $(TARGET1) $(TARGET2) $(TARGET2_SUM) $(TARGET2_CONCAT) $(TARGET4) $(TARGET5)