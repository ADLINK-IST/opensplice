.. _`Python Scripting Engine`:



#######################
Python Scripting Engine
#######################

*This section describes writing Python scripts to test OSPL applications.*


**********************
About Python Scripting
**********************

Python Scripting is a scripting environment for running unit tests and adhoc scripts against a Vortex OpenSplice environment.
Python Scripting connects to the OpenSplice environment through the Configuration and Management API.
It is based on the Python scripting language, and requires Jython 2.7.0 or later, the Java-based implementation of Python.

Design Goals
============

Python Scripting is an alternative to the product specific Scenario language found in Vortex Tester.
Python Scripting design goals were:

* use a standard scripting language (Python)
* allow users to leverage the tooling environment build around this standard language
* allow users to leverage the testing infrastructure and libraries
* expose DDS semantics, particularly on reading and writing topic data in an easy-to-use API for Python.
* run independently of Vortex Tester
* allow execution of scripts and unit tests both directly from the command line and from an Integrated Development Environment (IDE)


*************
Configuration
*************

This section explains how to download and configure a Jython scripting engine with the Tester Scripting package.
Jython is an Java-based implementation of the Python scripting language.

Prerequisites
=============

A Java 7 or later runtime (JRE) is required. The JAVA_HOME environment variable must be set to the JRE installation directory.

Check that the following command yields a Java version of 1.7 or later. On Linux systems, use::

   $JAVA_HOME/bin/java -version

On Windows systems, use::

   %JAVA_HOME%\bin\java -version

Note that during the installation, it is assumed the the environment variable OSPL_HOME refers the the fully qualified
path of the Vortex OpenSplice installation.

Download and install Jython
===========================

The Tester Scripting package does not include the Jython scripting engine - it must be downloaded.
Jython is an open source project licensed under the Python Software Foundation License Version 2 (http://www.jython.org/license.html).
The license is OSI Approved (https://opensource.org/licenses/alphabetical).

Tester Scripting requires Jython 2.7.0 or later. Follow the following steps to download and install Jython:

1. Download the Jython 2.7.0 Installer from the Jython Download page (http://www.jython.org/downloads.html).

2. Install Jython. It is recommended, but not necessary that Jython be installed in the $OSPL_HOME/jython directory (linux) or the %OSPL_HOME%\jython directory (Windows). The following command line will install the standard Jython distribution::

    # for Linux systems
    $JAVA_HOME/bin/java -jar jython-installer-2.7.0.jar -s -d $OSPL_HOME/jython -t standard

    # For Windows Systems
    "%JAVA_HOME%\bin\java" -jar jython-installer.2.7.0.jar -s -d "%OSPL_HOME%\jython" -t standard

3. Change to the OSPL_HOME directory.

4. Install the Tester Scripting python package. Use the following command::

    # for Linux systems
    jython/bin/easy_install tools/scripting/OSPLScript-1.0.0.tar.gz

    # for Windows systems
    jython\bin\easy_install tools\scripting\OSPLScript-1.0.0.tar.gz

Verifying the installation
==========================

The following steps will verify that the Jython installation is correctly configured.

1. Ensure OpenSplice is running by issuing the following command::

    ospl start

2. Start OpenSplice Tester to create some test topics used below::

    ospltest

3. Start the Jython interpreter::

    # for Linux systems
    jython/bin/jython -Djava.ext.dirs=$OSPL_HOME/jar

    # for Windows systems
    jython\bin\jython -Djava.ext.dirs=%OSPL_HOME%\jar

4. Enter the following commands at the interpreter command prompt:

    >>> from osplscript import dds
    >>> topic = dds.findTopic('OsplTestTopic')
    >>> OsplTestTopic = dds.classForTopic(topic)
    >>> data = OsplTestTopic()
    >>> data.id = 1
    >>> data.description = 'Smoke test'

    >>> writer = dds.Writer(topic)
    >>> writer.write(data)

    >>> reader = dds.Reader(topic)
    >>> sample = reader.take()
    >>> readData = sample.getData()

    >>> assert data.id == readData.id
    >>> assert data.description == readData.description


******************************
A Quick Tour of OSPL Scripting
******************************

The following is a brief tour of the scripting engine's capabilities.

Prerequisites
=============

This demo assumes a shell instance that has been initialized with the release.com script found in the OSPL installation directory.
In particular, the quick tour relies on the $OSPL_HOME, $OSPL_URI and $LD_LIBRARY_PATH variables being appropriately set.
To run the script, do the following::

	cd OSPL-install-directory
	. release.com

Preliminaries
=============

Start OSPL, typically with::

	ospl start

Then, start Vortex OpenSplice Tester::

	ospltest

Tester is used to define topics used in the scripting engine, and to observe samples. From within Tester, create a default Reader on the following topics: OsplTestTopic.

Writing and Reading samples
===========================
The instructions below assume that you have installed and configured Jython in $OSPL_HOME/jython.
Start the OSPL Scripting engine::

	$OSPL_HOME/jython/bin/jython -Djava.ext.dirs=$OSPL_HOME/jar

You will see a standard start up banner from the Jython engine similar to the following::

	Jython 2.7.0 (default:9987c746f838, Apr 29 2015, 02:25:11)
	[Java HotSpot(TM) Server VM (Oracle Corporation)] on java1.7.0_80
	Type "help", "copyright", "credits" or "license" for more information. 
	>>>

The text '>>>' is the interpreter prompt.

To start our script, import the DDS module from OSPL Script. Enter the following at the prompt:

	>>> from osplscript import dds

As part of the import, a connection is made to OSPL. If OSPL is not running, or if the environment variables are not correctly set, you may receive an error at this point.

The dds module provides allows you to find topics, and create readers and writers. We will start with find the OsplTestTopic created when we started Tester. Enter the following at the prompt:

	>>> t = dds.findTopic('OsplTestTopic')
	>>> # The next statement create a Python type from the topic
	>>> OsplTestTopic = dds.classForTopic(t)
	>>> # You can then instantiated instances of this class
	>>> d1 = OsplTestTopic()
	>>> # You can then set fields in the data
	>>> d1.id = 1
	>>> d1.index = 100
	>>> d1.description = 'Hello from osplscript'
	>>> d1.state = 'boost'
	>>> d1.x = 1.1
	>>> d1.y = 2.2
	>>> d1.z = 3.3
	>>> d1.t = 4.4

Once some data is created, we can the create a writer for the topic, and write the data:

	>>> # create a Writer from the topic object we found previously
	>>> w = dds.Writer(t)
	>>> w.write(d1)

Once the write has executed, example the Sample List in Tester. You should see a new sample in the list.

Next, we can create a reader, and read the sample we have just written:

	>>> # create a Reader on the topic, and read a sample
	>>> r = dds.Reader(t)
	>>> s1 = r.take()
	>>> # s1 is a Sample, the user data is access via getData()
	>>> rd1 = s1.getData()

We can check that the read data is what we expected:

	>>> assert rd1.id == 1
	>>> assert rd1.index == 100
	>>> assert rd1.description == 'Hello from osplscript'

You can continue your exploration by writing a sample via Tester, and confirming that you can read it using OSPL Script. When you are done, exit the interpreter by typing:

	>>> exit()

Working with QoS settings
=========================
By default, OSPL Scripting creates a publisher and a subscriber using the partition pattern '*'. All other publisher and subscriber QoS policies are DDS defaults.
Similarly, data readers and data writers have, by default, QoS policies derived from topic to which they are bound. If default QoS policies do not statisfy your requirements,
you can explicitly create publishers and subscribers, and assign them explicity QoS policies. Similarly, you can explicitly assign QoS policies to data readers and data writers
that you create.

The following example shows the explicit creation of a Subscriber:

    >>> from osplscript import dds, qos
    >>> topic = dds.findTopic('OsplTestTopic')
    >>> # create an explicit subscriber on a partition 'test'
    >>> sub = dds.Subscriber(
    >>>     qos.SubscriberQos().withPolicy(
    >>>         qos.Partition().withName('test'))
    >>>     )
    >>> # create a reader with topic-derived defaults
    >>> drDefault = sub.reader(topic)
    >>> # create a reader with explicity reader QoS policies
    >>> drExplicit = sub.reader(
    >>>     topic,
    >>>     qos.DataReaderQos().withPolicy(
    >>>         qos.Durability().withVolatile())
    >>>     )

The following example shows adding explicit QoS policies to a data writer on the default publisher:

    >>> dwExplicit = dds.Writer(
    >>>     topic, 
    >>>     qos.DataWriterQos().withPolicy(
    >>>         qos.Durability().withVolatile())
    >>>     )

The QoS classes for publishers, subscribers, data readers and data writers are, respectively, PublisherQos, SubscriberQos, DataReaderQos and DataWriterQos.
Although OSPL Scripting does not allow creating of topics, topics can return their QoS settings via a TopicQos instance. 
The help for each of these QoS classes describes the applicable policies.
The help for policy classes describes all the methods available to configure the policy.
You can view help on QoS classes and policies via the help function:

    >>> help(qos.SubscriberQos)
    >>> help(qos.Parition)
    >>> help(qos.Durability)

Working with WaitSets
=====================
OSPL Scripting implements DDS wait sets with read conditions, query conditions and status conditions. This allows your code to block until data is
available on a data reader. Here is a simple example:

    >>> from osplscript import dds
    >>> 
    >>> # find the topic, create a data reader and wait set
    >>> topic = dds.findTopic('OsplTestTopic')
    >>> dr = dds.Reader(topic)
    >>> ws = dds.WaitSet()
    >>> 
    >>> # create a read condition of Alive, NotRead, New samples
    >>> rc = dr.readCondition(dds.DataState().withAlive().withNotRead().withNew())
    >>> 
    >>> # attach the read condition to the wait set
    >>> ws.attachCondition(rc)
    >>> 
    >>> # wait...
    >>> ws.waitForConditions()
    >>>
    >>> # waiting returned, we have a sample
    >>> sample = dr.take()
    >>> # do something with the sample

The waitForConditions() method can accept optional arguments, include a timeout. See the help for details:

    >>> help(dds.WaitSet.waitForConditions)

Filtering data
==============
OSPL Scripting allows you to filter data from a reader using a 'selector'. The selector can also be used to create a condition for a wait set.

A selector is created via a reader's newSelectBuilder() method. A 'select builder' allows you to specify state conditions as well as an optional query expression.
The following example creates a selector:

    >>> from osplscript import dds
    >>> 
    >>> # find the topic, create a data reader
    >>> topic = dds.findTopic('OsplTestTopic')
    >>> dr = dds.Reader(topic)
    >>>
    >>> # create a selector for Alive, NotRead, New samples with 'index = 100'
    >>> selector = dr.newSelectBuilder().withAlive().withNotRead().withNew() \
    >>>     .content('index = 100').build()

Each of the 'selector builder' methods returns the builder, so that calls can be chained as above.
Apart from build(), none of the builder methods is required.
A selector with no filters works identically to the data reader from which it was created.

Once created, a selector can be used like a data reader, with take() or read() methods:

    >>> # use the selector like a reader
    >>> sample = selector.take()

Alternatively, the selector can be used with a waitset, by calling the selector's condition() method:

    >>> # Use the selector in conjuction with a waitset
    >>> ws = dds.WaitSet()
    >>> ws.attachCondition(selector.condition())
    >>> ws.waitForConditions()
    >>> # the selector now has data...
    >>> sample = selector.take()

In the above example, the selector.condition() method returns a QueryCondition.
If the content() method had not been called, a ReadCondition would have been returned.

You can create a QueryCondition directly, and then use it with a wait set:

    >>> queryCond = reader.queryCondition(
    >>>     'index = 100', [], 
    >>>     dds.DataState().withAlive().withNotRead().withNew())
    >>> ws.attachCondition(queryCond)

A selector, however, has the advantage of providing your with filtered access to the data that triggered the query condition.
Creating a query or read condition explicitly does not provide such filtering; the data reader from which the condition
was defined will still return all available samples, whether they satisfy the condition or not.
For this reason, selectors are the preferred method for defining data filters, waiting for filtered data availability, and
for accessing the filtered data.

Query Expressions, Query Parameters and their Limitations
=========================================================
Both the selector builder's content() method and the QueryCondition() constructor allow the query expression to contain
substitution parameters of the form {n}, where n is a zero-based index into an list of string parameter values.
For example, we could filter OsplTestTopic's index value to be between an upper and lower bound, and specify the query as follows:

    >>> from osplscript import dds
    >>> 
    >>> # find the topic, create a data reader
    >>> topic = dds.findTopic('OsplTestTopic')
    >>> dr = dds.Reader(topic)
    >>>
    >>> # create a selector for Alive, NotRead, New samples with 'index = 100'
    >>> selector = dr.newSelectBuilder().withAlive().withNotRead().withNew() \
    >>>     .content('index >= {0} and index < {1}', ['100', '200']) \
    >>>     .build()

We could use similar parameters in creating a QueryCondition directly:

    >>> queryCond = reader.queryCondition(
    >>>     'index >= {0} and index < {1}',
    >>>     ['100', '200'],
    >>>     dds.DataState().withAlive().withNotRead().withNew())

OSPL Scripting attempts to replace the passed parameter values in the query expression, formatting the values as valid query expression constants.
However, because of limitations in the APIs available to OSPL Scripting, this formatting is imperfect. In particular, the following values are likely
to be formatted incorrectly:

* enumeration values will incorrectly be quoted
* boolean values will incorrectly be quoted
* string values that can be converted to numbers will incorrectly be unquoted.

The work around for all these limitations is to avoid using parameter substitution.
For example, instead of the following parameterized condition against the 'state' enumeration field in OsplTestTopic:

    >>> # DON'T DO THIS FOR AN ENUMERATED FIELD
    >>> selector = dr.content('state = {0}', ['init']).build()

Instead, write the condition without substitution:

    >>> selector = dr.content('state = init').build()

Using Coherent access
=====================
OSPL Scripting now supports Group and Topic coherence when reading and writing samples.

Group coherence allows a publisher to release a group of samples, possibly spanning
several topics to subscribers in a group.
Subscribers will not see any samples in a coherent group until the publisher has completed group.
A subscriber using Group coherence may, if desired, retrieve the samples, across all readers in the group, in the order
that the publisher wrote them.

Topic coherence allows a publisher to release changes across multiple instances of the sample topic
as a coherent set. Subscribers will not see any samples in a coherent set util the publisher has
completed the set.
A subscriber using Topic coherence may, if desired, retrieve the samples with-in a specific topic, in the order
that the publisher wrote them.

To establish a publisher or subscriber with coherent access, use the Presentation Policy when creating the 
publisher or subscriber QoS:

    >>> from osplscript import dds, qos
    >>>
    >>> # create a presentation policy, enabling Group coherence and ordered access
    >>> groupPresentation = qos.Presentation().withCoherentAccess().withGroup() \
    >>>     .withOrderedAccess()
    >>>
    >>> # create a publisher with the policy
    >>> pub = dds.Publisher(qos.PublisherQos().withPolicy(groupPresentation))
    >>>
    >>> # create a subscriber with the policy
    >>> sub = dds.Subscriber(qos.SubscriberQos().withPolicy(groupPresentation))

To create a presentation policy with Topic scope, use the withTopic() method.
To create a presentation policy with Instance scope (the default), use the withInstance() method.

To write samples using any coherent scope, the publisher object offers the methods beginCoherentChanges()
and endCoherentChanges(). Each call beginCoherentChanges() should be matched with a call to endCoherentChanges().
Calls may be nested, for programmer convenience, but only the outer most pair of calls have any impact.
Within a coherent change, any of the publisher's data writers may be used to write samples.
When the coherent change is completed (via a call to endCoherentChanges()), the samples are released to subscribers.

To enable subscribers to read coherent changes, the subscriber object offers two methods: beginAccess() and
endAccess(). Their use is optional, but without them, the subscriber will not guarantee that samples will be returned
according to the coherent groups in which they were created.

The following code pattern my be used to access samples written with Group coherence, or with ordered access:

    >>> # sub is a subscriber with Group coherence presentation policy
    >>> # the subscriber must create data readers for all the topics
    >>> # it wants to access
    >>> sub.beginAccess()
    >>> # return the dataReaders, in the order their corresponding writers
    >>> # wrote samples
    >>> drList = sub.dataReaders()
    >>> for dr in drList:
    >>>     # take (or read) ONLY ONE sample for each element of the list
    >>>     sample = dr.take()
    >>>     # using the reader's topicDescription() method to identify the topic
    >>>     if 'foo' == dr.topicDescription().getName():
    >>>         # do something with a 'foo' sample
    >>> sub.endAccess()

When not using Group access or ordered access, a subscriber may still use the dataReaders() method, however,
in this case, dataReaders() will return a set of readers with available data.
The subscribing application can then take or read as many samples as are available from each reader in the set.

Note: Using waitsets with group coherence
-----------------------------------------
The DDS specification indicates that subscribers will raise a 'data available on readers' event when a new
group is release. 
However, a defect in underlying APIs prevent OPSL Scripting from receiving this event.
In other words, the following will not work:

    >>> # This will NOT WORK!!!
    >>> from osplscript import status
    >>> ws = dds.WaitSet()
    >>> sc = sub.statusCondition()
    >>> sc.setEnabledStatuses([status.DataOnReadersStatus])
    >>> ws.attachCondition(sc)
    >>> ws.waitForConditions()
    >>> # will NEVER get here

As a work around, attached status conditions from each of the subscriber's readers to the wait set:

    >>> # This will work
    >>> ws = dds.WaitSet()
    >>>
    >>> # do this for each reader (dr) you care about
    >>> sc = dr.statusCondition()
    >>> sc.setEnabledStatuses([status.DataAvailableStatus])
    >>> ws.attachCondition(sc)
    >>>
    >>> ws.waitForConditions()

Creating a unit test script
===========================
Although using an interactive interpreter provides instant feedback, it is more likely that you will create script files, and execute them.
In this section, we will create and execute a script that performs a unit test using the stand python unittest module.
Start by created a text file in your favourite editor. Call the file firstUnitTest.py. Copy and paste the text below::

	import unittest
	from osplscript import dds

	class firstUnitTest(unittest.TestCase):

		def testReadOsplTestTopic(self):
		    t = dds.findTopic('OsplTestTopic')
		    dw = dds.Writer(t)
		    OsplTestTopic = dds.classForTopic(t)
		    wdata = OsplTestTopic()
		    wdata.id = 5
		    wdata.x = 5.1
		    wdata.y = 5.2
		    wdata.z = 5.3
		    wdata.t = 5.4
		    wdata.state = 'hit'
		    wdata.index = 5
		    wdata.description = 'test'
		    dw.write(wdata)

		    dr = dds.Reader(t)
		    sample = dr.take()
		    rdata = sample.getData()

		    self.assertEqual(wdata.id, rdata.id)
		    self.assertEqual(wdata.x, rdata.x)
		    self.assertEqual(wdata.y, rdata.y)
		    self.assertEqual(wdata.z, rdata.z)
		    self.assertEqual(wdata.t, rdata.t)
		    self.assertEqual(wdata.state, rdata.state)
		    self.assertEqual(wdata.index, rdata.index)
		    self.assertEqual(wdata.description, rdata.description)

	# standard python to run the unit test from the command line
	if __name__ == "__main__":
		#import sys;sys.argv = ['', 'Test.testName']
		unittest.main()

This test case essentially repeats the test we created in the interpreter. To run the test, enter the following command in your shell command prompt::

	$OSPL_HOME/jython/bin/jython -Djava.ext.dirs=$OSPL_HOME/jar firstUnitTest.py

The script engine will respond with output like the following::

	.
	----------------------------------------------------------------------
	Ran 1 test in 0.107s

	OK

The output is compact. Python's unit test philosophy is to minimize output except in the case of test failures.
Experiment with the test to introduce a failure, and see how the output changes.

Working with more Complex topics
================================
The OsplTestTopic used above is simple. This section examines working with more complex topics – ones that include sequences, arrays, nested structures and unions.
As with the preceding examples, Tester should be running, as it creates the samples that are used in this example.
If OSPL Scripting is not running, start it::

$OSPL_HOME/jython/bin/jython -Djava.ext.dirs=$OSPL_HOME/jar

The use the following Python to find the OsplSequenceTopic DDS topic and create a Python class from it:

	>>> from osplscript import dds

	>>> seqTopic = dds.findTopic('OsplSequenceTopic')
	>>> seqClass = dds.classForTopic(seqTopic)
	>>> seqInstance = seqClass()

From there, you can explore the instance data object. Fundamentally, it behaves pretty much like a C 'struct' would:
top level fields are accessed via the dot notation:

	>>> seqInstance.id = 1

Fields that are arrays or sequences may be indexed with zero-based value. Standard python sequence methods may be used to add and remove elements from the lists.
(Fields declared as arrays are pre-allocated to the declared size. OsplSequenceTopic contains no array fields, but you can explore OsplArrayTopic, which does.)

	>>> seqInstance.iVector.append(1)
	>>> seqInstance.iVector.append(2)
	>>> assert seqInstance.iVector[0] == 1
	>>> assert seqInstance.iVector[1] == 2
	>>> del seqInstance.iVector[0]
	>>> assert seqInstance.iVector[0] == 2

If a class includes a a sequence of structures, then a fieldName_new() method is created so you can instantiate instances of the class:

	>>> seqInstance.pVector.append(seqInstance.pVector_new())
	>>> seqInstance.pVector[0].state = 'boost'

Creating a sample time-line
===========================
The OSPL Scripting engine includes module (osplscript.recorder) that can automatically read samples from one or more topics and return these in the order received.
These sample sequence can then be queried using standard python mechanisms to create tests that consider multiple samples at once.
To use the recorder module, do the following:

	>>> from osplscript.recorder import Recorder
	>>> t1Recorder = Recorder('OsplTestTopic')
	>>> t2Recorder = Recorder('OsplSequenceTopic')
	>>> # … time passes …
	>>> # get a snapshot of samples recorded
	>>> t1Samples = t1Recorder.getSamples()
	>>> t2Samples = t2Recorder.getSamples()
	>>> # clear the recorded samples, but keep recording
	>>> t1Recorder.clearSamples()
	>>> # stop recording, the sample list is still available, but no longer updated
	>>> t1Recorder.stop()

**********************************************************
Using Eclipse and PyDev to create and run OsplScript files
**********************************************************

Eclipse is a popular open source IDE. PyDev is a Python specific open source add-on for Eclipse.
This chapter describes using Eclipse and PyDev.

Download and Installation
=========================

Eclipse may be obtained from the `Eclipse Download page <http://www.eclipse.org/downloads/>`_. Choose the **IDE for Java Developers** download.
Installation Instructions are available on the `Eclipse Install Guide page <http://wiki.eclipse.org/Eclipse/Installation>`_.

Once Eclipse is installed, start it and choose or create a workspace. You are then ready to proceed with
installing PyDev. See the `PyDev Download page <http://www.pydev.org/download.html>`_ for instructions - instructions appear on the
right-hand side of the page under **Quick Install** > **Update Manager**.

Configuration
=============

Once PyDev is installed, you must configure it with the location of you Jython installation:

1. Start Eclipse, and choose a workspace if prompted.
2. From the menu, choose **Window** -> **Preferences**.
3. In the left-hand tree, find  and click **PyDev** > **Interpreters** > **Jython interpreters**.
4. Click the **Add** button.
5. Enter a name for the Jython interpreter. Example: Tester Script
6. Browser for the jython.jar file in the root directory of the Tester Script Jython installation.
7. Click OK.
8. Still in the Preferences dialog on the PyDev > Interpreters > Jython interpreters page, select your
newly created interpreter entry in the upper list. Then click the **Environment** tab in the lower half
of the dialog.
9. Add the following environment variables: OSPL_HOME, OSPL_URI and LD_LIBRARY_PATH. Their values should
be the same as those found in your command line environment.
10. Click the **Libraries** tab in the lower half of the dialog.
11. Click **New Folder** and browse for and select the **jar** directory under your Vortex OpenSplice installation directory.
12. Click **OK** to complete the folder selection.
13. Click **OK** to close the preferences dialog.

You are new ready to create a new Jython project.

Creating a PyDev Project
========================

This section describes creating a project for editing Python files.

1. Start Eclipse and choose a workspace, if it is not already running.
2. From the menu, choose **File** -> **New** -> **Project**.
3. In the new project wizard, select **PyDev** > **PyDev Project**. Click **Next**.
4. Enter a project name.
5. Ensure the **Project type** is set to **Jython**.
6. Ensure that the **Interpreter** is set to your Tester Script Jython interpreter configured above.
7. Click **Finish**.
8. In the Package Explorere, right click over the newly created project, and choose **Properties**.
9. In the left-hand pane, click **PyDev - PYTHONPATH**.
10. Click the **External Libraries** tab.
11. Click **Add zip/jar/egg**, and choose cmapi.jar from the jar subdirectory of your Vortex OpenSplice installation. Click **OK**.
12. Click **Add zip/jar/egg**, and choose osplscript.jar from the jar subdirectory of your Vortex OpenSplice installation. Click **OK**.
13. Click **OK** to close the properties dialog.

You may be prompted to switch to the PyDev perspective. This is optional. The PyDev perspective adapts
the Eclipse display for editing python files with PyDev. If you are new to Eclipse, it is recommended that
your switch to the PyDev perspective. The following instructions assume you are in this perspective.

Create a Python script
======================

Python files may declare classes, define unit tests, or just provide instructions that
are to be executed when the file is run. To create a Python file:

1. From the menu choose **File** -> **New PyDev Module**.
2. If not set, browser for a Source Folder, which must be a directory in your project.
3. Optionally, enter a package name.
4. Enter the name of the python file. PyDev will add a .py extension automatically.
5. Click **OK**

An editor will open, and you will be prompted for a template for the newly created file. The most common choices are:
* one of UnitTest variations,  if you want to write tests.
* Main, if you want to write a python script to be executed directly by the interpreter
* Class, if you want to define python classes to be consumed by other modules.

Running a Python script
=======================

To run a python script (or unit test), do the following:

1. Right click anywhere in the editor and choose **Run As** -> **Jython Run**.
(For unit tests, choose **Run As** -> **Jython unit-test**.)

Debugging a Python script
=========================

Launching a debugger is similar to running a script. Right click the script, and choose **Debug As** and then
the appropriate sub-menu item. While debugging, note the following:

* You can set break points in a script by clicking in the left margin of the script editor.

************************************************
Using PyCharm to create and run Tester Scripting
************************************************

PyCharm is a Python specific IDE developed by Jet Brains, the makes of IntelliJ IDEA.
PyCharm comes in several forms. This chapter describes using the free Community Edition.

Note that similar instructions apply for using IntelliJ IDEA with the Python Plugin, as the Python Plugin
adds very similar capabilities.


Download and Installation
=========================

PyCharm may be obtained from the `PyCharm Download page <https://www.jetbrains.com/pycharm/download/>`_. Choose the Community Edition download.
The page includes a link to Installation Instructions appropriate to your platform.

Configuration
=============

This subsection explains how to configure PyCharm so that is will work with the Jython
installation you created and configured with Tester Script.

1. Start PyCharm. On the Welcome script.
2. Click the **Configure** drop down near the bottom of the screen, and then choose **Settings**.
3. In the left-hand tree, find **Default Project**, expand it and select **Project Interpreter**.
4. On the right-hand side of the dialog, click the gear icon, and choose **Add Local**.
5. In the file selection dialog, browse for the your Jython executable from your Jython installation. The Jython installation instructions used the following location: $OSPL_HOME/jython/bin/jython
6. Click OK to close the selection dialog. Click OK again to close the Settings dialog.

You are new ready to create a new Jython project.

Creating a PyCharm Project
==========================

This section describes creating Python projects in PyCharm.

1. Start PyCharm, if it is not already running.
2. If the Welcome screen is showing, click **Create New Project**. Otherwise, select **New Project** from the **File** menu.
3. Provide a project name and location. Verify that the Interpreter is the Tester Scripting Jython interpreter. Click **Create**.

Create a Python script
======================

Python files may declare classes, define unit tests, or just provide instructions that
are to be executed when the file is run. To create a Python file:

1. From the menu choose **File** -> **New...**.
2. In the pop-up that displays, click **Python File**.
3. Provide a name. If you do not add it, PyCharm will add a .py extension.
4. The **Kind** drop down allows you to choose between **Python file** or **Python unit test**.
5. Click **OK** to create the file. The file will open in an editor.

Running a Python script
=======================

Running a script requires some the first time setup:

1. From the menu, choose **Run** -> **Edit Configurations**.
2. In the left-hand tree, expand **Defaults** and click **Python**
3. Press the button containing ellipsis at the end of the **Environment Variables** line.
4. Add the following environment variables (as defined in your environment): OSPL_HOME, OSPL_URI and LD_LIBRARY_PATH
5. Click **OK** to close the Environment Variables dialog
6. In the **Interpreter Options** edit box, enter:
    -Djava.ext.dirs=<full-path-to-OSPL_HOME>/jar
7. Ensure **Python intepreter** is set to the Jython implementation you created earlier.

If you plan on running Python unit tests, you will have to repeat the above steps for the **Python tests** default.

Once the default configurations are setup, you can run Python script as follows:

1. Right click anywhere in the editor and choose **Run**

Debugging a Python script
=========================

Once run configurations are setup, debug is essentially another form of running. Note the following:

* You can launch a debug session by right clicking in a script editor, and choosing **Debug**
* You can set break points in a script by clicking in the left margin of the script editor.
