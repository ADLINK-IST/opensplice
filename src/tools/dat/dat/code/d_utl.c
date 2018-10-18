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
	int argc,
	char **argv,
	char *arg)
{
	for (--argc; 0 < argc; argc--)
		if (strncmp(argv[argc], arg, strlen(arg)) == 0)
			break;
	return (argc);
	
}


/*
 * returns a pointer to the data of switch arg, NULL if empty or not found
 */
char *
d_argData(
	int argc,
	char **argv,
	char *arg)
{
	char *result = NULL;
	int argPos = d_argExists(argc, argv, arg);
	if (argPos) {
		result = argv[argPos];
		result += 2;
	}
	return result;
}




// EOF  d_utl.c
