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
 * @file services/cmsoap/code/cms__soap.h
 *
 * Supplies all methods, which are supported by the Control & Monitoring SOAP
 * service. The method signature determines the required input arguments as
 * well as their type. The last argument of a method is the result of the
 * routine that will be returned to the client as response. Therefore the
 * result is filled by the method itself.
 */

//gsoap cms service name: cms
//gsoap cms service namespace: http://127.0.0.1/cms.wsdl
//gsoap cms service location: http://127.0.0.1

/**
 * Updates the lease of the client that sends the request.
 *
 * @param dummy A dummy argument, because no input arguments are not handled
 *              correctly.
 * @param result The result of the lease update action.
 * @return SOAP_OK if successfull, any other SOAP returncode if failed.
 */
int cms__updateLease(char* dummy, char** result);

/**
 * Initializes the C&M component for the client that sends the request. The
 * service supports multiple calls to this call by one client.
 *
 * @param dummy A dummy argument, because no input arguments are not handled
 *              correctly.
 * @param result The result of the initialization.
 * @return SOAP_OK if successfull, any other SOAP returncode if failed.
 */
int cms__initialise(char* dummy, char** result);

/**
 * Detaches the C&M component for the client that sends the request. All
 * used resources are freed for the client that sends the request. The
 * associated client cannot execute any action except for an initialise.
 *
 * @param dummy A dummy argument, because no input arguments are not handled
 *              correctly.
 * @param result The result of the initialization.
 * @return SOAP_OK if successfull, any other SOAP returncode if failed.
 */
int cms__detach(char* dummy, char** result);

/**
 * Creates a new participant for the client that sends the request. The service
 * must have been initialized by the client prior to calling this method.
 *
 * @param uri The uri for the domain where the participant must be created in.
 * @param timout The maximum amount of time to wait for the domain to be
 *               available when it is not available already (in milliseconds).
 * @param qos The quality of service for the participant.
 * @param result The XML representation of the created participant or NULL if
 *               it could not be created.
 * @return SOAP_OK if successfull, any other SOAP returncode if failed.
 */
int cms__participantNew(char* uri, int timeout, char* name, char* qos, char** result);

/**
 * Resolves all participants, which participate in the same domain as the supplied
 * participant. The supplied participant is also resolved.
 *
 * @param participant The XML representation of the participant. This
 *                    participant must have been resolved or created using this
 *                    service.
 * @param result The list of resolved participants in XML format or NULL if the
 *               supplied participant is not valid.
 * @return SOAP_OK if successfull, any other SOAP returncode if failed.
 */
int cms__participantAllParticipants(char* participant, char** result);

/**
 * Resolves all topics, which are available in the domain the supplied
 * participant participates in.
 *
 * @param participant The XML representation of the participant. This
 *                    participant must have been resolved or created using this
 *                    service.
 * @param result The list of resolved topics in XML format or NULL if the
 *               supplied participant is not valid.
 * @return SOAP_OK if successfull, any other SOAP returncode if failed.
 */
int cms__participantAllTopics(char* participant, char** result);

/**
 * Resolves all partitions, which are available in the domain the supplied
 * participant participates in.
 *
 * @param participant The XML representation of the participant. This
 *                    participant must have been resolved or created using this
 *                    service.
 * @param result The list of resolved partitions in XML format or NULL if the
 *               supplied participant is not valid.
 * @return SOAP_OK if successfull, any other SOAP returncode if failed.
 */
int cms__participantAllDomains(char* participant, char** result);

/**
 * Resolves all topics in the domain the supplied participant is participating
 * in and have a name that matches the supplied topicName.
 *
 * @param participant The XML representation of the participant. This
 *                    participant must have been resolved or created using this
 *                    service.
 * @param topicName The topic name expression that may contain
 *                  wildcard characters (*,?).
 * @param result The list of resolved topics in XML format or NULL if the
 *               supplied participant is not valid.
 * @return SOAP_OK if successfull, any other SOAP returncode if failed.
 */
int cms__participantFindTopic(char* participant, char* topicName, char** result);

/**
 * Tries to register the supplied type in the SPLICE-DDS domain where the
 * supplied participant is participating in.
 *
 * @param participant The XML representation of the participant. This
 *                    participant must have been resolved or created using this
 *                    service.
 * @param type The XML representation of the type to register.
 * @param result Whether or not the registration succeeded.
 * @return SOAP_OK if successfull, any other SOAP returncode if failed.
 */
int cms__participantRegisterType(char* participant, char* type, char** result);

/**
 * Frees the supplied XML entity and its user layer counterpart.
 *
 * @param entity The XML representation of an entity, which must be freed.
 * @param dummy A dummy result, because an empty result is not handled
 *              successfully.
 * @return SOAP_OK if successfull, any other SOAP returncode if failed.
 */
int cms__entityFree(char* entity, char** dummy);

/**
 * Resolves the communication status of the supplied entity.
 *
 * @param entity The XML representation of the entity where to resolve the
 *               current communication status of.
 * @param result The XML representation of the communication status of the
 *               supplied entity.
 * @return SOAP_OK if successfull, any other SOAP returncode if failed.
 */
int cms__entityGetStatus(char* entity, char** result);

/**
 * Resolves the quality of service of the supplied entity.
 *
 * @param entity The XML representation of the entity where to resolve the
 *               current quality of service of.
 * @param result The XML representation of the quality of service of the
 *               supplied entity.
 * @return SOAP_OK if successfull, any other SOAP returncode if failed.
 */
int cms__entityGetQos(char* entity, char** result);

/**
 * Applies the supplied quality of service to the supplied entity.
 *
 * @param entity The XML representation of the entity where to apply quality of
 *               service to.
 * @param result The result of the quality of service application.
 * @return SOAP_OK if successfull, any other SOAP returncode if failed.
 */
int cms__entitySetQos(char* entity, char* qos, char** result);

/**
 * Resolves the statistics of the supplied entity.
 *
 * @param entity The XML representation of the entity where to resolve the
 *               statistics of.
 * @param result The XML representation of the statistics.
 * @return SOAP_OK if successful, any other SOAP returncode if failed.
 */
int cms__entityStatistics(char* entity, char** result);

/**
 * Enables the supplied entity.
 *
 * @param entity The XML representation of the entity to enable.
 * @param result The result of the enable action.
 * @return SOAP_OK if successfull, any other SOAP returncode if failed.
 */
int cms__entityEnable(char* entity, char** result);

/**
 * Resets (a part of) the statistics of the supplied entity.
 *
 * @param entity The XML representation of the entity where to reset the
 *               statistics of.
 * @param fieldName The field name within the statistics to reset, or NULL
 *                  if all fields must be resetted.
 * @param result The result of the reset action.
 * @return SOAP_OK if successfull, any other SOAP returncode if failed.
 */
int cms__entityResetStatistics(char* entity, char* fieldName, char** result);

/**
 * Resolves the entities, which are owned by the supplied entity and match
 * the supplied filter.
 *
 * @param entity The XML representation of the entity where to resolve the owned
 *               entities of.
 * @param filter A filter that specifies the kind of entities to resolve.
 * @param result The list of owned entities in XML format.
 * @return SOAP_OK if successfull, any other SOAP returncode if failed.
 */
int cms__entityOwnedEntities(char* entity, char* filter, char** result);

/**
 * Resolves the entities, which depend on the supplied entity and match
 * the supplied filter.
 *
 * @param entity The XML representation of the entity where to resolve the
 *               dependant entities of.
 * @param filter A filter that specifies the kind of entities to resolve.
 * @param result The list of dependant entities in XML format.
 * @return SOAP_OK if successfull, any other SOAP returncode if failed.
 */
int cms__entityDependantEntities(char* entity, char* filter, char** result);

/**
 * Resolves the data type of the supplied topic.
 *
 * @param topic The XML representation of the topic where to resolve the data
 *              type of.
 * @param result The data type of the topic in XML format.
 * @return SOAP_OK if successfull, any other SOAP returncode if failed.
 */
int cms__topicDataType(char* topic, char** result);

/**
 * Resolves the data type of the userData in the database of the supplied reader.
 *
 * @param reader The XML representation of the reader where to resolve the data
 *               type of.
 * @param result The data type of the userData in the reader database in XML
 *               format.
 * @return SOAP_OK if successfull, any other SOAP returncode if failed.
 */
int cms__readerDataType(char* reader, char** result);

/**
 * Resolves the data type of the userData that is written by the supplied writer.
 *
 * @param writer The XML representation of the writer where to resolve the data
 *               type of.
 * @param result The data type of the userData that is written by the supplied
 *               writer in XML format.
 * @return SOAP_OK if successfull, any other SOAP returncode if failed.
 */
int cms__writerDataType(char* writer, char** result);

/**
 * Resolves the current state of the supplied service.
 *
 * @param service The XML representation of the service where to resolve the
 *                state of.
 * @param result The current state of the service in XML format.
 * @return SOAP_OK if successfull, any other SOAP returncode if failed.
 */
int cms__serviceGetState(char* service, char** result);

/**
 * Reads a sample from the supplied reader.
 *
 * @param reader The XML representation of the reader where to read a sample
 *               from.
 * @param result The read sample in XML format.
 * @return SOAP_OK if successfull, any other SOAP returncode if failed.
 */
int cms__readerRead(char* reader, char** result);

/**
 * Takes a sample from the supplied reader.
 *
 * @param reader The XML representation of the reader where to take a sample
 *               from.
 * @param result The taken sample in XML format.
 * @return SOAP_OK if successfull, any other SOAP returncode if failed.
 */
int cms__readerTake(char* reader, char** result);

/**
 * Reads the next sample from the supplied reader.
 *
 * @param reader The XML representation of the reader where to read a sample
 *               from.
 * @param localId Local id of the previous sample.
 * @param systemId Extended id of the previous sample.
 * @param result The read sample in XML format.
 * @return SOAP_OK if successfull, any other SOAP returncode if failed.
 */
int cms__readerReadNext(char* reader, char* localId, char* systemId, char** result);

/**
 * Makes a snapshot of the database of the supplied reader.
 *
 * @param reader The XML representation of the reader where to make a snapshot
 *               of.
 * @param result The constructed snapshot in XML format or NULL if not
 *               successfull.
 * @return SOAP_OK if successfull, any other SOAP returncode if failed.
 */
int cms__readerSnapshotNew(char* reader, char** result);

/**
 * Makes a snapshot of the history of the supplied writer.
 *
 * @param reader The XML representation of the writer where to make a snapshot
 *               of.
 * @param result The constructed snapshot in XML format or NULL if not
 *               successfull.
 * @return SOAP_OK if successfull, any other SOAP returncode if failed.
 */
int cms__writerSnapshotNew(char* writer, char** result);

/**
 * Frees the supplied snapshot.
 *
 * @param snapshot The XML representation of the snapshot to free.
 * @param empty Dummy result which will not be filled.
 * @return SOAP_OK if successfull, any other SOAP returncode if failed.
 */
int cms__snapshotFree(char* snapshot, char** empty);

/**
 * Reads a sample from the supplied snapshot.
 *
 * @param snapshot The XML representation of the snapshot where to read a sample
 *                 from.
 * @param result The read sample in XML format.
 * @return SOAP_OK if successfull, any other SOAP returncode if failed.
 */
int cms__snapshotRead(char* snapshot, char** result);

/**
 * Takes a sample from the supplied snapshot.
 *
 * @param snapshot The XML representation of the snapshot where to take a sample
 *                 from.
 * @param result The taken sample in XML format.
 * @return SOAP_OK if successfull, any other SOAP returncode if failed.
 */
int cms__snapshotTake(char* snapshot, char** result);

/**
 * Injects one instance of userData (XML) using the supplied writer.
 *
 * @param writer The XML representation of the writer where to inject the data
 *               with.
 * @param userData The XML representation of the userData to write. The data
 *                 type of the data must match the data type of the writer.
 * @param result Whether or not the write succeeded.
 * @return SOAP_OK if successfull, any other SOAP returncode if failed.
 */
int cms__writerWrite(char* writer, char* userData, char** result);

/**
 * Disposes one instance of userData (XML) using the supplied writer.
 *
 * @param writer The XML representation of the writer where to inject the data
 *               with.
 * @param userData The XML representation of the userData to dispose. The data
 *                 type of the data must match the data type of the writer.
 * @param result Whether or not the write succeeded.
 * @return SOAP_OK if successfull, any other SOAP returncode if failed.
 */
int cms__writerDispose(char* writer, char* userData, char** result);

/**
 * Disposes one instance of userData (XML) using the supplied writer.
 *
 * @param writer The XML representation of the writer where to inject the data
 *               with.
 * @param userData The XML representation of the userData to writeDispose. The data
 *                 type of the data must match the data type of the writer.
 * @param result Whether or not the write succeeded.
 * @return SOAP_OK if successfull, any other SOAP returncode if failed.
 */
int cms__writerWriteDispose(char* writer, char* userData, char** result);

/**
 * Disposes one instance of userData (XML) using the supplied writer.
 *
 * @param writer The XML representation of the writer where to inject the data
 *               with.
 * @param userData The XML representation of the userData to register. The data
 *                 type of the data must match the data type of the writer.
 * @param result Whether or not the write succeeded.
 * @return SOAP_OK if successfull, any other SOAP returncode if failed.
 */
int cms__writerRegister(char* writer, char* userData, char** result);

/**
 * Disposes one instance of userData (XML) using the supplied writer.
 *
 * @param writer The XML representation of the writer where to inject the data
 *               with.
 * @param userData The XML representation of the userData to unregister. The data
 *                 type of the data must match the data type of the writer.
 * @param result Whether or not the write succeeded.
 * @return SOAP_OK if successfull, any other SOAP returncode if failed.
 */
int cms__writerUnregister(char* writer, char* userData, char** result);

/**
 * Creates a new publisher and attaches it to the supplied participant.
 *
 * @param participant The XML representation of the participant to attach the
 *                    publisher to.
 * @param name The name for the publisher.
 * @param qos The quality of service for the publisher.
 * @param result The XML representation of the created publisher or NULL if
 *               the creation failed.
 * @return SOAP_OK if successfull, any other SOAP returncode if failed.
 */
int cms__publisherNew(char* participant, char* name, char* qos, char** result);

/**
 * Creates a new subscriber and attaches it to the supplied participant.
 *
 * @param participant The XML representation of the participant to attach the
 *                    subscriber to.
 * @param name The name for the subscriber.
 * @param qos The quality of service for the subscriber.
 * @param result The XML representation of the created subscriber or NULL if
 *               the creation failed.
 * @return SOAP_OK if successfull, any other SOAP returncode if failed.
 */
int cms__subscriberNew(char* participant, char* name, char* qos, char** result);

/**
 * Creates a new partition and attaches it to the supplied participant.
 *
 * @param participant The XML representation of the participant to attach the
 *                    partition to.
 * @param name The name for the partition.
 * @param result The XML representation of the created partition or NULL if
 *               the creation failed.
 * @return SOAP_OK if successfull, any other SOAP returncode if failed.
 */
int cms__domainNew(char* participant, char* name, char** result);

/**
 * Creates a new writer and attaches it to the supplied publisher.
 *
 * @param publisher The XML representation of the publisher to attach the
 *                    writer to.
 * @param name The name for the writer.
 * @param topic The topic that will be written by the writer.
 * @param qos The quality of service for the writer.
 * @param result The XML representation of the created writer or NULL if
 *               the creation failed.
 * @return SOAP_OK if successfull, any other SOAP returncode if failed.
 */
int cms__writerNew(char* publisher, char* name, char* topic, char* qos, char** result);

/**
 * Creates a new data reader and attaches it to the supplied subscriber.
 *
 * @param subscriber The XML representation of the subscriber to attach the
 *                   data reader to.
 * @param name The name for the data reader.
 * @param view The view expression for the data reader.
 * @param qos The quality of service for the data reader.
 * @param result The XML representation of the created data reader or NULL if
 *               the creation failed.
 * @return SOAP_OK if successfull, any other SOAP returncode if failed.
 */
int cms__dataReaderNew(char* subscriber, char* name, char* view, char* qos, char** result);

int cms__dataReaderWaitForHistoricalData(char* dataReader, int seconds, int nanoseconds, char** result);

/**
 * Creates a new query and attaches it to the supplied reader.
 *
 * @param reader The XML representation of the reader to attach the query to.
 * @param name The name for the query.
 * @param expression The query expression.
 * @param result The XML representation of the created query or NULL if
 *               the creation failed.
 * @return SOAP_OK if successfull, any other SOAP returncode if failed.
 */
int cms__queryNew(char* reader, char* name, char* expression, char** result);

/**
 * Creates a new topic and attaches it to the supplied participant. The topic
 * must be available in the domain already.
 *
 * @param participant The XML representation of the participant to attach the
 *                    topic to.
 * @param name The name of the topic.
 * @param typeName The type name of the topic.
 * @param keyList The comma seperated list of keys of the topic.
 * @param qos The quality of service for the topic.
 * @param result The XML representation of the created topic or NULL if
 *               the creation failed.
 * @return SOAP_OK if successfull, any other SOAP returncode if failed.
 */
int cms__topicNew(char* participant, char* name, char* typeName, char* keyList, char* qos, char** result);

/**
 * Makes the supplied publisher publish in the partitions that match the
 * supplied expression.
 *
 * @param publisher The XML representation of the publisher.
 * @param expression The partition expression for the publisher.
 * @param result The result of the publish action.
 * @return SOAP_OK if successfull, any other SOAP returncode if failed.
 */
int cms__publisherPublish(char* publisher, char* expression, char** result);

/**
 * Makes the supplied subscriber subscribe to the partitions that match the
 * supplied expression.
 *
 * @param subscriber The XML representation of the subscriber.
 * @param expression The partition expression for the subscriber.
 * @param result The result of the subscribe action.
 * @return SOAP_OK if successfull, any other SOAP returncode if failed.
 */
int cms__subscriberSubscribe(char* subscriber, char* expression, char** result);

/**
 * Creates a new waitset in the participant.
 *
 * @param participant The XML representation of the participant.
 * @param result The newly created waitset.
 * @return SOAP_OK if successfull, any other SOAP returncode if failed.
 */
int cms__waitsetNew(char* participant, char** result);

/**
 * Attaches the entity to the waitset.
 *
 * @param waitset The XML representation of the waitset.
 * @param entity The XML representation of the entity.
 * @param result The result of the attach action.
 * @return SOAP_OK if successfull, any other SOAP returncode if failed.
 */
int cms__waitsetAttach(char* waitset, char* entity, char** result);

/**
 * Detaches the entity from the waitset.
 *
 * @param waitset The XML representation of the waitset.
 * @param entity The XML representation of the entity.
 * @param result The result of the detach action.
 * @return SOAP_OK if successfull, any other SOAP returncode if failed.
 */
int cms__waitsetDetach(char* waitset, char* entity, char** result);

/**
 * Waits for events of entities in the waitset.
 *
 * @param waitset The XML representation of the waitset.
 * @param result The list of entities that had an event.
 * @return SOAP_OK if successfull, any other SOAP returncode if failed.
 */
int cms__waitsetWait(char* waitset, char** result);

/**
 * Waits for events of entities in the waitset for a specific amount of time.
 *
 * @param waitset The XML representation of the waitset.
 * @param seconds The amount of seconds to wait.
 * @param nanoseconds The amount of nanoseconds to wait.
 * @param result The list of entities that had an event.
 * @return SOAP_OK if successfull, any other SOAP returncode if failed.
 */
int cms__waitsetTimedWait(char* waitset, int seconds, int nanoseconds, char** result);

/**
 * Resolves the event mask of the waitset.
 *
 * @param waitset The XML representation of the waitset.
 * @param result The event mask of the waitset.
 * @return SOAP_OK if successfull, any other SOAP returncode if failed.
 */
int cms__waitsetGetEventMask(char* waitset, unsigned int* result);

/**
 * Applies the event mask to the waitset.
 *
 * @param waitset The XML representation of the waitset.
 * @param mask The event mask to apply to the waitset.
 * @param result Whether the applying of the mask succeeded.
 * @return SOAP_OK if successfull, any other SOAP returncode if failed.
 */
int cms__waitsetSetEventMask(char* waitset, unsigned int mask, char** result);

int cms__storageOpen(char* attrs, char** result);
int cms__storageClose(char* storage, char** result);
int cms__storageAppend(char* storage, char* metadata, char* data, char** result);
int cms__storageRead(char* storage, char** result);
int cms__storageGetType(char* storage, char* typeName, char** result);

int cms__getVersion(char* dummy, char** result);

int cms__entitiesStatistics(char* entities, char** result);
