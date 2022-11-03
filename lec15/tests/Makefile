#x86 compiler options for SIMD instruction set can be found at:
#https://gcc.gnu.org/onlinedocs/gcc/x86-Options.html
#Some of them are as following:
#-mmmx
#-msse
#-msse2
#-msse3
#-mssse3
#-msse4
#-msse4a
#-msse4.1
#-msse4.2
#-mavx
#-mavx2
#-mavx512f

VECFLAG = -msse4 -fopt-info-vec
TARGETS := vecadd vecmul matmul

all: clean $(TARGETS)
clean:
	rm -f $(TARGETS) 2>/dev/null
%: %.cpp
	g++ -std=c++17 -O3 $(VECFLAG) -I../vcl_include/version2 -o $@ $^

