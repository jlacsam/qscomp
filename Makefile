CP       = g++
SOURCES  = main.cpp iac.cpp quicksort.cpp bwt.cpp rle.cpp prep.cpp dict.cpp
INCLUDES = -I.
CPFLAGS  = -m64 -fPIC -Wall -DBSD=1 -std=c++17 -O3 -DDEBUG 
TARGET   = comp9

CP_OBJS  = $(SOURCES:.cpp=.o)

all: $(TARGET)

$(TARGET): $(CP_OBJS)
	$(CP) $(CPFLAGS) $(INCLUDES) -o $(TARGET) $(CC_OBJS) $(CP_OBJS)

.cpp.o:
	$(CP) $(INCLUDES) $(CPFLAGS) -c $< -o $@

clean:
	rm $(TARGET) *.o 
