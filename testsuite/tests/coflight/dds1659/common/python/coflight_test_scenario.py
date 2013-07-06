from base_test_scenario import BaseTestScenario
#===============================================================================
class CoFlightTestScenario(BaseTestScenario):
    """
        Represents a single test CoFlight scenario within a test.
    """
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def __init__(self, name = "", description = "", log_root = ".", q_steps = 0):
        """Constructs a test scenario."""
        BaseTestScenario.__init__(self, name, description, log_root)
        # Quantity of test scenario steps 
        self.q_steps = q_steps
    #gettin operation key for process    
    def get_quantity_of_steps(self):
        """
        getting quantity of test scenario steps
        """
        return self.q_steps
    #gettin operation key for process    
    def set_quantity_of_steps(self, q_steps):
        """
        setting quantity of test scenario steps
        """
        self.q_steps = q_steps
    
#===============================================================================