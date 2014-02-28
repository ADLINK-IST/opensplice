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
#include "os.h"
#include "cfg_parser.h"
#include "cf_config.h"

#define URI_FILESCHEMA   "file://"
#define DEFAULT_CFILE_NAME "ospl_config.c"
#define MAX_EVN_ARRAY 256

typedef struct config_t {
   char *progname;
   char *ifuri;
   char *ofname;
   char **env;
   int enableDynamicLoad;
   int excludeXML;
} config_t;

/* Service description read from XML */
struct serviceInfo {
   const char *name;
   const char *command;
};

/* Application description read from XML */
struct applicationInfo {
   const char *name;
   const char *libname;
   const char *command;
};

/* CPU number for CPU Affinity */
struct cpuAffinity {
   int cpuNumber;
};


static void
print_usage(
   char *name)
{
   fprintf(stderr,
           "\nUsage:\n"
           "      %s -h\n"
           "      %s [-d [-x]] [-u <URI>] [-e <env=var> ]... [-o <file>]\n"
           "\n"
#ifdef TARGET_VXWORKS_KM
           "      -d        Enable dynamic loading\n"
           "      -x        Exclude xml\n"
#endif
           "      -h,-?     Show this help\n", name, name);
}

static c_iter
getKnownServices(cf_element root)
{
   c_iter services;
   cf_element domain;
   cf_element el;
   cf_element cmd;
   cf_data data;
   c_iter children;
   cf_attribute attr;
   struct serviceInfo *si;

   assert(root);

   services = NULL;
   if (root
       && (domain = (cf_element)cf_elementChild(root, "Domain")) )
   {
      children = cf_elementGetChilds(domain);
      while ( (el = c_iterTakeFirst(children)) != NULL)
      {
         if ( strcmp(cf_nodeGetName(cf_node(el)), "Service") == 0
              && (cmd = (cf_element)cf_elementChild(el, "Command"))
              && (data = cf_data(cf_elementChild(cmd, "#text")))
              && (si = os_malloc(sizeof(struct serviceInfo))))
         {

            si->command = cf_dataValue(data).is.String;

            attr = cf_elementAttribute(el, "name");
            if (attr)
            {
               si->name = cf_attributeValue(attr).is.String;
            }
            else
            {
               si->name = si->command;
            }

            if (!si->name || !si->command)
            {
               /* detected an invalid service configuration, report and skip */
               fprintf(stderr,
                       "Warning: detected invalid 'Service'"
                       " element (name = %s; command = %s) -> skip\n",
                       (si->name?si->name:"<null>"),
                       (si->command?si->command:"<null>"));
               os_free(si);
               si = NULL;
            }
            else
            {
               services = c_iterInsert(services, si);
            }
         }
      }
      c_iterFree(children);
   }
   return services;
}
static c_iter
getCPUAffinities(cf_element root)
{
   c_iter affinities = NULL;
   cf_element domain;
   cf_element cpuAffinity;
   cf_data data;

   assert(root);

   if (root
       && (domain = (cf_element)cf_elementChild(root, "Domain")) )
   {
      cpuAffinity = cf_element(cf_elementChild(domain, CFG_CPUAFFINITY));
      if (cpuAffinity != NULL)
      {
         data = cf_data(cf_elementChild(cpuAffinity, "#text"));
         if (data != NULL)
         {
            char *saveptr;
            char *next;
            char *value;
            value = cf_dataValue(data).is.String;
            next = os_strtok_r(value, ",", &saveptr);
            while ( next != NULL )
            {
               signed int corenum;
               struct cpuAffinity *newCpuAffinity;
               sscanf( next, "%d", &corenum);
               newCpuAffinity = (struct cpuAffinity *)malloc (sizeof(struct cpuAffinity));
               newCpuAffinity->cpuNumber = corenum;
               affinities = c_iterInsert(affinities, newCpuAffinity);
               next = os_strtok_r(NULL, ",", &saveptr);
            }
         }
      }
   }
   return affinities;
}

static c_iter
getKnownApplications(cf_element root)
{
   c_iter apps;
   cf_element domain;
   cf_element el;
   cf_element cmd;
   cf_element lib;
   cf_data data;
   c_iter children;
   cf_attribute attr;
   struct applicationInfo *ai;

   assert(root);

   apps = NULL;
   if (root
       && (domain = (cf_element)cf_elementChild(root, "Domain")) )
   {
      children = cf_elementGetChilds(domain);
      while ( (el = c_iterTakeFirst(children)) != NULL)
      {
         if ( strcmp(cf_nodeGetName(cf_node(el)), "Application") == 0
              && (cmd = (cf_element)cf_elementChild(el, "Command"))
              && (data = cf_data(cf_elementChild(cmd, "#text")))
              && (ai = os_malloc(sizeof(struct applicationInfo))))
         {

            ai->command = cf_dataValue(data).is.String;

            attr = cf_elementAttribute(el, "name");
            if (attr)
            {
               ai->name = cf_attributeValue(attr).is.String;
            }
            else
            {
               ai->name = ai->command;
            }

            if ( (lib = (cf_element)cf_elementChild(el, "Library"))
                 && (data = cf_data(cf_elementChild(lib, "#text"))) )
            {
               ai->libname = cf_dataValue(data).is.String;
            }
            else
            {
               ai->libname = ai->command;
            }

            if (!ai->name || !ai->command)
            {
               /* detected an invalid application configuration, report and skip */
               fprintf(stderr,
                       "Warning: detected invalid 'Application'"
                       " element (name = %s; command = %s) -> skip\n",
                       (ai->name?ai->name:"<null>"),
                       (ai->command?ai->command:"<null>"));
               os_free(ai);
               ai = NULL;
            }
            else
            {
               apps = c_iterInsert(apps, ai);
            }
         }
      }
      c_iterFree(children);
   }
   return apps;
}

void genServiceEntryPointDecl( void *o, c_iterActionArg arg)
{
#ifdef TARGET_VXWORKS_KM
   fprintf( (FILE*)arg,
            "int ospl_%s(char *args);\n",
            ((struct serviceInfo*)o)->command );
#else
   fprintf( (FILE*)arg,
            "OS_API_IMPORT OPENSPLICE_ENTRYPOINT_DECL(ospl_%s);\n",
            ((struct serviceInfo*)o)->command );
#endif
}

void genAppEntryPointDecl( void *o, c_iterActionArg arg)
{
#ifdef TARGET_VXWORKS_KM
   fprintf( (FILE*)arg,
            "int ospl_%s(char *args);\n",
            ((struct applicationInfo*)o)->command );
#else
   fprintf( (FILE*)arg,
            "OS_API_IMPORT OPENSPLICE_ENTRYPOINT_DECL(ospl_%s);\n",
            ((struct applicationInfo*)o)->command );
#endif
}

void genCPUAffinity_SET( void *o, c_iterActionArg arg)
{
   fprintf( (FILE*)arg,
            "   CPUSET_SET(affinity, %d);\n",
            ((struct cpuAffinity*)o)->cpuNumber );
}

static int
generate_cf_file(
   config_t *cfg,
   const cf_element root)
{
   int i = 0;
   c_iter services = NULL;
   c_iter apps = NULL;
   struct serviceInfo *si;
   struct applicationInfo *ai;
   FILE *file;
   FILE *xml;
   c_iter affinityIter;
   int success = 1;

   file = fopen( cfg->ofname, "w+" );
   if ( file )
   {
#ifdef TARGET_VXWORKS_KM
      fprintf( file,
               "#include \"os_EntryPoints.h\"\n"
               "#include \"include/os_dynamicLib_plugin.h\"\n"
               "#include \"os.h\"\n"
               "#include \"taskLib.h\"\n"
               "\n"
               "int ospl_spliced(char *);\n" );
#else
      fprintf( file,
               "#include \"os_EntryPoints.h\"\n"
               "#include \"os.h\"\n"
               "\n"
               "OS_API_IMPORT OPENSPLICE_ENTRYPOINT_DECL(ospl_spliced);\n" );
#endif
      if ( root != NULL )
      {
         services = getKnownServices(root);
         apps = getKnownApplications(root);
         c_iterWalk(services, genServiceEntryPointDecl, file);
         c_iterWalk(apps, genAppEntryPointDecl, file);
      }

      if ( cfg->enableDynamicLoad )
      {
         fprintf( file,
                  "\n"
                  "struct os_dynamicLoad_plugin *os_dynamicLibPlugin = &os_dynamicLibPluginImpl;\n");
      }
      else
      {
         fprintf( file,
                  "\n"
                  "struct os_dynamicLoad_plugin *os_dynamicLibPlugin = NULL;\n");
      }

      fprintf( file,
               "\n"
               "struct os_librarySymbols os_staticLibraries[] =\n"
               "{\n");
      if ( !cfg->excludeXML )
      {
         fprintf( file,
                  "   {\n"
                  "      \"spliced\",\n"
                  "      (os_entryPoint[])\n"
                  "      {\n"
                  "         { \"ospl_spliced\", ospl_spliced },\n"
                  "         { NULL, NULL }\n"
                  "      }\n"
                  "   },\n");
      }
      if ( root != NULL )
      {
         /* Generate address space entry for each service */
         while ( (si = (struct serviceInfo*)c_iterTakeFirst(services)) != 0
                 && success)
         {
#ifdef TARGET_VXWORKS_KM
            fprintf( file,
                     "   {\n"
                     "      \"%s\",\n"
                     "      (os_entryPoint[])\n"
                     "      {\n"
                     "         { \"%s\", ospl_%s },\n"
                     "         { NULL, NULL }\n"
                     "      }\n"
                     "   },\n",
                     si->command, si->command, si->command );
#else
            fprintf( file,
                     "   {\n"
                     "      \"%s\",\n"
                     "      (os_entryPoint[])\n"
                     "      {\n"
                     "         { \"ospl_%s\", ospl_%s },\n"
                     "         { NULL, NULL }\n"
                     "      }\n"
                     "   },\n",
                     si->command, si->command, si->command );
#endif
            os_free(si);
         }
         c_iterFree(services);

         while ( (ai = (struct applicationInfo*)c_iterTakeFirst(apps)) != 0
                 && success)
         {
            fprintf( file,
                     "   { \n"
                     "      \"%s\",\n"
                     "      (os_entryPoint[])\n"
                     "      {\n"
                     "         { \"%s\", %s },\n"
                     "         { \"\", NULL }\n"
                     "      }\n"
                     "   },\n",
                     ai->libname, ai->command, ai->command );
            os_free(ai);
         }
         c_iterFree(apps);
      }
      fprintf( file,
               "   {\n"
               "      NULL,\n"
               "      (os_entryPoint[])\n"
               "      {\n"
               "         { NULL, NULL }\n"
               "      }\n"
               "   }\n"
               "};\n" );

      /* Generate environment array */
      fprintf( file,
               "\n"
               "char *os_environ[] =\n"
               "{\n" );
      for ( i=0; cfg->env[i]; i++ )
      {
          fprintf( file, "   \"%s\",\n", cfg->env[i]);
      }
      fprintf( file,
               "   NULL\n"
               "};  \n" );


#ifdef TARGET_VXWORKS_KM
      fprintf( file,
               "\n"
               "void setCPUAffinityHook()\n"
               "{\n");
      if ( root != NULL )
      {
         affinityIter = getCPUAffinities(root);
         if ( affinityIter != NULL )
         {
            fprintf( file,
                     "   cpuset_t affinity;\n"
                     "   CPUSET_ZERO(affinity);\n");
            c_iterWalk(affinityIter, genCPUAffinity_SET, file);
            fprintf( file,
                     "   taskCpuAffinitySet (0, affinity);\n");
         }
      }
      fprintf( file,
               "}\n");
#endif

      if ( !cfg->excludeXML )
      {
         if ( !strncmp( cfg->ifuri, URI_FILESCHEMA, sizeof(URI_FILESCHEMA)-1 ))
         {
            size_t totalchars=0;
            char buf[16];
            size_t index;
            size_t readchars;
            /* Copy the xml file in as a hex array */
            fprintf( file,
                     "\n"
                     "char ospl_xml_data[] =\n"
                     "{\n");
            xml = fopen( &cfg->ifuri[sizeof(URI_FILESCHEMA)-1], "r" );
            if ( xml )
            {
               while ( (readchars = fread( buf, 1, sizeof(buf), xml )) != 0 )
               {
                  totalchars+=readchars;
                  fprintf(file, "   " );
                  for ( index=0; index<readchars; index++ )
                  {
                     fprintf(file, "0x%02x, ", buf[index]);
                  }
                  fprintf(file, "\n" );
               }
               fprintf(file,
                       "   0x0\n"
                       "};\n"
                       "size_t ospl_xml_data_size = %d;\n",
                       (int)totalchars);
               fclose(xml);
            }
            else
            {
               fprintf(stderr, "ERROR: could not open \"%s\"", &cfg->ifuri[sizeof(URI_FILESCHEMA)-1] );
               print_usage(cfg->progname);
            }
         }
         else
         {
            fprintf(stderr, "ERROR: only file URIs are supported\n");
            print_usage(cfg->progname);
         }
      }
      else
      {
         fprintf( file,
                  "\n"
                  "char ospl_xml_data[] = { '\\0' };\n"
                  "size_t ospl_xml_data_size = 0;\n");
      }
      fclose(file);
   }

   if ( !success )
   {
      /* Erase part generated file */
      unlink( cfg->ofname );
   }

   return(success);
}


int
main(
   int argc,
   char *argv[])
{
   int opt;
   int ret;
   int i = 0;
   char *uri = NULL;
   char *env[MAX_EVN_ARRAY];
   config_t cfg = { NULL, NULL, NULL, NULL, 0, 0};
   cf_element root = NULL;
   cfgprs_status parser_status = CFGPRS_OK;

   while ((opt = getopt(argc, argv, "hdxo:u:e:")) != -1)
   {
      switch (opt)
      {
         case 'h':
         {
            print_usage(argv[0]);
            return(0);
         }
         case 'd':
         {
            cfg.enableDynamicLoad = 1;
            break;
         }
         case 'x':
         {
            cfg.excludeXML = 1;
            break;
         }
         case 'o':
         {
            if (cfg.ofname)
            {
               print_usage(argv[0]);
               return -1;
            }
            cfg.ofname = optarg;
            break;
         }
         case 'u':
         {
            if (uri)
            {
               print_usage(argv[0]);
               return -1;
            }
            uri = optarg;
            break;
         }
         case 'e':
         {
            if ( i < MAX_EVN_ARRAY )
            {
               env[i++] = optarg;
            }
            break;
         }
      }
   }
   if ( cfg.excludeXML && !cfg.enableDynamicLoad )
   {
      fprintf(stderr, "Error: Use of the option -x requires the -d option.\n");
      print_usage(argv[0]);
      return -1;
   }
   env[i] = NULL;
   if ( i >= MAX_EVN_ARRAY )
   {
      print_usage(argv[0]);
      fprintf(stderr,"Max env array reached : %d\n", MAX_EVN_ARRAY - 1);
   }

   cfg.env = env;
   cfg.progname = argv[0];
   /* if uri is not set on the command line use environment variable to
    * discover configuration file
    */
   if (!uri)
   {
      uri = os_getenv("OSPL_URI");
   }
   cfg.ifuri = uri;

   if (!cfg.ofname)
   {
      cfg.ofname = DEFAULT_CFILE_NAME;
   }

   if ( cfg.excludeXML )
   {
      cfg.ifuri = NULL;
      ret = ! generate_cf_file( &cfg, root );
   }
   else
   {

     /* At this point uri must be set and valid otherwise the xml parser
      * will return CFGPRS_NO_INPUT.
      */
     parser_status = cfg_parse_ospl((const char *)uri, &root);

     switch (parser_status)
     {
        case CFGPRS_OK: /* do nothing as we are ok for now */
        {
           break;
        }
        case CFGPRS_NO_INPUT:
        {
           fprintf(stderr, "No valid URI specified:\n");
           fprintf(stderr, "1. on command line\n");
           fprintf(stderr, "2. OSPL_URI environment variable\n");
           print_usage(argv[0]);
           return -1;
        }
        case CFGPRS_ERROR:
        {
           fprintf(stderr, "Failure reading %s, please check syntax of configuration file.\n", uri);
           fprintf(stderr, "\n");
           return -1;
        }
     }
     ret = ! generate_cf_file( &cfg, root );
     cf_nodeFree(cf_node(root));

   }
   return(ret);
}
