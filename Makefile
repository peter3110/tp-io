CPLEX_LIB       = /opt/IBM/ILOG/CPLEX_Studio1262/cplex/lib/x86-64_osx/static_pic/libcplex.a /opt/IBM/ILOG/CPLEX_Studio1262/cplex/lib/x86-64_osx/static_pic/libilocplex.a
LP_LIBS         = $(CPLEX_LIB)
INC_DIR		= -I/opt/IBM/ILOG/CPLEX_Studio1262/cplex/include/ -I/opt/IBM/ILOG/CPLEX_Studio1262/concert/include/

CCC = g++

CCFLAGS = -m64 -O2 -fPIC -fexceptions -DNDEBUG -DIL_STD

LDFLAGS = $(LP_LIBS) -lc -lm

SRCPRI = TP.cpp 

SOURCES = $(SRCPRI)

OBJECTS = $(SOURCES:.cpp=.o)

all: $(OBJECTS) 
	$(CCC) -o TP $(OBJECTS) $(CCFLAGS) $(LDFLAGS) -lpthread -lm

#
# produce the .o files from the .cc and .c files
#

%.o: %.cpp
	$(CCC) $(CCFLAGS) $(INC_DIR) -c $<
