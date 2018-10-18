.. _`Generating C code with Simulink Coder`:

#####################################
Generating C code with Simulink Coder
#####################################

The Vortex DDS Blockset for Simulink supports Simulink Coder generation of C code, if you have a Simulink Coder license from MathWorks.

Prerequisites for C generation
******************************

In order to generate and compile C code containing DDS blocks, you must:

* Have Simulink Coder and MATLAB Coder installed and licensed from MathWorks.
* You have an appropriate C compile installed, as described by the MATLAB documentation.
* Vortex OpenSplice must be installed, and the appropriate ``release.com`` (Linux) or ``release.bat`` (Windows) script must have been executed in a command window.
* MATLAB must have been started from the same command window. You can check this by running the MATLAB command ``getenv('OSPL_HOME')``. It should return a non-empty value.
* Your Simulink mode should execute correctly in simulation mode.

Preparing for C generation
**************************

Once your model has been validated via simulation mode, you are ready to generate and compile code.
Because of an issue with the OpenSplice C99 language headers, you must manualy change the code generation options for your model.
Follow these steps:

- From the model's menu, choose **Code > C/C++ Code > Code Generation Options**.
- Click on the **Code Generation** tab in the left-hand pane.
- In the **Build Configurations** drop-down, choose **Specify**.
- In the table that appears below this, edit the **Options** value in the **C Compiler** row to remove the text **$(ANSI_OPTS)**.
- Click **OK** or **Apply** to save your changes, then close the Code Generation Options dialog.

See the image, below, for an example of the code generation dialog.

.. figure:: images/linuxCodeGenNoANSI_OPTS.png  
    :alt: Removing $(ANSI_OPTS) from C Compiler options
       
    C/C++ Code Generation Options. Remove the text $(ANSI_OPTS) for C Compiler to avoid compile errors.

Generating code
***************

At least from the Vortex DDS Blockset point of view, you are ready to generate code. Follow these steps:

- From the model's menu, choose **Code > C/C++ Code > Build Model**.
- Simulink will get busy. You may see the following warnings in the Diagnostic View. These are OK, but are explained below.

**Domain Participant Warning**

A warning may appear about the domain participant block::

  Source 'SimpleDomain/Domain/Participant_Entity ' specifies that its sample time (-1)
  is back-inherited. You should explicitly specify the sample time of sources. You can
  disable this diagnostic by setting the 'Source block specifies -1 sample time'
  diagnostic to 'none' in the Sample Time group on the Diagnostics pane of the
  Configuration Parameters dialog box.

  Component:Simulink | Category:Blockwarning

As the message states, this is because the block specifies a sample time of -1. The block only creates meaningful output on initialization (it connects to DDS),
so any inherited sample time is sufficient. Specifying a sample time of -1 allows the block to be place into a function-call subblock.

**Full header search warning**

The following warning about reverting to full header searches may appear::

  The following error occurred while attempting to run the preprocessor to find the
  minimum needed set of include files:

  While parsing the source file '<path-to>/source/debug_utils.c' the following error
  occurred

  <path-to>/source/debug_utils.c:14: cannot open source file "os_stdlib.h"
  |  #include "os_stdlib.h"
  |                        ^

  Reverting to full header search.

This may occur as you are trying to package code from compilation on another platform. The referenced header file is part of the OpenSplice distrubution.
When you compile on another platform, you will need to have that platform's OpenSplice distribution installed, and ``release`` variables set. The warning may be ignored.

**Copy File information messages**

If you are creating a source distrubution, you may see information messages such as the following::

  cp: cannot stat ‘/libdcpsc99’: No such file or directory

The build is attempting to copy OpenSplice shared libraries (which are refered to via environment variables). These should not be copied by the build. Instead, when you
compile the source on a target platform, these libraries will be found in the local OpenSplice installation.

Running built models
********************

When you run a compiled Simulink executable, you will need:

* An appropriate OpenSplice runtime installation on the machine executing the model
* The correct OpenSplice environment variables, which are set by the ``release`` script in the installation root directory.



