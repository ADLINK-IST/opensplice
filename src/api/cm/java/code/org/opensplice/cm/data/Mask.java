/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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
 */
package org.opensplice.cm.data;

public class Mask {
    private int value;

    private static final int _NONE = 0; /* 0 */
    private static final int _READ = (0x0001 << 0); /* 1 */
    private static final int _NOT_READ = (0x0001 << 1); /* 2 */
    private static final int _NEW = (0x0001 << 2); /* 4 */
    private static final int _NOT_NEW = (0x0001 << 3); /* 8 */
    private static final int _ALIVE = (0x0001 << 4); /* 16 */
    private static final int _DISPOSED = (0x0001 << 5); /* 32 */
    private static final int _NO_WRITERS = (0x0001 << 6); /* 64 */

    private static final int _ANY_SAMPLE = (_READ | _NOT_READ);
    private static final int _ANY_VIEW = (_NEW | _NOT_NEW);
    private static final int _ANY_INSTANCE = (_DISPOSED | _NO_WRITERS | _ALIVE);
    private static final int _ANY = (_ANY_SAMPLE | _ANY_VIEW | _ANY_INSTANCE);

    public static final Mask NONE = new Mask(_NONE);
    public static final Mask READ = new Mask(_READ);
    public static final Mask NOT_READ = new Mask(_NOT_READ);
    public static final Mask NEW = new Mask(_NEW);
    public static final Mask NOT_NEW = new Mask(_NOT_NEW);
    public static final Mask ALIVE = new Mask(_ALIVE);
    public static final Mask DISPOSED = new Mask(_DISPOSED);
    public static final Mask NO_WRITERS = new Mask(_NO_WRITERS);
    public static final Mask ANY_SAMPLE = new Mask(_ANY_SAMPLE);
    public static final Mask ANY_VIEW = new Mask(_ANY_VIEW);
    public static final Mask ANY_INSTANCE = new Mask(_ANY_INSTANCE);
    public static final Mask ANY = new Mask(_ANY);

    private Mask(int mask) {
        this.value = mask;
    }

    public Mask withMask(Mask mask) {
        return new Mask(this.value | mask.value);
    }

    @Override
    public String toString() {
        StringBuffer buf = new StringBuffer();

        if (value == _NONE) {
            buf.append("NONE");
        } else if (value == _ANY) {
            buf.append("ANY, ANY, ANY");
        } else {
            if ((value & _ANY_VIEW) == _ANY_VIEW) {
                buf.append("ANY, ");
            } else if ((value & _NEW) == _NEW) {
                buf.append("NEW, ");
            } else {
                buf.append("NOT_NEW, ");
            }
            if ((value & _ANY_SAMPLE) == _ANY_SAMPLE) {
                buf.append("ANY, ");
            } else if ((value & _READ) == _READ) {
                buf.append("READ, ");
            } else {
                buf.append("NOT_READ, ");
            }
            if ((value & _ANY_INSTANCE) == _ANY_INSTANCE) {
                buf.append("ANY, ");
            } else {
                if ((value & _ALIVE) == _ALIVE) {
                    buf.append("ALIVE, ");
                }
                if ((value & _DISPOSED) == _DISPOSED) {
                    buf.append("DISPOSED, ");
                }
                if ((value & _NO_WRITERS) == _NO_WRITERS) {
                    buf.append("NO_WRITERS, ");
                }
            }
            buf.delete(buf.length() - 2, buf.length() - 1);
        }
        return buf.toString();
    }

    /**
     * Provides access to mask.
     * 
     * @return Returns the mask.
     */
    public int getValue() {
        return value;
    }

    public static Mask fromValue(int mask) {
        return new Mask(mask);
    }
}
