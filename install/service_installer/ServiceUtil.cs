using System;
using System.Runtime.InteropServices;
using System.IO;
using System.Collections;

namespace SvcInstaller
{
    struct serviceInfo
    {
        public string srvPath;
        public string srvDisplayName;
        public string srvName;
    }

    class ServiceUtil
    {
        #region Private Variables
        static int NOT_SET = 0;
        static int INSTALL = 1;
        static int REMOVE = 2;
        #endregion Private Variables

        #region DLLImport
        [DllImport("advapi32.dll")]
        public static extern IntPtr OpenSCManager(string lpMachineName, string lpSCDB, int scParameter);
        [DllImport("Advapi32.dll")]
        public static extern IntPtr CreateService(IntPtr SC_HANDLE, string lpSvcName, string lpDisplayName,
                          int dwDesiredAccess, int dwServiceType, int dwStartType,
                          int dwErrorControl, string lpPathName,
                          string lpLoadOrderGroup, int lpdwTagId, string lpDependencies,
                          string lpServiceStartName, string lpPassword);
        [DllImport("advapi32.dll")]
        public static extern void CloseServiceHandle(IntPtr SCHANDLE);
        [DllImport("advapi32.dll")]
        public static extern int StartService(IntPtr SVHANDLE, int dwNumServiceArgs, string lpServiceArgVectors);
        [DllImport("advapi32.dll", SetLastError = true)]
        public static extern IntPtr OpenService(IntPtr SCHANDLE, string lpSvcName, int dwNumServiceArgs);
        [DllImport("advapi32.dll")]
        public static extern int DeleteService(IntPtr SVHANDLE);
        [DllImport("kernel32.dll")]
        public static extern int GetLastError();
        #endregion DLLImport

        #region Main method
        [STAThread]
        static void Main(string[] args)
        {
            string serviceFileStr = null;
            ArrayList services = new ArrayList();
            int type = NOT_SET;

            if (args.Length != 2)
            {
                usage("Incorrect number of Arguments");
                return;
            }

            for (int i = 0; i < args.Length; i++)
            {
                string arg = args[i];

                if (arg == "-install")
                {
                    if (type == NOT_SET)
                    {
                        type = INSTALL;
                    }
                    else
                    {
                        usage("Multiple install/remove commands");
                        return;
                    }
                }
                else if (arg == "-remove")
                {
                    if (type == NOT_SET)
                    {
                        type = REMOVE;
                    }
                    else
                    {
                        usage("Multiple install/remove commands");
                        return;
                    }
                }
                else
                {
                    serviceFileStr = args[i];
                }
            }

            if (!File.Exists(serviceFileStr))
            {
                usage("Input File does not exist");
                return;
            }

            FileStream serviceFile = File.OpenRead(serviceFileStr);
            StreamReader sr = new StreamReader(serviceFile);
            string input = null;
            serviceInfo si = new serviceInfo();
  
            while ((input = sr.ReadLine()) != null)
            {
                input = input.Trim();
                if (!input.StartsWith("#"))
                {
                    if (input.StartsWith("ServicePath:"))
                    {
                        si.srvPath = input.Substring(input.IndexOf(':') + 1);;
                    }
                    else if (input.StartsWith("ServiceDisplayName:"))
                    {
                        si.srvDisplayName = input.Substring(input.IndexOf(':') + 1);
                    }
                    else if (input.StartsWith("ServiceName:"))
                    {
                        si.srvName = input.Substring(input.IndexOf(':') + 1);
                    }
                    else if (input != "")
                    {
                        usage("Invalid file format '" + input + "'");
                        return;
                    }
                }
                else if (input == "#ServiceInfo-START")
                {
                    si = new serviceInfo();
                }
                else if (input == "#ServiceInfo-END")
                {
                    services.Add(si);
                }
            }

            sr.Close();
            serviceFile.Close();

            ServiceUtil c = new ServiceUtil();

            for (int i = 0; i < services.Count; i++)
            {
                si = (serviceInfo)services[i];
                if (type == INSTALL)
                {
                    c.InstallService(si.srvPath, si.srvName, si.srvDisplayName);
                }
                else if (type == REMOVE)
                {
                    c.UnInstallService(si.srvName);
                }
            }
        }
        #endregion Main method

        static void usage(string err)
        {
            Console.WriteLine(err);
            Console.WriteLine("Usage :");
            Console.WriteLine("ServiceUtil.exe [ -install | -remove ] ServiceInfoFileName");
        }

        /// <summary>
        /// This method installs and runs the service in the service control manager.
        /// </summary>
        /// <param name="svcPath">The complete path of the service.</param>
        /// <param name="svcName">Name of the service.</param>
        /// <param name="svcDispName">Display name of the service.</param>
        /// <returns>True if the process went thro successfully. False if there was any error.</returns>
        public bool InstallService(string svcPath, string svcName, string svcDispName)
        {
            #region Constants declaration.
            int SC_MANAGER_CREATE_SERVICE = 0x0002;
            int SERVICE_WIN32_OWN_PROCESS = 0x00000010;
            //int SERVICE_DEMAND_START = 0x00000003;
            int SERVICE_ERROR_NORMAL = 0x00000001;
            int STANDARD_RIGHTS_REQUIRED = 0xF0000;
            int SERVICE_QUERY_CONFIG = 0x0001;
            int SERVICE_CHANGE_CONFIG = 0x0002;
            int SERVICE_QUERY_STATUS = 0x0004;
            int SERVICE_ENUMERATE_DEPENDENTS = 0x0008;
            int SERVICE_START = 0x0010;
            int SERVICE_STOP = 0x0020;
            int SERVICE_PAUSE_CONTINUE = 0x0040;
            int SERVICE_INTERROGATE = 0x0080;
            int SERVICE_USER_DEFINED_CONTROL = 0x0100;
            int SERVICE_ALL_ACCESS = (STANDARD_RIGHTS_REQUIRED |
                      SERVICE_QUERY_CONFIG |
                      SERVICE_CHANGE_CONFIG |
                      SERVICE_QUERY_STATUS |
                      SERVICE_ENUMERATE_DEPENDENTS |
                      SERVICE_START |
                      SERVICE_STOP |
                      SERVICE_PAUSE_CONTINUE |
                      SERVICE_INTERROGATE |
                      SERVICE_USER_DEFINED_CONTROL);
            int SERVICE_AUTO_START = 0x00000002;
            #endregion Constants declaration.

            try
            {
                IntPtr sc_handle = OpenSCManager(null, null, SC_MANAGER_CREATE_SERVICE);
                if (sc_handle.ToInt32() != 0)
                {
                    IntPtr sv_handle = CreateService(sc_handle, svcName, svcDispName,
                                     SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS,
                                     SERVICE_AUTO_START, SERVICE_ERROR_NORMAL,
                                     svcPath, null, 0, null, null, null);
                    if (sv_handle.ToInt32() == 0)
                    {
                        CloseServiceHandle(sc_handle);
                        return false;
                    }
                    else
                    {
                        //now trying to start the service
                        int i = StartService(sv_handle, 0, null);
                        // If the value i is zero, then there was an error starting the service.
                        // note: error may arise if the service is already running or some other problem.
                        if (i == 0)
                        {
                            Console.WriteLine("Couldnt start service");
                            return false;
                        }
                        Console.WriteLine("Success");
                        CloseServiceHandle(sc_handle);
                        return true;
                    }
                }
                else
                {
                    Console.WriteLine("SCM not opened successfully");
                    return false;
                }
            }
            catch (Exception e)
            {
                throw e;
            }
        }


        /// <summary>
        /// This method uninstalls the service from the service conrol manager.
        /// </summary>
        /// <param name="svcName">Name of the service to uninstall.</param>
        public bool UnInstallService(string svcName)
        {
            int GENERIC_WRITE = 0x40000000;
            IntPtr sc_hndl = OpenSCManager(null, null, GENERIC_WRITE);
            if (sc_hndl.ToInt32() != 0)
            {
                int DELETE = 0x10000;
                IntPtr svc_hndl = OpenService(sc_hndl, svcName, DELETE);
                if (svc_hndl.ToInt32() != 0)
                {
                    int i = DeleteService(svc_hndl);
                    if (i != 0)
                    {
                        CloseServiceHandle(sc_hndl);
                        Console.WriteLine("Service was Removed");
                        return true;
                    }
                    else
                    {
                        CloseServiceHandle(sc_hndl);
                        return false;
                    }
                }
                else
                {
                    return false;
                }
            }
            else
            {
                return false;
            }
        }

        static void process(string arg)
        {
            if (arg != null)
            {
                Console.WriteLine(arg);
            }
        }
    }
}
