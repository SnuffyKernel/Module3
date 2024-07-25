CC=gcc

CFLAGS= -Wall -Werror -Wextra

SRC=main.c error/error.c function_phone_book/function.c phone_book/phone_book.c file/file.c

TARGET=phone_book.out

LIB_SRC=error/error.c function_phone_book/function.c phone_book/phone_book.c

STATIC_LIB_NAME=libphonebook.a

DYNAMIC_LIB_NAME=libphonebook.so

all: build

build:
	$(CC)  $(CFLAGS) $(SRC) -o $(TARGET)

build_static: static_lib
	$(CC) $(CFLAGS) main.c -o $(TARGET) -L. $(STATIC_LIB_NAME)

static_lib: obj_static
	ar rcs $(STATIC_LIB_NAME) *.o

obj_static:
	$(CC) $(CFLAGS) -c $(LIB_SRC)

build_dynamic: dynamic_lib
	$(CC) $(CFLAGS) main.c -o $(TARGET) -L. -lphonebook && export LD_LIBRARY_PATH=.:$LD_LIBRARY_PATH

dynamic_lib: obj_dynamic
	$(CC) -shared -o $(DYNAMIC_LIB_NAME) *.o

obj_dynamic:
	$(CC) $(CFLAGS) -fPIC -c $(LIB_SRC)

clean:
	rm -rf $(TARGET) *.a *.so *.o