#include "main.hpp"

bool             running = true;
OscSender*       ptr_osc_sender;
OscListener*     ptr_osc_listener;
DataHandler*     ptr_data_handler;
SerialConnector* ptr_serial_connector;

void handle_sigint()
{
    pid_t pid = getpid();
    std::cout << std::endl << BACO_YELLO << "Terminating main thread: " << pid << BACO_END << std::endl;
    running = false;
    ptr_osc_sender->stop();
    ptr_osc_listener->stop();
    ptr_serial_connector->stop();
}

int main(int argc, char* argv[])
{
    bool debug = (argc == 2 && std::string(argv[1]) == "-d");
    signal(SIGINT, (void (*)(int))handle_sigint);

    pid_t pid = getpid();
    std::cout << BACO_YELLO << "Started main thread: " << pid << BACO_END << std::endl;

    DataHandler data_handler(debug);
    ptr_data_handler = &data_handler;
    data_handler.getParams(true);
    if (debug) {
        data_handler.printParamConfig();
    }

    OscSender osc_sender(&data_handler, debug);
    ptr_osc_sender = &osc_sender;
    osc_sender.start();

    /* ---------------------------------------------------- */
    /* -- Runs in main thread, therefore MUST start last -- */
    /* ---------------------------------------------------- */
    SerialConnector serial_connector(&osc_sender, &data_handler, debug);
    ptr_serial_connector = &serial_connector;

    OscListener osc_listener(&data_handler, &serial_connector, debug);
    ptr_osc_listener = &osc_listener;
    osc_listener.start();
    // osc_sender.addRNBOListenter();

    serial_connector.start();

    while (running) {
        sleep(5);
    }
    std::cout << BACO_YELLO << "Main thread terminated!" << BACO_END << std::endl;

    return 0;
}