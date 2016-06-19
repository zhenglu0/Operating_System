#
# Makefile for lab0
#

##############################################################
# Compiler and flags to use
##############################################################

CXX       = gcc
CXXFLAGS  = -g -Wall -Werror

##############################################################
# You shouldn't have to change anything below this line.
##############################################################

LAB0_EXE  = xssh
LAB0_OBJS = xssh.o

###

$(LAB0_EXE): $(LAB0_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(LAB0_OBJS)

clean:
	rm -f $(LAB0_EXE) $(LAB0_OBJS)

###

.c.o:
	$(CXX) $(CXXFLAGS) -c $<

###

xssh.o: xssh.c
