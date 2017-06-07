/* Copyright 2010, Object Management Group, Inc.
 * Copyright 2010, PrismTech, Inc.
 * Copyright 2010, Real-Time Innovations, Inc.
 * All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package org.omg.dds.core;

import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.TimeUnit;

import org.omg.dds.core.policy.PolicyFactory;
import org.omg.dds.core.status.Status;
import org.omg.dds.domain.DomainParticipantFactory;
import org.omg.dds.type.TypeSupport;
import org.omg.dds.type.builtin.KeyedBytes;
import org.omg.dds.type.builtin.KeyedString;
import org.omg.dds.type.dynamic.DynamicDataFactory;
import org.omg.dds.type.dynamic.DynamicTypeFactory;


/**
 * DDS implementations are rooted in this class, a concrete subclass
 * of which can be instantiated based on a system property.
 * <p>
 * All public concrete and abstract methods of this class are reentrant. The
 * reentrancy of any new methods that may be defined by subclasses is
 * unspecified.
 */
public abstract class ServiceEnvironment implements DDSObject {
    // -----------------------------------------------------------------------
    // Public Fields
    // -----------------------------------------------------------------------

    public static final String IMPLEMENTATION_CLASS_NAME_PROPERTY =
        "org.omg.dds.serviceClassName";



    // -----------------------------------------------------------------------
    // Private Fields
    // -----------------------------------------------------------------------

    private static final String ERROR_STRING =
        "Unable to load OMG DDS implementation. ";



    // -----------------------------------------------------------------------
    // Object Life Cycle
    // -----------------------------------------------------------------------

    /**
     * Create and return a new instance of a concrete implementation of this
     * class with the given environment. This method is equivalent to calling:
     *  <p>
     * <code>
     * createInstance(IMPLEMENTATION_CLASS_NAME_PROPERTY, null, classLoader);
     * </code>
     *
     * @see     #createInstance(String, Map, ClassLoader)
     * @see     #IMPLEMENTATION_CLASS_NAME_PROPERTY
     */
    public static ServiceEnvironment createInstance(
            ClassLoader classLoader)
    {
        return createInstance(
                IMPLEMENTATION_CLASS_NAME_PROPERTY,
                null,
                classLoader);
    }


    /**
     * Look up the system property identified by the given string and load,
     * then instantiate, the ServiceEnvironment implementation class
     * identified by its value. The class must be accessible and have a
     * public constructor.
     * <p>
     * The public constructors of the implementation class will first be
     * searched for one accepting a single argument of type {@link Map}. If
     * one is found, it will be called with the <code>environment</code> map
     * provided to this method as its argument. If no such constructor is
     * found, a no-argument constructor will be used instead, and the
     * provided <code>environment</code>, if any, will be ignored. If the
     * implementation class provides no public constructor with either of
     * these signatures, an exception will be thrown.
     * <p>
     * By default, the class loader for the <code>ServiceEnvironment</code>
     * class will be used to load the indicated class. If this class loader
     * is null -- for instance, if it is the bootstrap class loader -- then
     * the system class loader will be used in its place. If it is also null,
     * a <code>ServiceConfigurationException</code> will be thrown.
     *  <p>
     * Neither the class loader nor the loaded class will be cached between
     * invocations of this method. As a result, execution of this method is
     * expected to be relatively expensive. However, as any DDS object can
     * provide a reference to its creating ServiceEnvironment via
     * {@link org.omg.dds.core.DDSObject#getEnvironment()}, executions of this method are also
     * expected to be rare.
     *
     * @param   implClassNameProperty       The name of a system property,
     *          the value of which will be taken as the name of a
     *          ServiceEnvironment implementation class to load.
     * @param   environment                 A collection of name-value pairs
     *          to be provided to the concrete ServiceEnvironment subclass.
     *          If that class does not provide a constructor that can accept
     *          this environment, the environment will be ignored. This
     *          argument may be null; a null environment shall be considered
     *          equivalent to an empty map.
     * @param   classLoader                 The class loader to use to load
     *          the service implementation class. If it is null, this class's
     *          class loader will be used if it is accessible; otherwise, the
     *          system class loader will be used.
     *
     * @return  A non-null ServiceEnvironment.
     *
     * @throws  NullPointerException        If the given property name is
     *          null.
     * @throws  IllegalArgumentException    If the given property name
     *          is the empty string.
     * @throws  ServiceConfigurationException   If the class could not be
     *          loaded because of an issue with the the invocation of this
     *          method or the configuration of the runtime environment. For
     *          example, the class may not be on the class path, it may
     *          require a native library that is not available, or an
     *          inappropriate class may have been requested (e.g. one that is
     *          not a ServiceEnvironment or that doesn't have a no-argument
     *          constructor).
     * @throws  ServiceInitializationException  If the class was found but
     *          could not be initialized and/or instantiated because of an
     *          error that occurred within its implementation.
     *
     * @see     #createInstance(ClassLoader)
     * @see     DDSObject#getEnvironment()
     * @see     System#getProperty(String)
     * @see     Class#getClassLoader()
     * @see     ClassLoader#getSystemClassLoader()
     * @see     ClassLoader#loadClass(String)
     */
    public static ServiceEnvironment createInstance(
            String implClassNameProperty,
            Map<String, Object> environment,
            ClassLoader classLoader)
    {
        // --- Get implementation class name --- //
        /* System.getProperty checks the implClassNameProperty argument as
         * described in the specification for this method and throws
         * NullPointerException or IllegalArgumentException if necessary.
         */
        String className = System.getProperty(implClassNameProperty);
        if (className == null || className.length() == 0) {
            // no implementation class name specified
            throw new ServiceConfigurationException(
                    ERROR_STRING + "Please set " +
                        implClassNameProperty + " property.");
        }

        try {
            // --- Load implementation class --- //
            if (classLoader == null) {
                classLoader = getDefaultClassLoader();
            }
            assert classLoader != null;
            /* IMPORTANT: Load class with ClassLoader.loadClass, not with
             * Class.forName. The latter provides insufficient control over
             * the class loader used and also caches class references in
             * undesirable ways, both of which can cause problems in
             * container environments such as OSGi.
             */
            Class<?> ctxClass = classLoader.loadClass(className);

            // --- Instantiate new object --- //
            try {
                // First, try a constructor that will accept the environment.
                Constructor<?> ctor = ctxClass.getConstructor(Map.class);
                return (ServiceEnvironment) ctor.newInstance(environment);
            } catch (NoSuchMethodException nsmx) {
                /* No Map constructor found; try a no-argument constructor
                 * instead.
                 *
                 * Get the constructor and call it explicitly rather than
                 * calling Class.newInstance(). The latter propagates all
                 * exceptions, even checked ones, complicating error handling
                 * for us and the user.
                 */
                Constructor<?> ctor = ctxClass.getConstructor(
                        (Class<?>[]) null);
                return (ServiceEnvironment) ctor.newInstance((Object[]) null);
            }

            // --- Initialization problems --- //
        } catch (ExceptionInInitializerError initx) {
            // Presumably thrown by ClassLoader.loadClass, but not documented.
            // Thrown by Constructor.newInstance.
            throw new ServiceInitializationException(
                    ERROR_STRING + "Error during static initialization.",
                    initx.getCause());
        } catch (InvocationTargetException itx) {
            // Thrown by Constructor.newInstance
            throw new ServiceInitializationException(
                    ERROR_STRING + "Error during object initialization.",
                    itx.getCause());

            // --- Configuration problems --- //
        } catch (ClassNotFoundException cnfx) {
            // Thrown by ClassLoader.loadClass.
            throw new ServiceConfigurationException(
                    ERROR_STRING + className + " was not found.",
                    cnfx);
        } catch (LinkageError linkx) {
            // Presumably thrown by ClassLoader.loadClass, but not documented.
            throw new ServiceConfigurationException(
                    ERROR_STRING + className + " could not be loaded.",
                    linkx);
        } catch (NoSuchMethodException nsmx) {
            // Thrown by Class.getConstructor: no no-argument constructor
            throw new ServiceConfigurationException(
                    ERROR_STRING + className +
                        " has no appropriate constructor.",
                    nsmx);
        } catch (IllegalAccessException iax) {
            // Thrown by Constructor.newInstance
            throw new ServiceConfigurationException(
                    ERROR_STRING + className +
                        " has no appropriate constructor.",
                    iax);
        } catch (InstantiationException ix) {
            // Thrown by Constructor.newInstance
            throw new ServiceConfigurationException(
                    ERROR_STRING + className + " could not be instantiated.",
                    ix);
        } catch (SecurityException sx) {
            // Thrown by Class.getConstructor.
            throw new ServiceConfigurationException(
                    ERROR_STRING + "Prevented by security manager.", sx);
        } catch (ClassCastException ccx) {
            // Thrown by type cast
            throw new ServiceConfigurationException(
                    ERROR_STRING + className +
                        " is not a ServiceEnvironment.", ccx);

            // --- Implementation problems --- //
        } catch (IllegalArgumentException argx) {
            /* Thrown by Constructor.newInstance to indicate that formal
             * parameters and provided arguments are not compatible. Since
             * the constructor doesn't take any arguments, and we didn't
             * provide any, we shouldn't be able to get here.
             */
            throw new AssertionError(argx);
        }
        /* If any other RuntimeException or Error gets thrown above, it's
         * either a bug in the implementation of this method or an
         * undocumented behavior of the Java standard library. In either
         * case, there's not much we can do about it, so let the exception
         * propagate up the call stack as-is.
         */
    }

    // -----------------------------------------------------------------------
    // Instance Methods
    // -----------------------------------------------------------------------

    /**
     * <em>This method is not intended for use by applications.</em> The
     * DDS-standard classes use it to delegate to a Service implementation.
     */
    public abstract ServiceProviderInterface getSPI();


    // --- From DDSObject: ---------------------------------------------------

    @Override
    public final ServiceEnvironment getEnvironment() {
        return this;
    }


    // --- Private methods: --------------------------------------------------

    /**
     * A client did not provide a non-null class loader to be used to load
     * the DDS service implementation; choose an alternative.
     * <p>
     * If it can, this method will use the class loader used to load this
     * class. If this class loader is null -- most likely it is the bootstrap
     * class loader -- then the system class loader will be used instead.
     *
     * @return  a non-null class loader.
     *
     * @throws  ServiceConfigurationException   if the system class loader
     *                                          is not accessible.
     * @throws  ServiceInitializationException  if the system class loader
     *                                          could not be initialized.
     */
    private static ClassLoader getDefaultClassLoader() {
        // --- Get class loader from this class --- //
        ClassLoader classLoader =
                ServiceEnvironment.class.getClassLoader();
        if (classLoader != null) {
            return classLoader;
        }

        // --- Fallback: get system class loader --- //
        /* The class loader is probably the bootstrap class loader, which is
         * not directly accessible. Substitute the system class loader if
         * possible.
         */
        try {
            classLoader = ClassLoader.getSystemClassLoader();
        } catch (SecurityException sx) {
            throw new ServiceConfigurationException(
                    ERROR_STRING + "Prevented by security manager.",
                    sx);
        } catch (IllegalStateException isx) {
            /* The documentation for ClassLoader.getSystemClassLoader()
             * says this is thrown if the system class loader tries to
             * instantiate itself recursively. This situation should not
             * occur unless a custom system class loader is injected
             * which uses DDS.
             */
            throw new ServiceConfigurationException(
                    ERROR_STRING +
                        "Circular system class loader dependencies.",
                    isx);
        } catch (Error err) {
            /* The documentation for ClassLoader.getSystemClassLoader() says
             * this is thrown if the system class loader cannot be
             * reflectively instantiated.
             */
            throw new ServiceConfigurationException(
                    ERROR_STRING +
                        "System class loader could not be initialized.",
                    err.getCause());
        }

        // --- Check for null return result --- //
        /* The documentation for ClassLoader.getSystemClassLoader() says that
         * the method may return null if there is no system class loader.
         * However, it doesn't say why that would be the case.
         *
         * Do this check outside of the try/catch above to make sure that
         * no exceptions thrown below will be handled incorrectly. The
         * exception handling logic above is closely tied to the documented
         * behavior of ClassLoader.getSystemClassLoader().
         */
        if (classLoader == null) {
            throw new ServiceConfigurationException(
                    ERROR_STRING + "No system class loader available.");
        }
        return classLoader;
    }

    // -----------------------------------------------------------------------
    // Service-Provider Interface
    // -----------------------------------------------------------------------

    /**
     * This interface is for the use of the DDS implementation, not of DDS
     * applications. It simplifies the creation of objects of certain types in
     * the DDS API.
     */
    public static interface ServiceProviderInterface {
        // --- Singleton factories: ------------------------------------------

        public DomainParticipantFactory getParticipantFactory();

        public DynamicTypeFactory getTypeFactory();


        // --- Types: --------------------------------------------------------

        /**
         * Create a new {@link org.omg.dds.type.TypeSupport} object for the given physical
         * type. The Service will register this type under the given name
         * with any participant with which the <code>TypeSupport</code> is
         * used.
         *
         * @param <TYPE>    The physical type of all samples read or written
         *                  by any {@link org.omg.dds.sub.DataReader} or
         *                  {@link org.omg.dds.pub.DataWriter} typed by the
         *                  resulting <code>TypeSupport</code>.
         * @param type      The physical type of all samples read or written
         *                  by any {@link org.omg.dds.sub.DataReader} or
         *                  {@link org.omg.dds.pub.DataWriter} typed by the
         *                  resulting <code>TypeSupport</code>.
         * @param registeredName    The logical name under which this type
         *                  will be registered with any
         *                  {@link org.omg.dds.domain.DomainParticipant}
         *                  with which the resulting
         *                  <code>TypeSupport</code> is used.
         * @return          A new <code>TypeSupport</code> object, which can
         *                  subsequently be used to create one or more
         *                  {@link org.omg.dds.topic.Topic}s.
         */
        public <TYPE> TypeSupport<TYPE> newTypeSupport(
                Class<TYPE> type, String registeredName);


        // --- Time & Duration: ----------------------------------------------

        /**
         * Construct a {@link org.omg.dds.core.Duration} of the given magnitude.
         * <p>
         * A duration of magnitude {@link Long#MAX_VALUE} indicates an
         * infinite duration, regardless of the units specified.
         */
        public Duration newDuration(long duration, TimeUnit unit);

        /**
         * @return      A {@link org.omg.dds.core.Duration} of infinite length.
         */
        public Duration infiniteDuration();

        /**
         * @return      A {@link org.omg.dds.core.Duration} of zero length.
         */
        public Duration zeroDuration();

        /**
         * Construct a specific instant in time.
         * <p>
         * Negative values are considered invalid and will result in the
         * construction of a time <code>t</code> such that:
         * <p>
         * <code>t.isValid() == false</code>
         */
        public ModifiableTime newTime(long time, TimeUnit units);

        /**
         * @return      A {@link org.omg.dds.core.Time} that is not valid.
         */
        public Time invalidTime();


        // --- Instance handle: ----------------------------------------------

        public InstanceHandle nilHandle();


        // --- Conditions & WaitSet: -----------------------------------------

        public GuardCondition newGuardCondition();

        public WaitSet newWaitSet();

        // --- Status: -------------------------------------------------------

        public Set<Class<? extends Status>> allStatusKinds();

        public Set<Class<? extends Status>> noStatusKinds();

        // --- QoS Provider --------------------------------------------------
        /** Create a QosProvider fetching QoS configuration from the specified URI.
         *  The URI determines the how the Qos configuration is fetched and the
         *  format in which it is represented. This specification requires compliant
         *  implementations to support at least one file based configuration using
         *  the XML syntax defined as part of the DDS for CCM specification (formal/12.02.01).
         *
         *  @param uri The uniform resource identifier. For example,
         *             "file:///somewhere/on/disk/qos-config.xml"
         *             "http:///somewhere.org/here/json-config.json"
         *  @param profile Name of a profile in the document obtained via the uri
         *  @return a new QosProvider object
         */
        public abstract QosProvider newQosProvider(String uri, String profile);

        // --- PolicyFactory -----------------------------------------------------

        /**
         * Provides an instance of {@link org.omg.dds.core.policy.PolicyFactory}.
         * @return An instance of {@link org.omg.dds.core.policy.PolicyFactory}
         */
        public abstract PolicyFactory getPolicyFactory();

        public abstract DynamicDataFactory getDynamicDataFactory();

     // --- Built-in Types -----------------------------------------------------

        public abstract KeyedString newKeyedString();

        public abstract KeyedBytes newKeyedBytes();
    }
}
