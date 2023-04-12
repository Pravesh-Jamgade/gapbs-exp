TARGET=tc
include ../shared/Makefile.shared

CXX_FLAGS += -std=c++17 -Wall
PAR_FLAG = -fopenmp

ifneq (,$(findstring icpc,$(CXX)))
	PAR_FLAG = -openmp
endif

ifneq (,$(findstring sunCC,$(CXX)))
	CXX_FLAGS = -std=c++11  -m64 -xtarget=native
	PAR_FLAG = -xopenmp
endif

ifneq ($(SERIAL), 1)
	CXX_FLAGS += $(PAR_FLAG)
endif

$(TARGET): $(TARGET).o
	$(CXX) $(CXX_FLAGS) src/$(TARGET).o -lm $(SNIPER_LDFLAGS) -o $(TARGET)

$(TARGET).o:
	$(CXX) $(CXX_FLAGS) $(SNIPER_CFLAGS) -c src/$(TARGET).cc -o src/$(TARGET).o -g

run_$(TARGET):
	@echo "running snipper"
	../../run-sniper -v -n 1 -c gainestown --roi -- ./$(TARGET) -g 2 -n 1 

include test/test.mk

# Benchmark Automation
include benchmark/bench.mk

clean:
	rm -rf $(SUITE) test/out/* *.db $(TARGET) *.csv sim.* *.log *.out *.png *.o topo.* power.* $(CLEAN_EXA)
CLEAN_EXTRA=viz
