A.	Implementation Choice
		1.	(EXTRA CREDIT) Concurrent Games with Interruption

B.	Test Plan: 
		1.	Please refer to requirements.txt for all project requirements that were tested, as well as how our
		    test suite (test_suite) is structured.
		2. 	Note that for every test case in test_suite, there is a doc.txt file that provides additional details about:
				a.	the test case
				b.	the scripted client messages
				c.	how the server passes the test case
		3.	You can call ./ttts [PORT] in order to start the server before executing the test suite in a seperate terminal.
		4.	You can call ./test [HOST] [PORT] in order to execute the test suite. The test suite connects to the ttts 
			server, so make sure the host and port you provide to the test suite is of the ttts server.
		5.	In order to terminate the server, you must kill the terminal window for the server. 
		6.	In order to terminate the test suite, you must use CTRL-C to send a SIGINT. There are a total of 12 clients to
			wait for before you can terminate the test suite. They will be finished in approximately 30 seconds or less.
		7.	We implemented the test suite this way because we wanted to wait for all the game threads to finish. 


C.	Use of Locks
		1.	Create a mutex lock for the set of related shared resources, in this case {number_of_players, player_names}
		2.	Whenever we do a read/write operation on variables number_of_players and player_names, 
				a.	We obtain the lock
				b. 	We release the lock after we finish
