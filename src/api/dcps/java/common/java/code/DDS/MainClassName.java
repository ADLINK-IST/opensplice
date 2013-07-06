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

public final class MainClassName
{
  private String classname = null;
 
  public MainClassName() {
      Map<Thread,StackTraceElement[]> map = Thread.getAllStackTraces();
      for (Map.Entry entry : map.entrySet()) {
          Thread thread = (Thread)entry.getKey();
          if ("main".equals(thread.getName()) && "main".equals(thread.getThreadGroup().getName())) {
              StackTraceElement[] trace = (StackTraceElement[])entry.getValue();
              classname = trace[trace.length - 1].getClassName();
          }
      }
  }
  
  public String getMainClassName() {
      return classname;
  }
}
