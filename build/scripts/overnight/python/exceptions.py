""" Exceptions module

Defines the exceptions used 
"""

class LogCheckFail(Exception):

    """ Indicate a scenario failure.
    """

    def __init__(self, reason):

        """ Constructor

        Parameters:

           reason: string
              the reason for the failure.
        """
        Exception.__init__(self, reason)


class MissingExecutable(Exception):

    """ Indicate a scenario failure.
    """

    def __init__(self, reason):

        """ Constructor

        Parameters:

           reason: string
              the reason for the failure.
        """
        Exception.__init__(self, reason)
