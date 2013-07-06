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
