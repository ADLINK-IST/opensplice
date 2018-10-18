/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#include "QueryCondition.h"
#include "StatusUtils.h"
#include "ReadCondition.h"
#include "SequenceUtils.h"
#include "MiscUtils.h"
#include "ReportUtils.h"
#include "FooDataReader_impl.h"
#include "FooDataReaderView_impl.h"
#include "cmn_samplesList.h"
#include "cmn_reader.h"
#include "Constants.h"

DDS::OpenSplice::QueryCondition::QueryCondition() :
    DDS::OpenSplice::ReadCondition(DDS::OpenSplice::QUERYCONDITION)
{
}

DDS::OpenSplice::QueryCondition::~QueryCondition()
{
}

DDS::ReturnCode_t
DDS::OpenSplice::QueryCondition::init   (DDS::OpenSplice::Entity *reader,
    DDS::SampleStateMask sample_states,
    DDS::ViewStateMask view_states,
    DDS::InstanceStateMask instance_states,
    const char *queryExpression,
    const DDS::StringSeq *queryParameters)
{
    return nlReq_init(reader, sample_states, view_states, instance_states,
                      queryExpression, queryParameters);
}

DDS::ReturnCode_t
DDS::OpenSplice::QueryCondition::nlReq_init (DDS::OpenSplice::Entity *reader,
    DDS::SampleStateMask sample_states,
    DDS::ViewStateMask view_states,
    DDS::InstanceStateMask instance_states,
    const char *queryExpression,
    const DDS::StringSeq *queryParameters)
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;
    u_reader uReader = NULL;
    u_sampleMask mask = statesMask(sample_states, view_states, instance_states);

    char **params;

    this->query_expression = DDS::string_dup(queryExpression);
    this->query_parameters = *queryParameters;

    if (reader->rlReq_get_kind() == DATAREADER) {
        uReader = u_reader(dynamic_cast<DataReader*>(reader)->rlReq_get_user_entity ());
    } else if (reader->rlReq_get_kind() == DATAREADERVIEW) {
        uReader = u_reader(dynamic_cast<DataReaderView*>(reader)->rlReq_get_user_entity ());
    } else {
        result = DDS::RETCODE_ALREADY_DELETED;
        CPP_REPORT(result, "Could not create QueryCondition, Entity already deleted.");
    }

    if (uReader) {
        params = DDS::OpenSplice::Utils::stringSeqToStringArray(*queryParameters, FALSE);
        if (params || queryParameters->length() == 0) {
            this->uQuery = u_queryNew(uReader, NULL,
                                      queryExpression, (const char **)params,
                                      queryParameters->length(), mask);
            if (this->uQuery) {
                result = ReadCondition::init(reader, sample_states, view_states, instance_states);
                if ( result != DDS::RETCODE_OK) {
                    wlReq_deinit();
                }
            } else {
                result = DDS::RETCODE_ERROR;
                CPP_REPORT(result, "Could not create QueryCondition.");
            }

            /* Cleanup params. */
            DDS::OpenSplice::Utils::freeStringArray(params, this->query_parameters.length());
        } else {
            result = DDS::RETCODE_BAD_PARAMETER;
            CPP_REPORT(result, "Could not copy query_parameters.");
        }
    }

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::QueryCondition::wlReq_deinit()
{
    DDS::ReturnCode_t result;
    result = DDS::OpenSplice::ReadCondition::wlReq_deinit();
    if (result == DDS::RETCODE_OK) {
        DDS::string_free(this->query_expression);
        this->query_parameters = 0;
    }
    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::QueryCondition::read (
    DDS::OpenSplice::Entity *source,
    void *data_seq,
    DDS::SampleInfoSeq &info_seq,
    const long max_samples,
    void* samplesList)
{
    DDS::ReturnCode_t result;
    u_result uResult;

    CPP_REPORT_STACK();

    result = source->write_lock();
    if (result == DDS::RETCODE_OK) {
        cmn_samplesList_reset((cmn_samplesList)samplesList, realMaxSamples(max_samples, info_seq));
        uResult = u_queryRead(this->uQuery, cmn_reader_action, (cmn_samplesList*)samplesList, OS_DURATION_ZERO);
        if (uResult == U_RESULT_OK) {
            if (source->rlReq_get_kind() == DATAREADER) {
                FooDataReader_impl *fooDataReader = dynamic_cast<FooDataReader_impl*>(source);
                if (fooDataReader) {
                    result = fooDataReader->flush(samplesList, data_seq, info_seq);
                } else {
                    result = DDS::RETCODE_ERROR;
                }
            } else {
                FooDataReaderView_impl *fooDataReaderView = dynamic_cast<FooDataReaderView_impl*>(source);
                assert (source->rlReq_get_kind() == DATAREADERVIEW);
                if (fooDataReaderView) {
                    result = fooDataReaderView->flush(samplesList, data_seq, info_seq);
                } else {
                    result = DDS::RETCODE_ERROR;
                }
            }
        } else {
            result = this->uResultToReturnCode(uResult);
        }
        source->unlock();
    }

    CPP_REPORT_FLUSH(this, (result != DDS::RETCODE_OK) && (result != DDS::RETCODE_NO_DATA));

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::QueryCondition::take (
    DDS::OpenSplice::Entity *source,
    void *data_seq,
    DDS::SampleInfoSeq &info_seq,
    const long max_samples,
    void* samplesList)
{
    DDS::ReturnCode_t result;
    u_result uResult;

    CPP_REPORT_STACK();

    result = source->write_lock();
    if (result == DDS::RETCODE_OK) {
        cmn_samplesList_reset((cmn_samplesList)samplesList, realMaxSamples(max_samples, info_seq));
        uResult = u_queryTake(this->uQuery, cmn_reader_action, (cmn_samplesList*)samplesList, OS_DURATION_ZERO);
        if (uResult == U_RESULT_OK) {
            if (source->rlReq_get_kind() == DATAREADER) {
                FooDataReader_impl *fooDataReader = dynamic_cast<FooDataReader_impl*>(source);
                if (fooDataReader) {
                    result = fooDataReader->flush(samplesList, data_seq, info_seq);
                } else {
                    result = DDS::RETCODE_ERROR;
                }
            } else {
                FooDataReaderView_impl *fooDataReaderView = dynamic_cast<FooDataReaderView_impl*>(source);
                assert (source->rlReq_get_kind() == DATAREADERVIEW);
                if (fooDataReaderView) {
                    result = fooDataReaderView->flush(samplesList, data_seq, info_seq);
                } else {
                    result = DDS::RETCODE_ERROR;
                }
            }
        } else {
            result = this->uResultToReturnCode(uResult);
        }
        source->unlock();
    }

    CPP_REPORT_FLUSH(this, (result != DDS::RETCODE_OK) && (result != DDS::RETCODE_NO_DATA));

    return result;

}

DDS::ReturnCode_t
DDS::OpenSplice::QueryCondition::read_next_instance (
    DDS::OpenSplice::Entity *source,
    void *data_seq,
    DDS::SampleInfoSeq &info_seq,
    const long max_samples,
    DDS::InstanceHandle_t a_handle,
    void* samplesList)
{
    DDS::ReturnCode_t result;
    u_result uResult;
    CPP_REPORT_STACK();

    result = source->write_lock();
    if (result == DDS::RETCODE_OK) {
        cmn_samplesList_reset((cmn_samplesList)samplesList, realMaxSamples(max_samples, info_seq));
        uResult = u_queryReadNextInstance(this->uQuery, a_handle, cmn_reader_action, (cmn_samplesList*)samplesList, OS_DURATION_ZERO);
        if (uResult == U_RESULT_OK) {
            if (source->rlReq_get_kind() == DATAREADER) {
                FooDataReader_impl *fooDataReader = dynamic_cast<FooDataReader_impl*>(source);
                if (fooDataReader) {
                    result = fooDataReader->flush(samplesList, data_seq, info_seq);
                } else {
                    result = DDS::RETCODE_ERROR;
                }
            } else {
                FooDataReaderView_impl *fooDataReaderView = dynamic_cast<FooDataReaderView_impl*>(source);
                assert (source->rlReq_get_kind() == DATAREADERVIEW);
                if (fooDataReaderView) {
                    result = fooDataReaderView->flush(samplesList, data_seq, info_seq);
                } else {
                    result = DDS::RETCODE_ERROR;
                }
            }
        } else if (uResult == U_RESULT_HANDLE_EXPIRED) {
            result = DDS::RETCODE_HANDLE_EXPIRED;
        } else {
            result = this->uResultToReturnCode(uResult);
            CPP_REPORT(result, "Could not read next instance.");
        }
        source->unlock();
    }

    CPP_REPORT_FLUSH(this, (result != DDS::RETCODE_OK) && (result != DDS::RETCODE_NO_DATA) && (result != DDS::RETCODE_HANDLE_EXPIRED));

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::QueryCondition::take_next_instance (
    DDS::OpenSplice::Entity *source,
    void *data_seq,
    DDS::SampleInfoSeq &info_seq,
    const long max_samples,
    DDS::InstanceHandle_t a_handle,
    void* samplesList)
{
    DDS::ReturnCode_t result;
    u_result uResult;

    CPP_REPORT_STACK();

    result = source->write_lock();
    if (result == DDS::RETCODE_OK) {
        cmn_samplesList_reset((cmn_samplesList)samplesList, realMaxSamples(max_samples, info_seq));
        uResult = u_queryTakeNextInstance(this->uQuery, a_handle, cmn_reader_action, (cmn_samplesList*)samplesList, OS_DURATION_ZERO);
        if (uResult == U_RESULT_OK) {
            if (source->rlReq_get_kind() == DATAREADER) {
                FooDataReader_impl *fooDataReader = dynamic_cast<FooDataReader_impl*>(source);
                if (fooDataReader) {
                    result = fooDataReader->flush(samplesList, data_seq, info_seq);
                } else {
                    result = DDS::RETCODE_ERROR;
                }
            } else {
                FooDataReaderView_impl *fooDataReaderView = dynamic_cast<FooDataReaderView_impl*>(source);
                assert (source->rlReq_get_kind() == DATAREADERVIEW);
                if (fooDataReaderView) {
                    result = fooDataReaderView->flush(samplesList, data_seq, info_seq);
                } else {
                    result = DDS::RETCODE_ERROR;
                }
            }
        } else if (uResult == U_RESULT_HANDLE_EXPIRED) {
            result = DDS::RETCODE_HANDLE_EXPIRED;
        } else {
            result = this->uResultToReturnCode(uResult);
            CPP_REPORT(result, "Could not take next instance.");
        }
        source->unlock();
    }

    CPP_REPORT_FLUSH(this, (result != DDS::RETCODE_OK) && (result != DDS::RETCODE_NO_DATA) && (result != DDS::RETCODE_HANDLE_EXPIRED));

    return result;

}

char * DDS::OpenSplice::QueryCondition::get_query_expression ()
{
    DDS::ReturnCode_t result;
    char *expression = NULL;

    CPP_REPORT_STACK();

    result = this->read_lock ();
    if (result == DDS::RETCODE_OK) {
        expression = DDS::string_dup(this->query_expression);
        this->unlock ();
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return expression;
}

DDS::ReturnCode_t DDS::OpenSplice::QueryCondition::get_query_parameters (
    DDS::StringSeq & query_parameters
)
{
    DDS::ReturnCode_t result;

    CPP_REPORT_STACK();

    result = this->read_lock ();
    if (result == DDS::RETCODE_OK) {
        query_parameters = this->query_parameters;
        this->unlock ();
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

DDS::ReturnCode_t DDS::OpenSplice::QueryCondition::set_query_parameters (
    const DDS::StringSeq & queryParameters
)
{
    DDS::ReturnCode_t result;
    char **params;

    CPP_REPORT_STACK();

    result = this->write_lock ();
    if (result == DDS::RETCODE_OK) {
        params = DDS::OpenSplice::Utils::stringSeqToStringArray(queryParameters, FALSE);
        if (params || queryParameters.length()) {
            u_querySet(this->uQuery, (const char **)params, queryParameters.length());
            DDS::OpenSplice::Utils::freeStringArray(params, queryParameters.length());
            this->query_parameters = queryParameters;
        } else {
            result = DDS::RETCODE_BAD_PARAMETER;
            CPP_REPORT(result, "Could not copy query_paramters.");
        }
        this->unlock();
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}
