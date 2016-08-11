FLAGS = -ggdb -O0 -std=c++11 `pkg-config --cflags gtk+-3.0`
LIBS = `pkg-config --libs gtk+-3.0` -lpthread
OBJECTS = xffm_c.o \
           window_c.o \
	    tooltip_c.o \
	    pixbuf_c.o \
		pixbuf_hash_c.o \
	   view_c.o  \
		widgets_c.o   \
	  signals_c.o \
	  utility_c.o \
	  gtk_c.o \
	  test.o

all: test
.SUFFIXES: .cpp .hpp .o

%.o: %.cpp %.hpp
	g++ -c $(FLAGS) $(CPPFLAGS) -o $@ $< 
test.o: test.cpp 
	g++ -c $(FLAGS) $(CPPFLAGS) -o test.o test.cpp
	
test: $(OBJECTS)
	g++ $(LIBS) $(OBJECTS) -o test
	
clean:
	rm *.o test

