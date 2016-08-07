FLAGS=`pkg-config --cflags gtk+-3.0`
LIBS=`pkg-config --libs gtk+-3.0`
all: test
	
widgets_c.o: widgets_c.cpp
	g++ $(FLAGS) -c widgets_c.cpp
notebook_page_c.o: notebook_page_c.cpp
	g++ $(FLAGS) -c notebook_page_c.cpp
view_c.o: view_c.cpp
	g++ $(FLAGS) -c view_c.cpp
main.o: main.cpp
	g++ $(FLAGS) -c main.cpp
OBJECTS=widgets_c.o notebook_page_c.o view_c.o main.o
test: $(OBJECTS)
	g++ $({LIBS) $(OBJECTS) -o test

