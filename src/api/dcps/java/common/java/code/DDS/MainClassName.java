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

package DDS;

import java.util.*;
import java.lang.management.ManagementFactory;

public final class MainClassName
{
  private String classname = null;

  public MainClassName() {
      Map<Thread,StackTraceElement[]> map = Thread.getAllStackTraces();

      for (Map.Entry entry : map.entrySet()) {
          Thread thread = (Thread)entry.getKey();

          if(thread != null){
              if ("main".equals(thread.getName()) && "main".equals(thread.getThreadGroup().getName())) {
                  StackTraceElement[] trace = (StackTraceElement[])entry.getValue();

                  if(trace != null){
                      if(trace.length >= 1){
                          classname = trace[trace.length - 1].getClassName() + " " + getProcessId();
                      }
                  }
              }
          }
      }
  }

  public String getMainClassName() {
      return classname;
  }

  private static String getProcessId() {
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
