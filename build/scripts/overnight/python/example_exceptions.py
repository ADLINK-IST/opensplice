""" Exceptions module

Defines the exceptions used 
"""

class LogCheckFail(RuntimeError):

    """ Indicate a scenario failure.
    """

    def __init__(self, reason):

        """ Constructor

        Parameters:

           reason: string
              the reason for the failure.
        """
        RuntimeError.__init__(self, reason)

class MissingExecutable(RuntimeError):

    """ Indicate a scenario failure.
    """

    def __init__(self, reason):

        """ Constructor

        Parameters:

           reason: string
              the reason for the failure.
        """
        RuntimeError.__init__(self, reason)

class ExampleFail(RuntimeError):

    """ Indicate a scenario failure.
    """

    def __init__(self, reason):

        """ Constructor

        Parameters:

           reason: string
              the reason for the failure.
        """
        RuntimeError.__init__(self, reason)
