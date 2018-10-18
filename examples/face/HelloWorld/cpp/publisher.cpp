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
#include "Vortex_FACE.hpp"
#include "Vortex/FACE/ReportSupport.hpp"
#include "Vortex/FACE/Config.hpp"

#ifndef _WIN32
#include <unistd.h>
static void Sleep(unsigned int milliseconds)
{
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;
    nanosleep(&ts, NULL);
}
#else
#include <Windows.h>
#endif

#include "HelloWorldData_FACE.hpp"

int main(int argc, char *argv[])
{
    int retval = 1;
    FACE::RETURN_CODE_TYPE return_code;

    (void)argc;
    (void)argv;

    FACE::CONFIGURATION_RESOURCE configuration = "dds_face_config.xml";
    FACE::TS::Initialize(configuration, return_code);
    if (return_code != FACE::NO_ERROR) {
        std::cout << "Initialize error: " << Vortex::FACE::returnCodeToString(return_code) << std::endl;
    } else {
        FACE::CONNECTION_NAME_TYPE connection_name = "HelloWorldPub";
        FACE::MESSAGING_PATTERN_TYPE pattern = FACE::PUB_SUB;
        FACE::CONNECTION_ID_TYPE connection_id;
        FACE::CONNECTION_DIRECTION_TYPE connection_direction = FACE::SOURCE;
        FACE::TIMEOUT_TYPE timeout = 0;
        FACE::MESSAGE_SIZE_TYPE max_message_size = 0;

        FACE::TS::Create_Connection(connection_name, pattern, connection_id, connection_direction, max_message_size, timeout, return_code);
        if (return_code != FACE::NO_ERROR) {
            std::cout << "Create_Connection error: " << Vortex::FACE::returnCodeToString(return_code) << std::endl;
        } else {
            HelloWorldData::Msg msg(0, "Hello World");

            Sleep(2000);
            for (; msg.userID() < 5 && return_code == FACE::NO_ERROR ; msg.userID()++) {
                std::cout << " ________________________________________________________________" << std::endl;
                std::cout << "|" << std::endl;
                std::cout << "| Publisher message : " << msg.userID() << ", " << msg.message() << std::endl;
                std::cout << "|________________________________________________________________" << std::endl << std::endl;

                FACE::TRANSACTION_ID_TYPE transaction_id = 0;
                FACE::MESSAGE_SIZE_TYPE message_size = 0;
                FACE::MESSAGE_TYPE_GUID message_type_id = 0;
                FACE::TS::Send_Message(connection_id, timeout, transaction_id, msg, message_type_id, message_size, return_code);
                if (return_code == FACE::NO_ERROR) {
                    retval = 0; /* success */
                } else {
                    std::cout << "Send_Message error: " << Vortex::FACE::returnCodeToString(return_code) << std::endl;
                }
                Sleep(100);
            }

            /* Wait to ensure data is received before we delete writer */
            Sleep(1000);

            FACE::TS::Destroy_Connection(connection_id, return_code);
            if (return_code != FACE::NO_ERROR) {
                std::cout << "Destroy_Connection error: " << Vortex::FACE::returnCodeToString(return_code) << std::endl;
            }
        }
    }

    return retval;
}
