#include "main.hpp"

#include <string>

bool             running = true;
OscSender*       ptr_osc_sender;
OscListener*     ptr_osc_listener;
DataHandler*     ptr_data_handler;
SerialConnector* ptr_serial_connector; // foo

int main(int argc, char* argv[])
{
    bool debug = (argc == 2 && std::string(argv[1]) == "-d");
    signal(SIGINT, (void (*)(int))handle_sigint);
    signal(SIGSEGV, handler_sigsev);

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

    SerialConnector serial_connector(&osc_sender, &data_handler, debug);
    ptr_serial_connector = &serial_connector;

    OscListener osc_listener(&data_handler, &serial_connector, debug);
    ptr_osc_listener = &osc_listener;
    osc_listener.start();

    /* ---------------------------------------------------- */
    /* -- Runs in main thread, therefore MUST start last -- */
    /* ---------------------------------------------------- */
    serial_connector.start();

    while (running) {
        sleep(5);
    }
    std::cout << BACO_YELLO << "Main thread terminated!" << BACO_END << std::endl;

    return 0;
}

void print_trace_gdb()
{
    char pid_buf[30];
    sprintf(pid_buf, "%d", getpid());
    char name_buf[512];
    name_buf[readlink("/proc/self/exe", name_buf, 511)] = 0;

    int child_pid = fork();
    if (!child_pid) {
        dup2(2, 1); // redirect output to stderr
        fprintf(stdout, "stack trace for %s pid=%s\n", name_buf, pid_buf);
        // execlp("gdb", "gdb", "--batch", "-n", "-ex", "bt", name_buf, pid_buf, NULL);
        execlp("gdb", "gdb", "--batch", "-n", "-ex", "thread apply all bt", name_buf, pid_buf, NULL);
        // if gdb failed to start
        abort();
    } else {
        waitpid(child_pid, NULL, 0);
    }
}

void handle_sigint()
{
    pid_t pid = getpid();
    std::cout << std::endl << BACO_YELLO << "Terminating main thread: " << pid << BACO_END << std::endl;
    running = false;
    ptr_osc_sender->stop();
    ptr_osc_listener->stop();
    ptr_serial_connector->stop();
}

void handler_sigsev(int sig)
{
    // void*  array[500];
    // size_t size;

    // // get void*'s for all entries on the stack
    // size = backtrace(array, 500);

    // // print out all the frames to stderr
    // fprintf(stderr, "\nError: signal %d:\n", sig);
    // backtrace_symbols_fd(array, size, STDERR_FILENO);

    print_trace_gdb();

    exit(1);

    // print_trace_gdb();
    // exit(1);
}
