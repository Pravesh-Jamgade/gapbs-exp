#include <bits/stdc++.h>
using namespace std;
#include <unistd.h>


#define M 1024
#define ML 1023

const int NUM_INSTR_DESTINATIONS_SPARC = 4;
const int NUM_INSTR_DESTINATIONS = 2;
const int NUM_INSTR_SOURCES = 4;

struct input_instr {
 char context = '0';
    uint64_t ip = 0;

    // branch info
    unsigned char is_branch = 0;
    unsigned char branch_taken = 0;

    unsigned char destination_registers[NUM_INSTR_DESTINATIONS] = {}; // output registers
    unsigned char source_registers[NUM_INSTR_SOURCES] = {};           // input registers

    uint64_t destination_memory[NUM_INSTR_DESTINATIONS] = {}; // output memory
    uint64_t source_memory[NUM_INSTR_SOURCES] = {};      
};

string cmd_fmtstr = "%1$s -dc %2$s";
string decomp_program = "xz";
string trace_string = "/media/pravesh/A/snipersim/pin_kit/extras/pinplay/scripts/xyzzz_champsim.trace.xz";
FILE* trace_file;

void open()
{
    char gunzip_command[4096];
    sprintf(gunzip_command, cmd_fmtstr.c_str(), decomp_program.c_str(), trace_string.c_str());
    trace_file = popen(gunzip_command, "r");
    if (trace_file == NULL) {
        std::cerr << std::endl << "*** CANNOT OPEN TRACE FILE: " << trace_string << " ***" << std::endl;
        assert(0);
    }
}
input_instr read_single_instr()
{
    input_instr trace_read_instr;

    while (!fread(&trace_read_instr, sizeof(input_instr), 1, trace_file)) {
        // reached end of file for this trace
        std::cout << "*** Reached end of trace: " << trace_string << std::endl;
        exit(0);
        // close the trace file and re-open it
        if (trace_file != NULL) {
            pclose(trace_file);
        }
        open();
    }

    return trace_read_instr;
}

int main()
{
    open();
    while(1)
    {
        input_instr tmp = read_single_instr();
        if(tmp.context != '0')
            cout << tmp.context << "," << tmp.ip << '\n';
    }
    return 0;
}