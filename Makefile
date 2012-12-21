
TARGET = libtwicpp.a

OBJS = \
	hash.o \
	oauth.o \
	oauth_http.o \
	sha1.o \
	socket.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(AR) r $@ $^

clean:
	-rm $(TARGET)
	-rm *.o

