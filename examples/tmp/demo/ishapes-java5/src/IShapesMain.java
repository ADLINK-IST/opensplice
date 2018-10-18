/**
 *                             Vortex Cafe
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
 */


import java.io.BufferedReader;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.Arrays;
import java.util.List;
import javafx.application.Application;


public class IShapesMain
{

   private static boolean NO_HMI = false;
   private static boolean NO_PRINT = false;
   private static String SCRIPT_CONTENT = null;

   private static void printUsage()
   {
      System.out
            .println("Usage: java -jar <this_jar> [--noHMI] [--noPrint] [--script <script_file>]");
      System.out.println("   [--noHMI]                : run without HMI (ASCII mode)");
      System.out
            .println("   [--noPrint]              : used with --noHMI, no published/subscribed shapes are printed");
      System.out.println("   [--script <script_file>] : use a JavaScript file" +
            " to create of publications and subcriptions at start-up");
      System.out.println("");
      System.out.println("You can also define the following System Properties (with -D):");
      System.out.println("  dds.domainId     : to change the DDS domain ID (0 by default)");
      System.out.println("  dds.partition    : to use a partition (no partition by default)");
      System.out.println("  dds.registerType : to register the topic type with another name " +
            "(\"ShapeType\" by default)");
      System.out.println("  refreshTimeout   : to change the refresh rate " +
            "(0ms by default - i.e. maximum framerate)");
      System.out.println("");
      System.out.println("Example of usage:");
      System.out
            .println("  java -Ddds.domainId=1 -Ddds.partition=A -jar <this_jar> --script pubCirclesSubSquares.js");
   }

   private static String readFile(String file) throws IOException
   {
      BufferedReader reader = new BufferedReader(new InputStreamReader(new FileInputStream(file),
            "UTF-8"));
      try
      {
         StringBuffer result = new StringBuffer();
         String s = null;
         while ( (s = reader.readLine()) != null)
         {
            result.append(s);
            result.append('\n');
         }
         return result.toString();
      }
      finally
      {
         reader.close();
      }
   }

   private static void parseArgs(List<String> argsList)
   {
      // parse arguments
      if (argsList.contains("--help"))
      {
         printUsage();
         System.exit(0);
      }

      if (argsList.contains("--noHMI"))
      {
         NO_HMI = true;
      }

      if (argsList.contains("--noPrint"))
      {
         NO_PRINT = true;
      }
      int scriptOptionIdx = argsList.indexOf("--script");
      if (scriptOptionIdx >= 0)
      {
         // check is there is an arg after "--script"
         if (argsList.size() >= scriptOptionIdx + 2)
         {
            String scriptFile = argsList.get(scriptOptionIdx + 1);
            try
            {
               SCRIPT_CONTENT = readFile(scriptFile);
            }
            catch (IOException e)
            {
               System.err.println("ERROR reading " + scriptFile + ":");
               e.printStackTrace();
               System.exit(1);
            }
         }
         else
         {
            System.err.println("ERROR: no filename specified for --script option");
            printUsage();
            System.exit(1);
         }
      }
   }

   public static String getScript()
   {
      return SCRIPT_CONTENT;
   }

   public static void main(String[] args) throws Exception
   {

      parseArgs(Arrays.asList(args));
      if (NO_HMI)
      {
         // start command-line controller
         ShapesController controller = new CmdlineController(NO_PRINT);
         ((CmdlineController) controller).initialize();

         // is a script was provided evaluate it
         String script = IShapesMain.getScript();
         if (script != null)
         {
            controller.evalScript(script);
         }

      }
      else
      {
         Application.launch(IShapesApplication.class, args);
      }

   }


}
