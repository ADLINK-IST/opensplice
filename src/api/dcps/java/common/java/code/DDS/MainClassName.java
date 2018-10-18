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

package DDS;

import java.util.Map.Entry;
import java.lang.management.ManagementFactory;

public final class MainClassName
{
  private String classname = "java " + getProcessId();

  public MainClassName() {
      for (Entry<Thread, StackTraceElement[]> entry : Thread.getAllStackTraces().entrySet()) {
          Thread thread = entry.getKey();
          if (thread.getThreadGroup() != null && thread.getThreadGroup().getName().equals("main")) {
              for (StackTraceElement stackTraceElement : entry.getValue()) {
                  /* check if the found class has a main method and if it matches the correct signature */
                  if (stackTraceElement.getMethodName().equals("main")) {
                      try {
                          Class<?> c = Class.forName(stackTraceElement.getClassName());
                          @SuppressWarnings("rawtypes")
                          Class[] argTypes = new Class[] { String[].class };
                          /*check for valid signature of the main method */
                          c.getDeclaredMethod("main", argTypes);
                          classname = stackTraceElement.getClassName() + " " + getProcessId();
                          break;
                      } catch (NoSuchMethodException e) {

                      } catch (ClassNotFoundException e) {

                      }
                  }
              }
          }
      }
  }

  public String getMainClassName() {
      return classname;
  }

  public static String getProcessId() {
      String pid = "";

      /* result looks like '<pid>@<hostname>' */
      final String jvmName = ManagementFactory.getRuntimeMXBean().getName();
      final int index = jvmName.indexOf('@');

      if (index < 1) {
          /* no pid found */
      } else {
          pid = "<" + jvmName.substring(0, index) + ">";
      }

      return pid;
  }
}
