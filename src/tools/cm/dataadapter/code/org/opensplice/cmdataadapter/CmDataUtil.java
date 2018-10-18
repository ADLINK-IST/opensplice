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
package org.opensplice.cmdataadapter;

import java.math.BigInteger;
import java.nio.ByteBuffer;

import org.opensplice.cm.data.UserData;

public class CmDataUtil {
    public static void setByteSequence (UserData userData, String fieldName, byte[] bytes) {
        String colSuffix = "";
        if (fieldName.endsWith("]")) {
            int dotIndex = fieldName.lastIndexOf('.');
            if (dotIndex == -1) dotIndex = 0;
            int bracketIndex = fieldName.indexOf('[', dotIndex);
            colSuffix = fieldName.substring(bracketIndex);
            fieldName = fieldName.substring(0, bracketIndex);
        }
        StringBuilder fieldNameBuilder = new StringBuilder(fieldName);
        for (int i = 0; i < bytes.length; i++) {
            String byteString;
            if (bytes[i] >= 0) {
                byteString = Short.toString(bytes[i]);
            } else {
                byteString = Short.toString((short) (bytes[i] + 256));
            }
            fieldNameBuilder.setLength(fieldName.length());
            fieldNameBuilder.append("[").append(i).append("]").append(colSuffix);
            userData.setData(fieldNameBuilder.toString(), byteString);
        }
    }

    public static byte[] getByteSequence (UserData userData, String fieldName) {
        String colSuffix = "";
        String colPrefix = fieldName;
        if (fieldName.endsWith("]")) {
            int dotIndex = fieldName.lastIndexOf('.');
            if (dotIndex == -1) dotIndex = 0;
            int bracketIndex = fieldName.indexOf('[', dotIndex);
            colSuffix = fieldName.substring(bracketIndex);
            colPrefix = fieldName.substring(0, bracketIndex);
        }
        StringBuilder fieldNameBuilder = new StringBuilder(colPrefix);
        int dataSize = userData.getCollectionRealSize(fieldName);
        ByteBuffer byteBuffer = ByteBuffer.allocate(dataSize);
        for (int i = 0; i < dataSize; i++) {
            fieldNameBuilder.setLength(colPrefix.length());
            fieldNameBuilder.append("[").append(i).append("]").append(colSuffix);
            String byteString = userData.getFieldValue(fieldNameBuilder.toString());
            byteBuffer.put(Short.valueOf(byteString).byteValue());
        }
        return byteBuffer.array();
    }

    public static String typeHashToHexString(String hash_msb, String hash_lsb) {
        BigInteger ulonglong_msb = new BigInteger(hash_msb);
        BigInteger ulonglong_lsb = new BigInteger(hash_lsb);
        return Long.toHexString(ulonglong_msb.longValue())
                + Long.toHexString(ulonglong_lsb.longValue());
    }
}
