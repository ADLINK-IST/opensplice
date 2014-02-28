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
#ifndef CCPP_DATAREADER_H
#define CCPP_DATAREADER_H

#include "ccpp.h"
#include "ccpp_Entity_impl.h"
#include "ccpp_Subscriber_impl.h"
#include "ccpp_DataReaderView_impl.h"
#include "gapi.h"
#include "gapi_loanRegistry.h"
#include "os_report.h"
#include "os_abstract.h"
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#ifdef OSPL_CCPP_CHECK_COPYARG
#define CCPP_CHECK 0x8badf00d
#define CCPP_CHECK_STATIC_INIT CCPP_CHECK,
#else
#define CCPP_CHECK_STATIC_INIT
#endif /* OSPL_CCPP_CHECK_COPYARG */

namespace DDS
{
    struct ccpp_parDemContext;

    class DataReaderView_impl;

    class OS_DCPS_API DataReader_impl
        : public virtual ::DDS::DataReader,
          public ::DDS::Entity_impl
    {
        friend class ::DDS::Subscriber_impl;

    private:
        ccpp_parDemContext* pdc; /**< Parallel demarshalling context used by workers of this DataReader_impl */
        os_threadId* workers; /**< Array containing references to worker-thread. Can be safely indexed up to nrofWorkers */
        unsigned int nrofWorkers; /**< The number of workers started and a minimum size for the workers array */
        void * (*pdcMainFnc)(void *); /**< Typed parallel demarshalling main function */

        /**
         * Sets the number of workers available for parallel demarshalling. The
         * application thread itself counts as well, so the number of threads
         * actually started by this call is one less than the value returned.
         *
         * @param nrofThreads the number of threads to be used for parallel demarshalling.
         * @return The number of threads that will be used for parallel
         * demarshalling. This number may be lower than the provided nrofThreads
         * due to resource limitations.
         * @throws DDS::RETCODE_ERROR When an internal (fatal/unrecoverable)
         * error occurred.
         * @throws DDS::RETCODE_OUT_OF_RESOURCES When there were not enough
         * resources (memory/threads) available to fulfill the request. This
         * exception will not be thrown if at least one thread was started
         * successfully. In that case a warning is logged and the actual
         * number of threads available for parallel demarshalling (including
         * the application thread) is returned.
         */
        unsigned int set_ParallelReadThreadCount(unsigned int nrofThreads);

    protected:
        os_mutex dr_mutex;
        DataReader_impl(gapi_dataReader handle, void * (*)(void *));
       ~DataReader_impl();

        ::DDS::ReturnCode_t read (
            void * data_values,
            ::DDS::SampleInfoSeq & info_seq,
            DDS::Long max_samples,
            ::DDS::SampleStateMask sample_states,
            ::DDS::ViewStateMask view_states,
            ::DDS::InstanceStateMask instance_states
        ) THROW_ORB_EXCEPTIONS;

        ::DDS::ReturnCode_t take (
            void * data_values,
            ::DDS::SampleInfoSeq & info_seq,
            DDS::Long max_samples,
            ::DDS::SampleStateMask sample_states,
            ::DDS::ViewStateMask view_states,
            ::DDS::InstanceStateMask instance_states
        ) THROW_ORB_EXCEPTIONS;

        ::DDS::ReturnCode_t read_w_condition (
            void * data_values,
            ::DDS::SampleInfoSeq & info_seq,
            DDS::Long max_samples,
            ::DDS::ReadCondition_ptr a_condition
        ) THROW_ORB_EXCEPTIONS;

        ::DDS::ReturnCode_t take_w_condition (
            void * data_values,
            ::DDS::SampleInfoSeq & info_seq,
            DDS::Long max_samples,
            ::DDS::ReadCondition_ptr a_condition
        ) THROW_ORB_EXCEPTIONS;

        ::DDS::ReturnCode_t read_next_sample (
            void * data_values,
            ::DDS::SampleInfo & sample_info
        ) THROW_ORB_EXCEPTIONS;

        ::DDS::ReturnCode_t take_next_sample (
            void * data_values,
            ::DDS::SampleInfo & sample_info
        ) THROW_ORB_EXCEPTIONS;

        ::DDS::ReturnCode_t read_instance (
            void * data_values,
            ::DDS::SampleInfoSeq & info_seq,
            DDS::Long max_samples,
            ::DDS::InstanceHandle_t a_handle,
            ::DDS::SampleStateMask sample_states,
            ::DDS::ViewStateMask view_states,
            ::DDS::InstanceStateMask instance_states
        ) THROW_ORB_EXCEPTIONS;

        ::DDS::ReturnCode_t take_instance (
            void * data_values,
            ::DDS::SampleInfoSeq & info_seq,
            DDS::Long max_samples,
            ::DDS::InstanceHandle_t a_handle,
            ::DDS::SampleStateMask sample_states,
            ::DDS::ViewStateMask view_states,
            ::DDS::InstanceStateMask instance_states
        ) THROW_ORB_EXCEPTIONS;

        ::DDS::ReturnCode_t read_next_instance (
            void * data_values,
            ::DDS::SampleInfoSeq & info_seq,
            DDS::Long max_samples,
            ::DDS::InstanceHandle_t a_handle,
            ::DDS::SampleStateMask sample_states,
            ::DDS::ViewStateMask view_states,
            ::DDS::InstanceStateMask instance_states
        ) THROW_ORB_EXCEPTIONS;

        ::DDS::ReturnCode_t take_next_instance (
            void * data_values,
            ::DDS::SampleInfoSeq & info_seq,
            DDS::Long max_samples,
            ::DDS::InstanceHandle_t a_handle,
            ::DDS::SampleStateMask sample_states,
            ::DDS::ViewStateMask view_states,
            ::DDS::InstanceStateMask instance_states
        ) THROW_ORB_EXCEPTIONS;

        ::DDS::ReturnCode_t read_next_instance_w_condition (
            void * data_values,
            ::DDS::SampleInfoSeq & info_seq,
            DDS::Long max_samples,
            ::DDS::InstanceHandle_t a_handle,
            ::DDS::ReadCondition_ptr a_condition
        ) THROW_ORB_EXCEPTIONS;

        ::DDS::ReturnCode_t take_next_instance_w_condition (
            void * data_values,
            ::DDS::SampleInfoSeq & info_seq,
            DDS::Long max_samples,
            ::DDS::InstanceHandle_t a_handle,
            ::DDS::ReadCondition_ptr a_condition
        ) THROW_ORB_EXCEPTIONS;

        ::DDS::ReturnCode_t return_loan (
            void *dataBuf,
            void *infoBuf
        ) THROW_ORB_EXCEPTIONS;

        ::DDS::ReturnCode_t get_key_value (
            void * key_holder,
            ::DDS::InstanceHandle_t handle
        ) THROW_ORB_EXCEPTIONS;

        ::DDS::InstanceHandle_t lookup_instance (
            const void * instance
        ) THROW_ORB_EXCEPTIONS;

  public:
        virtual ::DDS::ReadCondition_ptr
        create_readcondition (
            ::DDS::SampleStateMask sample_states,
            ::DDS::ViewStateMask view_states,
            ::DDS::InstanceStateMask instance_states
        ) THROW_ORB_EXCEPTIONS;

        virtual ::DDS::QueryCondition_ptr
        create_querycondition (
            ::DDS::SampleStateMask sample_states,
            ::DDS::ViewStateMask view_states,
            ::DDS::InstanceStateMask instance_states,
            const char * query_expression,
            const ::DDS::StringSeq & query_parameters
        ) THROW_ORB_EXCEPTIONS;

        virtual ::DDS::ReturnCode_t
        delete_readcondition (
            ::DDS::ReadCondition_ptr a_condition
        ) THROW_ORB_EXCEPTIONS;

        virtual ::DDS::ReturnCode_t
        delete_contained_entities (
        ) THROW_ORB_EXCEPTIONS;

        virtual ::DDS::ReturnCode_t
        set_qos (
            const ::DDS::DataReaderQos & qos
        ) THROW_ORB_EXCEPTIONS;

        virtual ::DDS::ReturnCode_t
        get_qos (
            ::DDS::DataReaderQos & qos
        ) THROW_ORB_EXCEPTIONS;

        virtual ::DDS::ReturnCode_t
        set_listener (
            ::DDS::DataReaderListener_ptr a_listener,
            ::DDS::StatusMask mask
        ) THROW_ORB_EXCEPTIONS;

        virtual ::DDS::DataReaderListener_ptr
        get_listener (
        ) THROW_ORB_EXCEPTIONS;

        virtual ::DDS::TopicDescription_ptr
        get_topicdescription (
        ) THROW_ORB_EXCEPTIONS;

        virtual ::DDS::Subscriber_ptr
        get_subscriber (
        ) THROW_ORB_EXCEPTIONS;

        virtual ::DDS::ReturnCode_t get_sample_rejected_status (
          ::DDS::SampleRejectedStatus & a_status
        ) THROW_ORB_EXCEPTIONS;

        virtual ::DDS::ReturnCode_t get_liveliness_changed_status (
          ::DDS::LivelinessChangedStatus & a_status
        ) THROW_ORB_EXCEPTIONS;

        virtual ::DDS::ReturnCode_t get_requested_deadline_missed_status (
          ::DDS::RequestedDeadlineMissedStatus & a_status
        ) THROW_ORB_EXCEPTIONS;

        virtual ::DDS::ReturnCode_t get_requested_incompatible_qos_status (
          ::DDS::RequestedIncompatibleQosStatus & a_status
        ) THROW_ORB_EXCEPTIONS;

        virtual ::DDS::ReturnCode_t get_subscription_matched_status (
          ::DDS::SubscriptionMatchedStatus & a_status
        ) THROW_ORB_EXCEPTIONS;

        virtual ::DDS::ReturnCode_t get_sample_lost_status (
          ::DDS::SampleLostStatus & a_status
        ) THROW_ORB_EXCEPTIONS;

        virtual ::DDS::ReturnCode_t
        wait_for_historical_data (
            const ::DDS::Duration_t & max_wait
        ) THROW_ORB_EXCEPTIONS;

        virtual ::DDS::ReturnCode_t
        wait_for_historical_data_w_condition (
            const char * filter_expression,
            const ::DDS::StringSeq & filter_parameters,
            const ::DDS::Time_t & min_source_timestamp,
            const ::DDS::Time_t & max_source_timestamp,
            const ::DDS::ResourceLimitsQosPolicy & resource_limits,
            const ::DDS::Duration_t & max_wait
        ) THROW_ORB_EXCEPTIONS;

        virtual ::DDS::ReturnCode_t
        get_matched_publications (
            ::DDS::InstanceHandleSeq & publication_handles
        ) THROW_ORB_EXCEPTIONS;

        virtual ::DDS::ReturnCode_t
        get_matched_publication_data (
            ::DDS::PublicationBuiltinTopicData & publication_data,
            ::DDS::InstanceHandle_t publication_handle
        ) THROW_ORB_EXCEPTIONS;

        virtual ::DDS::DataReaderView_ptr
        create_view (
            const ::DDS::DataReaderViewQos & qos
        ) THROW_ORB_EXCEPTIONS;

        virtual ::DDS::ReturnCode_t
        delete_view (
            ::DDS::DataReaderView_ptr a_view
        ) THROW_ORB_EXCEPTIONS;

        virtual ::DDS::ReturnCode_t
        get_default_datareaderview_qos (
          ::DDS::DataReaderViewQos & qos
        ) THROW_ORB_EXCEPTIONS;

        virtual ::DDS::ReturnCode_t
        set_default_datareaderview_qos (
          const ::DDS::DataReaderViewQos & qos
        ) THROW_ORB_EXCEPTIONS;

        virtual ::DDS::ReturnCode_t
        set_property (
            const ::DDS::Property & prop
        ) THROW_ORB_EXCEPTIONS;

        virtual ::DDS::ReturnCode_t
        get_property (
            ::DDS::Property & prop
        ) THROW_ORB_EXCEPTIONS;
    };
    typedef DataReader_impl* DataReader_impl_ptr;

    struct ccpp_parDemContext {
        os_ushort               nrofWorkers; /* Number of worker threads. This is configured #threads - 1 (application thread participates as well) */
        /* heuristics */
        os_ushort               threshold; /* Number of samples threshold at which parallelization should be performed */
        /* copy_info */
        os_uint32               nextIndex; /* Next index to be read by worker thread */
        unsigned int            len;       /* The number of samples to be copied */
        void                   *dataSeq;  /* Ref to the data-sqeuence to copy-out to */
        ::DDS::SampleInfoSeq   *infoSeq;  /* Ref to the info-sqeuence to copy-out to */
        gapi_copyOut            copyOut;   /* Copy-out function pointer */
        gapi_dataSampleSeq     *samples;
        /* synchronisation */
        os_mutex                superMtx;  /* Mutex that should be held by the 'main' read thread in order to serialize concurrent reads on a parralelized reader */
        os_mutex                mtx;
        os_cond                 startCnd;  /* Condition for notifying worker threads that work is to be done */
        os_cond                 readyCnd;  /* Condition for notifying application thread that all work is done */
        os_ushort               readyCnt;  /* Counter for readyCnd broadcast */
        /* The parity is used for the loop-condition for readyCnd. Members of the
         * copy_info struct may be reused, so don't provide a safe loop-condition.
         * The parity toggles every loop and since it is not changed until all threads
         * are waiting on the next loop, it is a safe loop condition. */
        os_boolean              parity;
        os_ushort               terminate; /* Flag used to signal blocked threads to stop. */

        ccpp_parDemContext():
            nrofWorkers(0),
            threshold(2),
            nextIndex(0), /* Init not strictly needed here */
            len(0), /* Init not strictly needed here */
            dataSeq(NULL),
            infoSeq(NULL), /* Init not strictly needed here */
            copyOut(NULL), /* Init not strictly needed here */
            samples(NULL), /* Init not strictly needed here */
            parity(OS_FALSE),
            terminate(0)
        {
            os_mutexAttr mtxAttrs = { OS_SCOPE_PRIVATE };
            os_condAttr cndAttrs = { OS_SCOPE_PRIVATE };

            if (os_mutexInit(&superMtx, &mtxAttrs) != os_resultSuccess) {
                OS_REPORT(OS_FATAL, "CCPP", 0, "Unable to create mutex");
                goto err_supermtx_init;
            }
            if(os_mutexInit(&mtx, &mtxAttrs) != os_resultSuccess) {
                OS_REPORT(OS_FATAL, "CCPP", 0, "Unable to create mutex");
                goto err_mtx_init;
            }
            if(os_condInit(&startCnd, &mtx, &cndAttrs) != os_resultSuccess) {
                OS_REPORT(OS_FATAL, "CCPP", 0, "Unable to create condition");
                goto err_startcnd_init;
            }
            if(os_condInit(&readyCnd, &mtx, &cndAttrs) != os_resultSuccess) {
                OS_REPORT(OS_FATAL, "CCPP", 0, "Unable to create condition");
                goto err_readycnd_init;
            }
            return;

            /******************/
            /* Error-handling */
        err_readycnd_init:
            os_condDestroy(&startCnd); /* Ignore result */
        err_startcnd_init:
            os_mutexDestroy(&mtx); /* Ignore result */
        err_mtx_init:
            os_mutexDestroy(&superMtx); /* Ignore result */
        err_supermtx_init:
            throw ::DDS::RETCODE_ERROR;
        }

        ~ccpp_parDemContext(){
            os_condDestroy(&readyCnd); /* Ignore result */
            os_condDestroy(&startCnd); /* Ignore result */
            os_mutexDestroy(&mtx); /* Ignore result */
            os_mutexDestroy(&superMtx); /* Ignore result */
        }
    };

    struct ccpp_DataReaderCopy_ctx {
#ifdef OSPL_CCPP_CHECK_COPYARG
        os_uint32           check;
#endif /* OSPL_CCPP_CHECK_COPYARG */
        void               *data_buffer;
        ccpp_parDemContext *pdc;
    };

    template <bool REQUIRED>
    inline void
    ccpp_DataReaderParallelCopyReadyBroadcast(
        ccpp_parDemContext * pdc)
    {
        assert(pdc);

        pdc->dataSeq = NULL;
        pdc->infoSeq = NULL;
        pdc->parity = pdc->parity == OS_TRUE ? OS_FALSE : OS_TRUE;
        os_condBroadcast(&pdc->readyCnd);
    }

    template <class DataSeq>
    void
    ccpp_DataReaderParallelCopy(
        ccpp_parDemContext *pdc)
    {
        assert(pdc);
        assert(pdc->len);
        assert(pdc->dataSeq);
        assert(pdc->infoSeq);
        assert(pdc->copyOut);
        assert(pdc->samples);

        os_uint32 i;
        DataSeq *data_seq = reinterpret_cast<DataSeq *>(pdc->dataSeq);
        ::DDS::SampleInfoSeq *info_seq = reinterpret_cast< ::DDS::SampleInfoSeq * >(pdc->infoSeq);
        while((i = pa_increment(&pdc->nextIndex) - 1) < pdc->len){
            pdc->copyOut ( pdc->samples->_buffer[i].data, &(*data_seq)[i]);
            ccpp_SampleInfo_copyOut( pdc->samples->_buffer[i].info, (*info_seq)[i] );
        }
    }

    template <class DataSeq>
    void *
    ccpp_DataReaderParallelDemarshallingMain(
        void * arg /* DDS::ccpp_parDemContext */)
    {
        DDS::ccpp_parDemContext *pdc = static_cast<DDS::ccpp_parDemContext *>(arg);
        if(os_mutexLock(&pdc->superMtx) != os_resultSuccess) goto err_mtx_lock;
        if(os_mutexLock(&pdc->mtx) != os_resultSuccess) goto err_double_mtx_lock;
        pdc->nrofWorkers++;
        os_mutexUnlock(&pdc->mtx);
        os_mutexUnlock(&pdc->superMtx);

        for (;;)
        {
            if(os_mutexLock(&pdc->mtx) != os_resultSuccess) goto err_mtx_lock;
            while(!pdc->terminate && !pdc->dataSeq){
                if(os_condWait(&pdc->startCnd, &pdc->mtx) != os_resultSuccess) goto err_condwait;
            }
            os_mutexUnlock(&pdc->mtx);
            /* pdc->terminate is guaranteed to only toggle to TRUE in a lock, and it
             * is only toggled back to FALSE in a single-threaded context, so reading
             * outside the lock is OK. */
            if(pdc->terminate) break;

            ccpp_DataReaderParallelCopy<DataSeq>(pdc);

            if(os_mutexLock(&pdc->mtx) != os_resultSuccess) goto err_mtx_lock;
            pdc->readyCnt++;
            if(pdc->readyCnt > pdc->nrofWorkers){
                /* Main thread was already done, so notify */
                ccpp_DataReaderParallelCopyReadyBroadcast<true>(pdc);
            } else {
                const os_boolean parity = pdc->parity;
                while(parity == pdc->parity){
                    os_condWait(&pdc->readyCnd, &pdc->mtx);
                }
            }
            os_mutexUnlock(&pdc->mtx);
        }

        if(os_mutexLock(&pdc->superMtx) != os_resultSuccess) goto err_mtx_lock;
        if(os_mutexLock(&pdc->mtx) != os_resultSuccess) goto err_double_mtx_lock;
        pdc->nrofWorkers--;
        if(pdc->nrofWorkers == 0){
            /* This is single-threaded access now, so pdc->terminate can be toggled
             * back to 0 outside the pdc->mtx lock. */
            pdc->terminate = 0U;
        }
        os_mutexUnlock(&pdc->mtx);
        os_mutexUnlock(&pdc->superMtx);

        return NULL;

    err_double_mtx_lock:
        os_mutexUnlock(&pdc->superMtx);
    err_mtx_lock:
        return NULL;

    err_condwait:
        os_mutexUnlock(&pdc->mtx);
        return NULL;
    }

    template <class DataSeq, class DataType>
    void
    ccpp_DataReaderCopy (
        gapi_dataSampleSeq *samples,
        gapi_readerInfo    *info
    )
    {
        unsigned int i, len;
        ccpp_DataReaderCopy_ctx *ctx = static_cast<ccpp_DataReaderCopy_ctx *>(info->data_buffer);
#ifdef OSPL_CCPP_CHECK_COPYARG
        assert(ctx->check == CCPP_CHECK);
#endif
        ccpp_parDemContext *pdc = ctx->pdc;
        DataSeq *data_seq = reinterpret_cast<DataSeq *>(ctx->data_buffer);
        ::DDS::SampleInfoSeq *info_seq = reinterpret_cast< ::DDS::SampleInfoSeq * >(info->info_buffer);

        if (samples) {
            len = samples->_length;
        } else {
            len = 0;
            data_seq->length(len);
            info_seq->length(len);
        }

        if ( (info->max_samples != (gapi_unsigned_long)GAPI_LENGTH_UNLIMITED) && (len > info->max_samples) ) {
            len = info->max_samples;
        }
        else if ( data_seq->maximum() > 0 && data_seq->maximum() < len ) {
            len = data_seq->maximum();
        }

        if ( len > 0 ) {
            if ( data_seq->maximum() == 0 ) {
                DataType *dataBuf = DataSeq::allocbuf(len);
                ::DDS::SampleInfo *infoBuf = ::DDS::SampleInfoSeq::allocbuf(len);
                data_seq->replace(len, len, dataBuf, false);
                info_seq->replace(len, len, infoBuf, false);
                if (*(info->loan_registry) == NULL) {
                    *(info->loan_registry) = gapi_loanRegistry_new();
                }
                gapi_loanRegistry_register((gapi_loanRegistry)*(info->loan_registry),
                                           dataBuf,
                                           infoBuf);
            } else {
                data_seq->length(len);
                info_seq->length(len);
            }

            if (len > 0) {
                os_boolean singleThreadedCopy = OS_TRUE;

                if(pdc && len >= pdc->threshold){
                    /* By keeping the pdc->superMtx locked, concurrent reads on
                     * a parallelized reader be serialized. It furthermore
                     * prevents new workers to be added/removed while copying. */
                    if(os_mutexLock(&pdc->superMtx) == os_resultSuccess){
                        if(pdc->nrofWorkers > 0){
                            if(os_mutexLock(&pdc->mtx) == os_resultSuccess){
                                pdc->readyCnt = 0U;
                                pdc->nextIndex = 0U;
                                pdc->len = len;
                                pdc->dataSeq = data_seq;
                                pdc->infoSeq = info_seq;
                                pdc->copyOut = info->copy_out;
                                pdc->samples = samples;
                                os_condBroadcast(&pdc->startCnd);
                                os_mutexUnlock(&pdc->mtx);
                            }

                            /* Perform the copy */
                            ccpp_DataReaderParallelCopy<DataSeq>(pdc);

                            if(os_mutexLock(&pdc->mtx) == os_resultSuccess){
                                pdc->readyCnt++;
                                if(pdc->readyCnt > pdc->nrofWorkers){
                                    /* I am the last one to finish, notify workers */
                                    ccpp_DataReaderParallelCopyReadyBroadcast<true>(pdc);
                                } else {
                                    const os_boolean parity = pdc->parity;
                                    while(parity == pdc->parity){
                                        os_condWait(&pdc->readyCnd, &pdc->mtx);
                                    }
                                }
                                os_mutexUnlock(&pdc->mtx);
                            }
                            /* Parallel copy succeeded; disable singeThreadedCopy fallback */
                            singleThreadedCopy = OS_FALSE;
                        }
                        os_mutexUnlock(&pdc->superMtx);
                    }
                }

                if(singleThreadedCopy) {
                    for ( i = 0; i < len; i++ ) {
                        info->copy_out ( samples->_buffer[i].data, &(*data_seq)[i] );
                        ccpp_SampleInfo_copyOut( samples->_buffer[i].info, (*info_seq)[i] );
                    }
                }
            }
        }

        info->num_samples = len;
    }

}

#endif /* CCPP_DATAREADER */
