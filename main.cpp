#include "main.hpp"

bool             running                 = true;
OscSender*       ptr_osc_sender          = NULL;
OscListener*     ptr_osc_listener        = NULL;
DataHandler*     ptr_data_handler        = NULL;
SerialConnector* ptr_serial_connector    = NULL;
SerialSender*    ptr_serial_sender       = NULL;
pthread_t*       thread_serial_sender    = NULL;
pthread_t*       thread_serial_connector = NULL;
pthread_t*       thread_osc_sender       = NULL;
pthread_t*       thread_osc_listener     = NULL;

int main(int argc, char* argv[])
{
    bool debug = (argc == 2 && std::string(argv[1]) == "-d");
    if((argc == 2 && std::string(argv[1]) == "-v")) {
        std::cout << "RainPots Runner by Jan Mech" << std::endl;
        std::cout << "version: " << VERSION << std::endl;
        return EXIT_SUCCESS;
    }
    signal(SIGINT, (void (*)(int))handle_sigint);
    signal(SIGSEGV, handler_sigsev);

    pid_t pid = getpid();
    if (debug) {
        std::cout << BACO_YELLO << "Started main thread: " << pid << BACO_END << std::endl;
    }

    DataHandler data_handler(debug);
    ptr_data_handler = &data_handler;
    data_handler.getParams(true);
    if (debug) {
        data_handler.printParamConfig();
    }

    OscSender osc_sender(&data_handler, debug);
    ptr_osc_sender    = &osc_sender;
    thread_osc_sender = osc_sender.start();

    SerialConnector serial_connector(&osc_sender, &data_handler, debug);
    ptr_serial_connector = &serial_connector;

    SerialSender serial_sender(debug);
    ptr_serial_sender = &serial_sender;

    OscListener osc_listener(&data_handler, &serial_connector, debug);
    ptr_osc_listener    = &osc_listener;
    thread_osc_listener = osc_listener.start();

    /* ---------------------------------------------------- */
    /* -- Runs in main thread, therefore MUST start last -- */
    /* ---------------------------------------------------- */
    thread_serial_connector = serial_connector.start();

    while (serial_connector.getFileDescriptor() == NULL) {
        usleep(200 * 1000);
        if (debug) {
            std::cout << BACO_YELLO << "SerialConnector Waiting for serial file descriptor..." << std::endl;
        }
    }
    if (debug) {
        std::cout << BACO_YELLO << "SerialConnector got file descriptor" << BACO_END << std::endl;
    }
    serial_sender.setFileDescriptor(serial_connector.getFileDescriptor());
    serial_sender.setMessageQueue(serial_connector.getMessageQueue());
    thread_serial_sender = serial_sender.start();

    while (running) {
        sleep(5);
    }
    std::cout << BACO_YELLO << "Main thread terminated!" << BACO_END << std::endl;

    return EXIT_SUCCESS;
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
    if (ptr_osc_sender != NULL) {
        pthread_kill(*thread_osc_sender, SIGKILL);
        // ptr_osc_sender->stop();
    }
    if (ptr_osc_listener != NULL) {
        pthread_kill(*thread_osc_listener, SIGKILL);
        // ptr_osc_listener->stop();
    }
    if (ptr_serial_connector != NULL) {
        pthread_kill(*thread_serial_connector, SIGKILL);
        // ptr_serial_connector->stop();
    }
    if (ptr_serial_sender != NULL) {
        pthread_kill(*thread_serial_sender, SIGKILL);
    }
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
