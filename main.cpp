#include "ConfigParser.hpp"
#include "Networking.hpp"
#include <csignal>

bool g_interrupted = false;

void signalHandler(int signal) {
    if (signal == SIGINT) {
        std::cout << "Ctrl-C received. Exiting..." << std::endl;
        g_interrupted = true;
    }
}

int main(int ac, char **av){
	struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = signalHandler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
	if (ac == 1 || ac == 2){
		try
		{
			std::string configPath;
			configPath = (ac == 1 ? "conf_files/default_config.txt" : av[1]);
			ConfigParser parse(configPath);
			Networking net(parse);
			net.booting();
			if (sigaction(SIGINT, &sigIntHandler, NULL) != 0) {
				std::cerr << "Error setting up SIGINT signal handler" << std::endl;
				return 1;
			}
			while (!g_interrupted) {
				signal(SIGTSTP, SIG_IGN);
                net.runservers();
            }
            std::cout << "Exiting program..." << std::endl;
		}
		catch(const std::exception& e)
		{
			std::cerr << e.what();
			return (1);
		}
	}
	return (0);
}
