PCMROOT=/opt/intel/pcm

TARGETS := matmul vecadd gpu_matmul

all: clean $(TARGETS)
clean:
	rm -f $(TARGETS) 2>/dev/null
%: %.cpp
	g++ -I../lec16/compute/include -I$(PCMROOT) -O3 -o $@ $^ -lOpenCL -lpthread -L$(PCMROOT)/pcm.so -lpcm

