.. _`Appendix A`:


##########
Appendix A
##########

.. _`RnR Topic API IDL specification`:

RnR Topic API IDL specification
*******************************

:: 

   /* Record & Replay data model
    *
    * This IDL file contains the R&R data model. The file is divided in two sections:
    * helper types and topics that use these types.
    */

   #include "dds_dcps.idl"

   module RnR {
       /************************ TYPES ************************/

       /* ValueKind is the discriminator of the 'value' union of a KeyValue */
       enum ValueKind {
           VALUEKIND_STRING,
           VALUEKIND_LONG,
           VALUEKIND_FLOAT,
           VALUEKIND_BOOLEAN,
           VALUEKIND_TIME
       };

       /* ConditionKind the discriminator of the 'Condition' union type */
       enum ConditionKind {
           COND_REL_TIME,
           COND_ABS_TIME,
           COND_DATA,
           COND_LIFECYCLE
       };

       /* CommandKind is the discriminator of the 'kind' union of a Command */
       enum CommandKind {
           ADD_RECORD_COMMAND,
           REMOVE_RECORD_COMMAND,
           ADD_REPLAY_COMMAND,
           REMOVE_REPLAY_COMMAND,
           START_SCENARIO_COMMAND,
           STOP_SCENARIO_COMMAND,
           SUSPEND_SCENARIO_COMMAND,
           CONFIG_COMMAND,
           SETREPLAYSPEED_COMMAND,
           TRUNCATE_COMMAND,
           GENERIC_COMMAND
       };

       /* ServiceState contains the possible states of an R&R service */
       enum ServiceState {
           SERVICE_INITIALISING,    /* Service is starting */
           SERVICE_OPERATIONAL,     /* Builtin-scenario is started, service is able 
                                       to receive commands */
           SERVICE_TERMINATING,     /* Service is stopping all scenarios and shutting 
                                       down */
           SERVICE_TERMINATED       /* Service is terminated */
       };

       /* ScenarioState contains the possible states of a R&R scenario */
       enum ScenarioState {
           SCENARIO_RUNNING,        /* Scenario is active and able to receive and 
                                       process commands */
           SCENARIO_STOPPED,        /* Scenario is stopped and unable to receive 
                                       commands */
           SCENARIO_SUSPENDED       /* Scenario is suspended and will resume 
                                       processing commands when scenario is 
                                       (re)started or continued */
       };

       /* StorageState contains the possible states of a R&R storage */
       enum StorageState {
           STORAGE_READY,           /* Defined, but not opened yet. */
           STORAGE_OPEN,            /* Storage successfully opened */
           STORAGE_ERROR,           /* An unrecoverable error has occurred in the 
                                       storage */
           STORAGE_OUTOFRESOURCES,  /* Storage is out-of-resources */
           STORAGE_CLOSED           /* Storage has been closed */
       };

       /* Condition is a union, used to express conditions in the Command topic */
       union Condition switch (ConditionKind) {
           case COND_REL_TIME:             /* Relative time since previous command, */
               DDS::Duration_t relTime;    /* i.e. the time that has passed since the 
                                              previous command was processed */
           case COND_ABS_TIME:             /* Absolute (wall) time, */
               DDS::Time_t absTime;        /* i.e. a fixed point in time */
           case COND_DATA:                 /* Content-expression on data samples */
               string dataExpr;            /* i.e. a specific sample matching the 
                                              expression, was published in the DDS 
                                              domain */
           case COND_LIFECYCLE:            /* Content-expression on data lifecycle, */
               string lifecycleExpr;       /* i.e. a specific instance transitions 
                                              from alive to not alive */
       };

       union Value switch(ValueKind) {
           case VALUEKIND_STRING:       /* Value is a string */
               string sValue;
           case VALUEKIND_LONG:         /* Value is a long number */
               long lValue;
           case VALUEKIND_FLOAT:        /* Value is a floating-point number */
               float fValue;
           case VALUEKIND_BOOLEAN:      /* Value is a boolean */
               boolean bValue;
           case VALUEKIND_TIME:         /* Value is a timestamp */
               DDS::Time_t tValue;
       };

       /* Generic key:value type, where value is an union supporting various 
          kinds of values */
       struct KeyValue {
           string keyval;                   /* String key */
           Value value;
       };

       /* Used for specifying a range of times */
       /* For every valid TimeRange 'start' <= 'end' should hold */
       struct TimeRange {
           /* Absolute time (inclusive) indicating the start of the range. When
            * start.sec == TIME_INVALID_SEC and start.nanosec == TIME_INVALID_NSEC,
            * start is considered to be smaller than all times it is compared to
            * (i.e., start is interpreted as -INFINITY). */
           DDS::Time_t start;
           /* Absolute time (inclusive) indicating the end of the range. When
            * end.sec == TIME_INVALID_SEC and end.nanosec == TIME_INVALID_NSEC,
            * end is considered to be greater than all times it is compared to
            * (i.e., end is interpreted as +INFINITY). */
           DDS::Time_t end;
       };

       /* Command-type to add record-interest to a storage */
       struct AddRecordCommand {
           string storage;                         /* Name identifying a storage to 
                                                      record to */

           /* Meta-filters */
           sequence<string> interestExpr;          /* Sequence of 'partition.topic' 
                                                      expressions to record */
           sequence<string> blacklistExpr;         /* Sequence of 'partition.topic' 
                                                      expressions to block from 
                                                      record */

           /* Content filters */
           sequence<string> filterExpr;            /* Sequence of content-filter-
                                                      expressions */
           sequence<string> excludedAttributeExpr; /* Sequence of expressions to 
                                                      exclude specific members of 
                                                      topics */
       };

       /* Command-type to remove record-interest from a storage */
       struct RemoveRecordCommand {
           string storage;                         /* Name identifying a storage to 
                                                      stop recording to */

           /* Meta-filters */
           sequence<string> interestExpr;          /* Sequence of 'partition.topic' 
                                                      expressions to stop recording */
           sequence<string> blacklistExpr;         /* Sequence of 'partition.topic' 
                                                      expressions to stop blocking 
                                                      from record */

           /* Content filters */
           sequence<string> filterExpr;            /* Sequence of content-filter-
                                                      expressions */
           sequence<string> excludedAttributeExpr; /* Sequence of expressions to 
                                                      exclude specific members of 
                                                      topics */
       };

       /* Command-type to add replay-interest to a storage */
       struct AddReplayCommand {
           string storage;                         /* Name identifying a storage to 
                                                      replay from */

           /* Meta-filters */
           sequence<string> interestExpr;          /* Sequence of 'partition.topic' 
                                                      expressions to replay */
           sequence<string> blacklistExpr;         /* Sequence of 'partition.topic' 
                                                      expressions to block from 
                                                      replay */
           sequence<TimeRange> timeExpr;           /* Sequence of time-ranges to 
                                                      replay. When empty no filtering 
                                                      on time is done */

           /* Content filters */
           sequence<string> filterExpr;            /* Sequence of content-filter-
                                                      expressions */

           /* Resource limits */
           boolean useOriginalTimestamps;          /* If true, replay with original 
                                                      timestamps. If false use current 
                                                      time */

           /* If TRUE, fast-forward to first matching sample. If FALSE, a delay will 
            * be introduced before the sample is inserted, to resemble timing 
            * behaviour of the recording */
           boolean skipToFirstSample;
       };

       /* Command-type to remove replay-interest from a storage */
       struct RemoveReplayCommand {
           string storage;                         /* Name identifying a storage to 
                                                      stop replaying from */

           /* Meta-filters */
           sequence<string> interestExpr;          /* Sequence of 'partition.topic' 
                                                      expressions to stop replaying */
           sequence<string> blacklistExpr;         /* Sequence of 'partition.topic' 
                                                      expressions to stop blocking 
                                                      from replay */
           sequence<TimeRange> timeExpr;           /* Sequence of time-ranges to 
                                                      stop replaying */

           /* Content filters */
           sequence<string> filterExpr;            /* Sequence of content-filter-
                                                      expressions */
       };

       /* Command-type to set the replay-speed of a storage */
       struct SetReplaySpeedCommand {
           string storage;                         /* Name identifying a storage to 
                                                      replay from */
           float speed;                            /* Replay speed factor */
       };

       /* Container type of the per-topic storage statistics */
       struct TopicStatistics {
           string name;                            /* partition.topic name */
           long numberOfSamplesRecorded;           /* Total number of samples 
                                                      recorded */
           long numberOfBytesRecorded;             /* Total number of bytes 
                                                      recorded */
           long recordRateMinimum;                 /* Record rates (per publication 
                                                      period) */
           long recordRateAverage;
           long recordRateMaximum;
           long numberOfSamplesReplayed;           /* Total number of samples 
                                                      replayed */
           long numberOfBytesReplayed;             /* Total number of bytes 
                                                      replayed */
           long replayRateMinimum;                 /* Replay rates (per publication 
                                                      period) */
           long replayRateAverage;
           long replayRateMaximum;
       };

       union Kind switch(CommandKind) {
           case ADD_RECORD_COMMAND:                /* Record command */
               AddRecordCommand addRecord;
           case REMOVE_RECORD_COMMAND:
               RemoveRecordCommand removeRecord;
           case ADD_REPLAY_COMMAND:                /* Replay command */
               AddReplayCommand addReplay;
           case REMOVE_REPLAY_COMMAND:
               RemoveReplayCommand removeReplay;
           case CONFIG_COMMAND:                    /* Config command */
               sequence<KeyValue> config;
           case START_SCENARIO_COMMAND:            /* Scenario-control commands */
           case STOP_SCENARIO_COMMAND:
           case SUSPEND_SCENARIO_COMMAND:
               string name;
           case SETREPLAYSPEED_COMMAND:            /* Storage replay-speed command */
               SetReplaySpeedCommand setreplayspeed;
           case TRUNCATE_COMMAND:                  /* Storage truncate command */
               string storage;
           case GENERIC_COMMAND:                   /* For future extensibility */
               sequence<KeyValue> extCommands;
       };

       /************************ TOPICS ************************/

       /* Topic used to control an R&R service */
       struct Command {
           string scenarioName;        /* Name identifying the scenario to which 
                                          this command belongs */
           string rnrId;               /* Name identifying the service, or '*' 
                                          to address all services */
           Kind kind;
           sequence<Condition> conditions; /* Sequence of conditions which must 
                                              all be true before the command is 
                                              executed */
       };
   #pragma keylist Command scenarioName

       /* Topic used to monitor the status of an R&R service */
       struct ServiceStatus {
           string rnrId;               /* Name identifying the service */
           ServiceState state;         /* Current state of the service */
       };
   #pragma keylist ServiceStatus rnrId

       /* Topic used to monitor the status of an R&R scenario */
       struct ScenarioStatus {
           string rnrId;               /* Name identifying the service */
           string scenarioName;        /* Name identifying the scenario */
           ScenarioState state;        /* Current state of the scenario */
       };
   #pragma keylist ScenarioStatus scenarioName rnrId

       /* Topic used to monitor the status of a storage controlled by 
          an R&R service */
       struct StorageStatus {
           string rnrId;               /* Name identifying the service */
           string storageName;         /* Name identifying the storage */
           StorageState state;         /* Current state of the storage */
           string storageAttr;         /* Current storage attributes */

           sequence<KeyValue> properties;  /* key = property name, 
                                              value = property value */
       };
   #pragma keylist StorageStatus storageName rnrId

       /* Topic used to publish statistics of a storage */
       struct StorageStatistics {
           string rnrId;
           string storageName;
           sequence<TopicStatistics> statistics;
       };
   #pragma keylist StorageStatistics storageName rnrId
   };

   module RnR_V2 {
       /* In v2 of the RnR API, the following changes were made:
        * - a KeyValue sequence 'extensions' has been added for future 
        *   extensions of Command.
        * - The Add- and RemoveReplayCommand contain a KeyValue sequence 
        *   'transformations' for changing properties
        *   of samples upon replay.
        */

        /************************ TYPES ************************/

       /* Command-type to add replay-interest with transformations to a storage */
       struct AddReplayCommand {
           string storage;                         /* Name identifying a storage 
                                                      to replay from */

           /* Meta-filters */
           sequence<string> interestExpr;          /* Sequence of 'partition.topic' 
                                                      expressions to replay */
           sequence<string> blacklistExpr;         /* Sequence of 'partition.topic' 
                                                      expressions to block from 
                                                      replay */
           sequence<RnR::TimeRange> timeExpr;           /* Sequence of time-ranges to 
                                                           replay. When empty no 
                                                           filtering on time is 
                                                           done */

           /* Content filters */
           sequence<string> filterExpr;            /* Sequence of content-filter-
                                                      expressions */

           /* Resource limits */
           boolean useOriginalTimestamps;          /* If true, replay with original 
                                                      timestamps. If false use 
                                                      current time */

           /* If TRUE, fast-forward to first matching sample. 
            * If FALSE, a delay will be introduced before the sample 
            * is inserted, to resemble timing behaviour of the recording */
           boolean skipToFirstSample;

           /* Transformations */
           sequence<RnR::KeyValue> transformations;     /* QoS transformations to 
                                                           apply to the sample before 
                                                           replaying */
       };

       /* Command-type to remove replay-interest with transformations */
       struct RemoveReplayCommand {
           string storage;                         /* Name identifying a storage to 
                                                      stop replaying from */

           /* Meta-filters */
           sequence<string> interestExpr;          /* Sequence of 'partition.topic' 
                                                      expressions to stop replaying */
           sequence<string> blacklistExpr;         /* Sequence of 'partition.topic' 
                                                      expressions to stop blocking 
                                                      from replay */
           sequence<RnR::TimeRange> timeExpr;           /* Sequence of time-ranges to 
                                                           stop replaying */

           /* Content filters */
           sequence<string> filterExpr;            /* Sequence of content-filter-
                                                      expressions */

           /* Transformations */
           sequence<RnR::KeyValue> transformations;     /* QoS transformations  
                                                           to stop replaying */
       };

       union Kind switch(RnR::CommandKind) {
           case ADD_RECORD_COMMAND:                /* Record command */
               RnR::AddRecordCommand addRecord;
           case REMOVE_RECORD_COMMAND:
               RnR::RemoveRecordCommand removeRecord;
           case ADD_REPLAY_COMMAND:                /* Replay command */
               AddReplayCommand addReplay;
           case REMOVE_REPLAY_COMMAND:
               RemoveReplayCommand removeReplay;
           case CONFIG_COMMAND:                    /* Config command */
               sequence<RnR::KeyValue> config;
           case START_SCENARIO_COMMAND:            /* Scenario-control commands */
           case STOP_SCENARIO_COMMAND:
           case SUSPEND_SCENARIO_COMMAND:
               string name;
           case SETREPLAYSPEED_COMMAND:            /* Storage replay-speed command */
               RnR::SetReplaySpeedCommand setreplayspeed;
           case TRUNCATE_COMMAND:                  /* Storage truncate command */
               string storage;
           case GENERIC_COMMAND:                   /* For future extensibility */
               sequence<RnR::KeyValue> extCommands;
       };

       /************************ TOPICS ************************/

       /* Topic used to control an R&R service */
       struct Command {
           string scenarioName;        /* Name identifying the scenario to which
                                          this command belongs */
           string rnrId;               /* Name identifying the service, or '*' to 
                                          address all services */
           Kind kind;
           sequence<RnR::Condition> conditions; /* Sequence of conditions which 
                                                   must all be true before the 
                                                   command is executed */
           sequence<RnR::KeyValue> extensions;  /* Sequence reserved for future 
                                                   enhancements */
       };
   #pragma keylist Command scenarioName
   };


  

.. END
