/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */

#include <os_stdlib.h>
#include <os_process.h>
#include <os_heap.h>
#include <os_firewall.h>
#include <os_defs.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define LINESIZE    120             /* Maximum length of a firewall rule    */

#define PORT_SPEC   "--dport %d "   /* destination port     */
#define SRC_SPEC    "-s %s/%s "     /* source host          */
#define PROT_SPEC   "-p %s -m %s "  /* tcp or udp           */
#define ACTION_SPEC "-j %s "        /* DROP or ACCEPT       */

#define FW_RULE     "-A INPUT " SRC_SPEC PROT_SPEC PORT_SPEC ACTION_SPEC
#define FW_END      "COMMIT"

char**
getFirewall()
{
    char buff[LINESIZE];
    FILE *pipeRead;
    int idx, len;
    char **dst;

    dst = os_malloc(sizeof(char*));
    idx = 0;
    if ((pipeRead = popen("/sbin/iptables-save", "r")) == NULL) {
        perror("getFirewall");
        dst = NULL;
    }

    /* Read firewall line by line */
    while ((fgets(buff, LINESIZE, pipeRead)) && (dst != NULL)) {
        len = strnlen(buff, LINESIZE);
        buff[len-1] = '\0'; /* remove newline-char */
        dst = os_realloc(dst, (idx+1) * sizeof(char*));
        dst[idx] = os_malloc(len*sizeof(char));
        strncpy(dst[idx], buff, len);
        idx++;
    }
    if (idx > 0) {
        dst = os_realloc(dst, (idx+1) * sizeof(char*));
        dst[idx] = NULL;
    } else {
        dst = NULL;
    }

    pclose(pipeRead);
    return dst;
}

os_result
setFirewall(char **fw) {
    FILE *pipeWrite;
    int idx;
    pipeWrite = NULL;
    if((pipeWrite = popen("/sbin/iptables-restore", "w")) == NULL) {
        perror("setFirewall");
        return os_resultFail;
    }

    idx = 0;
    while(fw[idx] != NULL) {
        if (strlen(fw[idx]) > 0) {
            fprintf(pipeWrite, "%s\n", fw[idx]);
        }
        idx++;
    }
    if ((pclose(pipeWrite)) < 0) {
        perror("setFirewall");
        return os_resultFail;
    }

    return os_resultSuccess;
}

char**
insertRule(char **fw, char *rule) {
    int idx, newIdx;
    char **newFw;

    if ((newFw = os_malloc(sizeof(char*))) != NULL) {
        idx = 0;
        newIdx = 0;
        while (fw[idx] != NULL) {
            if (strstr(fw[idx], FW_END)) {
                if ((newFw = os_realloc(newFw, (newIdx + 1) * sizeof(char*))) != NULL) {
                    newFw[newIdx] = os_strdup(rule);
                    newIdx++;
                } else {
                    printf("insertRule: os_realloc out of memory\n");
                    break;
                }
            }
            if ((newFw = os_realloc(newFw, (newIdx + 1) * sizeof(char*))) != NULL) {
                newFw[newIdx] = os_strdup(fw[idx]);
                idx++;
                newIdx++;
            } else {
                printf("insertRule: os_realloc out of memory\n");
                break;
            }
        }
        newFw = os_realloc(newFw, (newIdx + 1) * sizeof(char*));
        newFw[newIdx] = NULL;
    } else {
        printf("insertRule: os_malloc out of memory\n");
    }
    return newFw;
}

char*
newRule(
    struct sockaddr_in *host,
    int prot,
    os_portState state)
{
    char *protStr, *portStr, *hostStr, *actionStr, *prefix;
    char *tmp, *mask;
    char *fwRule;

    if (host != NULL) {
        if (host->sin_addr.s_addr == INADDR_ANY) {
            tmp = "0";
            mask = "0";
        } else {
            tmp = inet_ntoa(host->sin_addr);
            mask = "32";
        }
        hostStr = os_malloc(strlen(SRC_SPEC) + strlen(tmp) + strlen(mask));
        sprintf(hostStr, SRC_SPEC, tmp, mask);

        portStr = os_malloc(strlen(PORT_SPEC) + 5); /* 5 for max length of portnr */
        sprintf(portStr, PORT_SPEC, htons(host->sin_port));
    } else {
        hostStr = "";
        portStr = "";
    }

    if (prot != 0) {
        protStr = os_malloc(strlen(PROT_SPEC) + 6);
        if (prot == IPPROTO_TCP) {
            sprintf(protStr, PROT_SPEC, "tcp", "tcp");
        } else if (prot == IPPROTO_UDP) {
            sprintf(protStr, PROT_SPEC, "udp", "udp");
        }
    } else {
        protStr = "";
    }

    if (state == OPEN) {
        actionStr = os_malloc(strlen(ACTION_SPEC) + strlen("ACCEPT"));
        sprintf(actionStr, ACTION_SPEC, "ACCEPT");
    } else if (state == CLOSED) {
        actionStr = os_malloc(strlen(ACTION_SPEC) + strlen("DROP"));
        sprintf(actionStr, ACTION_SPEC, "DROP");
    } else {
        printf("Only block or open actions are currently supported!\n");
        /* no other states supported */
        return NULL;
    }

    prefix = "-A INPUT ";
    fwRule = os_malloc((strlen(prefix) + strlen(hostStr) + strlen(protStr) + strlen(portStr) + strlen(actionStr) + 1) * sizeof(char));
    sprintf(fwRule, "-A INPUT %s%s%s%s", hostStr, protStr, portStr, actionStr);
    return fwRule;
}

char*
getRule(char **fw, struct sockaddr_in *host, int prot)
{
    char *protStr, *portStr, *hostStr;
    char *tmp;
    char *match;
    int idx;

    if (host->sin_addr.s_addr == INADDR_ANY) {
        /* use ANY source host */
     /* tmp = "0";   */   /* not supported on redhat51 (iptables 1.3.5)*/
     /*   mask = "0"; */
        tmp = "";
    } else {
        tmp = inet_ntoa(host->sin_addr);
    /*    mask = "32"; */   /* not supported on redhat51 (iptables 1.3.5)*/
    }
 /* hostStr = os_malloc(strlen(SRC_SPEC) + strlen(tmp) + strlen(mask));
    sprintf(hostStr, SRC_SPEC, tmp, mask); */
    hostStr = os_strdup(tmp);

    portStr = os_malloc(strlen(PORT_SPEC) + 6); /* 6 for max length of portnr + trailing space */
    sprintf(portStr, PORT_SPEC, htons(host->sin_port));

    protStr = os_malloc(strlen(PROT_SPEC) + 6);
    if (prot == IPPROTO_TCP) {
        sprintf(protStr, PROT_SPEC, "tcp", "tcp");
    } else if (prot == IPPROTO_UDP) {
        sprintf(protStr, PROT_SPEC, "udp", "udp");
    }

    idx = 0;
    match = NULL;
    while (fw[idx] != NULL) {
       /* Match host */
       if (strstr(fw[idx], hostStr)) {
           /* Match port */
           if (strstr(fw[idx], portStr)) {
               /* Match protocol */
               if (strstr(fw[idx], protStr)) {
                   match = fw[idx];
                   break;
               }
           }
       }
       idx++;
    }
    os_free(portStr);
    os_free(protStr);
    os_free(hostStr);
    return match;

}

os_result
os_setPort(
    char *srcHost,
    int port,
    os_portState state,
    int prot)
{
    struct sockaddr_in host;
    char **fw;
    char *fwRule, *fwAction;
    os_result result;
    os_boolean done;

    result = os_resultSuccess;

    fw = getFirewall();
    if (fw != NULL) {
        host = resolveHost(srcHost, port);

        /* Check for pre-existing rule for this host/port/protocol and possibly remove it */
        fwRule = getRule(fw, &host, prot);
        done = OS_FALSE;
        if (fwRule != NULL) {
            if (state == OPEN) {
                fwAction = os_malloc((strlen(ACTION_SPEC) + strlen("ACCEPT"))
                    * sizeof(char));
                sprintf(fwAction, ACTION_SPEC, "ACCEPT");
                if ((strstr(fwRule, fwAction)) == NULL) {
                    /* Existing rule closes port, remove it */
                    *fwRule = '\0';
                } else {
                    /* Rule exists, no need to build a new one */
                    done = OS_TRUE;
                }
                os_free(fwAction);
            } else if (state == CLOSED) {
                fwAction = os_malloc((strlen(ACTION_SPEC) + strlen("DROP"))
                    * sizeof(char));
                sprintf(fwAction, ACTION_SPEC, "DROP");
                if ((strstr(fwRule, fwAction)) == NULL) {
                    /* Existing rule opens port, remove it */
                    *fwRule = '\0';
                } else {
                    /* Rule exists, no need to build a new one */
                    done = OS_TRUE;
                }
                os_free(fwAction);
            }
        }
        if (!done) {
            /* Build a new rule and insert it*/
            fwRule = newRule(&host, prot, state);
            if (fwRule != NULL && (fw = insertRule(fw, fwRule)) != NULL) {
                setFirewall(fw);
            } else {
                printf("Failed to insert new rule\n");
                if(fwRule == NULL) {
                    printf("Rule creation failed\n");
                } else {
                    printf("Rule insertion failed\n");
                }
                result = os_resultFail;
            }

        }
        os_free(fw);
    } else {
        printf("Failed to read firewall (IPTables not initialized?)\n");
        result = os_resultFail;
    }
    return result;
}



struct sockaddr_in
resolveHost(char *hostStr, int port)
{
    struct sockaddr_in result;
    result.sin_family = AF_INET;
    result.sin_port = htons(port);
    if (hostStr != NULL) {
        struct hostent *hp;
        in_addr_t ip;
        unsigned n1, n2, n3, n4;
        if ((sscanf(hostStr, "%u.%u.%u.%u", &n1, &n2, &n3, &n4)) == 4) { /* hostStr is an IP address */
            //printf("Host is ip: %s", hostStr);
            ip = inet_addr(hostStr);
            hp = gethostbyaddr(&ip, 4, AF_INET);
        } else if ((os_strcasecmp("0/0", hostStr)) == 0) {
            hp = NULL;
        } else {
            hp = gethostbyname(hostStr);
        }
        if (hp != NULL) {
            memcpy(&result.sin_addr, hp->h_addr_list[0], hp->h_length);
        } else {
            printf("Failed to resolve host: %s (Will use ANY host!)\n", hostStr);
            result.sin_addr.s_addr = htonl(INADDR_ANY);
        }
    } else {
        result.sin_addr.s_addr = htonl(INADDR_ANY);
    }
    return result;
}
