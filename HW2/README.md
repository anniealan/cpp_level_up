### Test Automation Framework
 

#### Implement an automated test execution system, which will allow test functions to be executed in separate processes. The child process output should be redirected either to a file, or to the parent process (for further processing if necessary) based on program parameters.

#### Test functions can be almost empty - returning true or false values or making the test to abort. All the case must be handled and reported by the parent process (Success, Failure, Unresolved).

#### As a result a report must be generated where all the test execution information, all the output and results must be represented in a user-readable form.

#### Code must be implemented using OOP approach wherever possible. Child process redirection must be implemented using either files, or pipes.