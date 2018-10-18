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

package org.opensplice.config;

import java.io.File;

import org.opensplice.config.data.DataConfiguration;

public class ValidateDataConfiguration {

	 public static void main(String[] args) {
		 String uri = null;
		 if (args.length > 0
				 && args[0] != null
				 && args[0].toLowerCase().startsWith("-uri=")) {
             uri = args[0].substring(5); /* '-uri=' is 5 characters */
         }
		 else{
			 System.out.println("Invalid parameters, expects format -uri=abc");
			 System.exit(1);
		 }

		 ValidateDataConfiguration vdc = new ValidateDataConfiguration();
		 boolean isValid = vdc.validateFromUri(uri);

		 System.out.println("isValidConfig: "+ isValid);

		 if(isValid){
			 System.exit(0);
		 }
		 else{
			 System.exit(-1);
		 }

	 }

	 public ValidateDataConfiguration() {
	        super();

	 }

	 private boolean validateFromUri(String uri) {
		 boolean isValid = true;

		 try{
			 File file = null;

			 if (uri.startsWith("file://")) {
		           /* strip off file:// */
				 uri = uri.substring(7);
		     }
		     file = new File(uri);
		     final File f = file;
		     new DataConfiguration(f, false);

		 }catch(Exception e){
			 isValid = false;
		 }

		 return isValid;
	 }

}
