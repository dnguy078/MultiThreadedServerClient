EXECUTABLES = itServer multiClient multiServer poolServer
OBJECTS = 
CXXFLAGS= -ggdb
CXX = g++
SOURCES = server.cpp multiclient.cpp multiserver.cpp poolserver.cpp

default:  $(EXECUTABLES) 

multiClient: multiclient.cpp
	$(CXX) $(CXXFLAGS) multiclient.cpp -lpthread -o multiClient

itServer: server.cpp
	$(CXX) $(CXXFLAGS) server.cpp -lpthread -o itServer

multiServer: multiserver.cpp
	$(CXX) $(CXXFLAGS) multiserver.cpp -lpthread -o multiServer

poolServer: poolserver.cpp
	$(CXX) $(CXXFLAGS) poolserver.cpp -lpthread -o poolServer



removedir: 
	rm -rf */ 
#conServer: conserver.cpp
#	$(CXX) $(CXXFLAGS) conserver.cpp -o conServer

#conClient: conclient.cpp
#	$(CXX) $(CXXFLAGS) conclient.cpp -o conClient

clean:
	rm -rf $(OBJECTS) $(EXECUTABLES) *.o *~ */ 
