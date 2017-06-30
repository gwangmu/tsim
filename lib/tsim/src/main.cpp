#include <TSim/Simulation/Simulator.h>
#include <TSim/Simulation/Testbench.h>
#include <TSim/Utility/Logging.h>

#include <string>
#include <map>

using namespace std;

extern Testbench *simtb;


Simulator::Option LoadSimOption (int argc, char *argv[])
{
    Simulator::Option opt;

    for (int i = 0; i < argc; i++)
    {
        string arg = string (argv[i]);
        
        if (i + 1 < argc)
        {
            if (arg == "-p")
            {
                opt.tsinterval = stoi (string (argv[i + 1]));
                i++;
            }
            else if (arg == "-l")
            {
                opt.timelimit = stoi (string (argv[i + 1]));
                i++;
            }
        }
    }

    return opt;
}


int main (int argc, char *argv[])
{
    if (argc < 2) {
        PRINT ("usage: %s <simspec_file> [-p <timestamp interval>] [-l <timelimit>]", argv[0]);
        PRINT ("  -p\ttimestamp print interval (-1 = off, by default)");
        PRINT ("  -l\tsimulation time limit (-1 = infinity, by default)");
        return 0;
    }

    Simulator::Option opt = LoadSimOption (argc, argv);                
    Simulator sim (argv[1], opt);

    if (!sim.AttachTestbench (simtb))
        DESIGN_FATAL ("design error(s) detected. exiting..",
                simtb->GetName().c_str());

    PRINT ("");
    sim.ReportDesignSummary ();
    PRINT ("");

    sim.Simulate ();

    PRINT ("");
    sim.ReportSimulationSummary ();
    PRINT ("");
    sim.ReportActivityEvents();
    PRINT ("");

    PRINT ("Done.");

    return 0;
}
