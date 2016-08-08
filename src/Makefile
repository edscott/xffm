FLAGS = -ggdb -O0 -std=c++11 `pkg-config --cflags gtk+-3.0`
LIBS = `pkg-config --libs gtk+-3.0` -lpthread
all: test
	
	
signals_c.o: signals_c.cpp signals_c.hpp
	g++ $(FLAGS) -c signals_c.cpp
utility_c.o: utility_c.cpp utility_c.hpp
	g++ $(FLAGS) -c utility_c.cpp
widgets_c.o: widgets_c.cpp widgets_c.hpp
	g++ $(FLAGS) -c widgets_c.cpp
notebook_page_c.o: notebook_page_c.cpp notebook_page_c.hpp
	g++ $(FLAGS) -c notebook_page_c.cpp
view_c.o: view_c.cpp view_c.hpp
	g++ $(FLAGS) -c view_c.cpp
test.o: test.cpp
	g++ $(FLAGS) -c test.cpp
OBJECTS = signals_c.o utility_c.o widgets_c.o notebook_page_c.o view_c.o test.o
test: $(OBJECTS)
	g++ $(LIBS) $(OBJECTS) -o test
	
clean:
	rm *.o test

