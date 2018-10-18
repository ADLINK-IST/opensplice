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
        FACE::CONNECTION_NAME_TYPE connection_name = "HelloWorldSub";
        FACE::MESSAGING_PATTERN_TYPE pattern = FACE::PUB_SUB;
        FACE::CONNECTION_ID_TYPE connection_id;
        FACE::CONNECTION_DIRECTION_TYPE connection_direction = FACE::DESTINATION;
        FACE::TIMEOUT_TYPE timeout = 0;
        FACE::MESSAGE_SIZE_TYPE max_message_size = 0;

        FACE::TS::Create_Connection(connection_name, pattern, connection_id, connection_direction, max_message_size, timeout, return_code);
        if (return_code != FACE::NO_ERROR) {
            std::cout << "Create_Connection error: " << Vortex::FACE::returnCodeToString(return_code) << std::endl;
        } else {
            HelloWorldData::Msg msg;
            FACE::TRANSACTION_ID_TYPE transaction_id = 0;
            FACE::MESSAGE_SIZE_TYPE message_size = 0;
            FACE::MESSAGE_TYPE_GUID message_type_id = 0;
            timeout = 30000000000LL; /* 30 seconds */

            do {
                FACE::TS::Receive_Message(connection_id, timeout, transaction_id, msg, message_type_id, message_size, return_code);
                if (return_code == FACE::NO_ERROR) {
                    std::cout << " ________________________________________________________________" << std::endl;
                    std::cout << "|" << std::endl;
                    std::cout << "| Subscriber message : " << msg.userID() << ", " << msg.message() << std::endl;
                    std::cout << "|________________________________________________________________" << std::endl << std::endl;
                    retval = 0;
                } else {
                    std::cout << "Receive_Message error: " << Vortex::FACE::returnCodeToString(return_code) << std::endl;
                }
            } while (return_code == FACE::NO_ERROR && msg.userID() < 4);

            FACE::TS::Destroy_Connection(connection_id, return_code);
            if (return_code != FACE::NO_ERROR) {
                std::cout << "Destroy_Connection error: " << Vortex::FACE::returnCodeToString(return_code) << std::endl;
            }
        }
    }

    return retval;
}
