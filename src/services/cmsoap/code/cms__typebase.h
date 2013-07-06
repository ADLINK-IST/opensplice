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
 
/**
 * @file services/cmsoap/code/cms__thread.h
 * 
 * Supplies all types for the Control & Monitoring SOAP service.
 */
#ifndef CMS__TYPEBASE_H
#define CMS__TYPEBASE_H
 
#include "c_typebase.h"
#include "c_iterator.h"
#include "os.h"
#include "u_service.h"
#include "u_serviceManager.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define CMS_RESULT_OK "<result>OK</result>"
#define CMS_RESULT_FAIL "<result>FAILED</result>"
#define CMS_CONTEXT "CM SOAP Service"

C_CLASS(cms_object);
C_CLASS(cms_service);
C_CLASS(cms_client);
C_CLASS(cms_thread);
C_CLASS(cms_soapThread);
C_CLASS(cms_configuration);

#define cms_object(obj) ((cms_object)(obj))

typedef enum cms_kind {
    CMS_SERVICE, CMS_THREAD, CMS_SOAPTHREAD, CMS_CLIENT, CMS_CONFIGURATION
} cms_kind;

/**
 * Base object for all cmsoap objects.
 */
C_STRUCT(cms_object){
    cms_kind kind; /* The cmsoap object kind.*/
};

/**
 * Represents the cmsoap service.
 */
C_STRUCT(cms_service){
    C_EXTENDS(cms_object);
    u_service uservice;                 /*The user layer service.*/
    u_serviceManager serviceManager;    /*The user layer service manager.*/
    struct soap* soap;                  /*The service SOAP environment.*/
    c_iter clients;                     /*List of current connected clients.(cms_client)*/
    os_mutex clientMutex;               /*Mutex for access to the list of clients.*/
    c_bool terminate;                   /*Determines if the service is currently terminating.*/
    cms_thread leaseThread;             /*Thread that takes care of updating the lease of the service.*/
    cms_thread garbageCollector;        /*Thread that collects garbage.*/
    c_iter clientGarbage;               /*List of disconnected clients.*/
    cms_configuration configuration;    /*Configuration of the service.*/
};

/**
 * Represents the configuration of the cmsoap service.
 */
C_STRUCT(cms_configuration){
    C_EXTENDS(cms_object);
    c_ulong maxClients;             /*The maximum number of concurrently connected clients.*/
    c_ulong maxThreadsPerClient;    /*The maximum number of concurrent threads for a client.*/
    c_ulong backlog;                /*The maximum number of requests that may await handling when service is 'full'.*/
    c_ulong verbosity;              /*The verbosity level of the service.*/
    c_ulong port;                   /*The port where the service is listening on.*/
    v_duration leasePeriod;         /*The lease period for the service.*/
    v_duration leaseRenewalPeriod;  /*The lease update period for the service.*/
    v_duration clientLeasePeriod;   /*The lease period for clients of the service.*/
    os_threadAttr clientScheduling; /*The properties for client thread creation*/
    os_threadAttr leaseScheduling;  /*The properties for the lease thread creation*/
    os_threadAttr garbageScheduling;/*The properites for the garbage collector thread*/
    c_ulong reportLevel;            /*The level on which reporting is done by the service*/
    c_bool reportEvents;            /*Defines if events are reported*/
    c_bool reportPeriodic;          /*Defines if reporting is done periodically*/ 
    c_bool reportOneShot;           /*Defines if reporting is done in one shot*/
    c_char * name;                  /*The name of the service.*/
    v_participantQos watchdogQos;              /**The properties for the watchdog thread*/
};

/**
 * Represents a cmsoap service thread.
 */
C_STRUCT(cms_thread){
    C_EXTENDS(cms_object);
    const c_char* name;     /*The name of the thread.*/
    os_threadId id;         /*The thread id of the thread.*/
    os_threadAttr attr;     /*The thread attributes of the thread.*/
    c_bool ready;           /*Whether or not the thread is ready to handle requests.*/
    c_bool terminate;       /*Whether or not the thread is terminating or should terminate.*/
    c_iter results;         /*The results of the thread.*/
    c_char* uri;            /*The the uri the soap service is connected to.*/
};

/**
 * Represents a thread of the cmsoap service that handles a specific client. It
 * contains a queue for SOAP requests of one specific clients and controls and
 * synchronizes the cms_soapThreads that can handle a request.
 */
C_STRUCT(cms_client){
    C_EXTENDS(cms_thread);
    unsigned long ip;           /*The IP address of the associated client.*/
    c_ulong initCount;          /*The number of initializations the client made.*/
    cms_service service;        /*The service the client is attached to.*/
    os_time leaseTime;          /*The last time the client updated its lease.*/
    c_iter soapEnvs;            /*The list of SOAP requests of the client that still need to be handled.*/
    os_mutex soapMutex;         /*Mutex for the soapEnvs list.*/
    os_cond condition;          /*Condition that is triggered when a new client request is recieved.*/
    os_mutex conditionMutex;    /*Mutex for the condition.*/
    c_bool internalFree;        /*Internal bool used to check if the thread should be freed after execution.*/
    c_iter threads;             /*The list of cms_soapThread's for the client.*/
    os_mutex threadMutex;       /*Mutex for the threads list.*/
};

/**
 * Represents a thread for a cms_client that is able to handle one request
 * at once.
 */
C_STRUCT(cms_soapThread){
    C_EXTENDS(cms_thread);
    cms_client client;      /*The client thread that owns the thread.*/
    struct soap* soap;      /*The current SOAP request that is handled.*/
    os_mutex soapMutex;     /*The mutex that synchronizes access to the current soap request.*/
    os_cond condition;      /*The condition that triggers the thread when a new request arrives.*/
};

#endif
