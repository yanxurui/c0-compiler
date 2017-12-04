.PHONY: c0c clean

c0c: main.o error.o Table.o Middle.o Lexer.o Parser.o Generate.o Utility.o
	$(CXX) -o c0c $^

# main.o main.cpp:
# 	$(CXX) -c main.cpp

%.o: %.cpp %.h
	$(CXX) -c $<

clean:
	rm *.o c0c
