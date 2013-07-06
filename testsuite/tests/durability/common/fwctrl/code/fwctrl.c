#include <os_stdlib.h>
#include <os_firewall.h>

static char* execName;

void
printUsage()
{
    printf("Usage: %s <host> [tcp|udp] [port] [open|close]\n", execName);
    printf("Description:\n");
    printf("This tool can be used to block or allow traffic coming from a source host\n");
    printf("<host>       The IP or hostname of the source host\n");
    printf("tcp|udp      Block either tcp or udp traffic\n");
    printf("port         Number of the port to block (between 0 and 65.535\n");
    printf("open|close   Allow or block traffic\n");
}

int
main(int argc, char **argv)
{
    char *host;
    char *portStr, *tmpPort;
    int prot, idx, port;
    os_portState state;
    os_result result;
    result = os_resultFail;

    idx = 0;
    portStr = NULL;
    execName = argv[idx];
    idx++;

    if (argc < 4) {
        printUsage();
        exit(1);
    } else if (argc == 4) {
        host = NULL; /* Match any host */
    } else if (argc == 5) {
        host = argv[idx];
        idx++;
    } else {
        printUsage();
        exit(1);
    }

    if ((os_strncasecmp(argv[idx], "udp", strlen("udp"))) == 0) {
        prot = IPPROTO_UDP;
    } else if ((os_strncasecmp(argv[idx], "tcp", strlen("tcp"))) == 0) {
        prot = IPPROTO_TCP;
    } else {
        printf("Invalid protocol\n");
        exit(1);
    }
    idx++;

    portStr = argv[idx];
    idx++;

    if ((os_strncasecmp(argv[idx], "open", strlen("open"))) == 0) {
        state = OPEN;
    } else if ((os_strncasecmp(argv[idx], "close", strlen("close"))) == 0) {
        state = CLOSED;
    } else {
        printf("Invalid port state\n");
        exit(1);
    }

    tmpPort = NULL;
    tmpPort = strtok(portStr, ",");
    if (tmpPort != NULL) {
        result = os_resultSuccess;
    }
    while ((tmpPort != NULL) && (result == os_resultSuccess)) {
        port = atoi(tmpPort);
        if (port < 0 || port > 65535) {
            printf("Invalid port: %d", port);
            result = os_resultFail;
        } else {
            result = os_setPort(host, port, state, prot);
        }
        tmpPort = strtok(NULL, ",");
    }
    return result;

}
