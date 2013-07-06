#===============================================================================
class TestError(Exception):
    """A simple exception for a test problems."""
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def __init__(self, cause = "unknown"):
        """Constructs the object."""
        self.cause = cause
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def __str__(self):
        """String object representation."""
        return self.cause
#===============================================================================