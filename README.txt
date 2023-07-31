Yash Patel - ypp10
Jasmit Singh - js3034

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

D.  DESIGN NOTES
		1.  If your test cases are different from ours, we want to have this in writing to serve as evidence and proof
			of our design choices (edge cases), which shall not be neglected, as these design choices are not
			explicitly mentioned in the instructions pdf file. Therefore, we reserve the right to make our own design
			choices that must be respected, and shall result in no point deductions, and shall be awarded the full
			points of the project. Failure to do so will result in strict consequences, which may involve a complaint
			to the professor.
		2.	In order to compile all the files of this project, please ensure you are in the project's directory. If you 
			are not, please use the cd command to do so. You can check your current directory by using pwd.	You can run 
			the command "make". When you execute any of the files, make sure to provide the path using ./[file_name]
		3.	Please refer to our design choices in section E.

E.	DESIGN CHOICES (EDGE CASES)
		1.	If a MOVE results in the game ending, we DO NOT send a MOVD message when the game is over. Instead, we send a
			OVER message to both clients directly and end both connections.
		2.	If one of the clients disconnects unexpectedly, we DO NOT send INVL to the other active client. Instead, we
			close both connections without sending any message.
		3.	If client 1 and client 2 send multiple complete messages at the same time, the server's buffers for client 1
			and client 2 would maintain all of them. But, in what order would the server process those complete messages in
			the buffer? Would the server alternate between both clients or simply process all the messages in client 1's
			buffer first followed by client 2's buffer?

				Example:
				Client 1 sends this message: "MOVE|6|X|2,2|DRAW|2|S|MOVE|6|X|1,2|"
				Client 2 sends this message: "MOVE|6|O|1,2|MOVE|6|O|3,1|RSGN|0|"


				Now, the server's buffers look like this:
				client 1 buffer = "MOVE|6|X|2,2|DRAW|2|S|MOVE|6|X|1,2|"
				client 2 buffer = "MOVE|6|O|1,2|MOVE|6|O|3,1|RSGN|0|"



				So at this point, it is not a matter of which message arrives first, as both clients have sent these at the
				same time, so assume that both messages have been received and placed into their respective buffers.

				Now, how shall the server go about choosing which complete message to process first and should it use a
				deterministic algorithm for selection or a randomized one?

				If the server chooses a simple approach of processing all the messages of client 1 first followed by client
				2, then all the messages after the first "MOVE" will be processed as out-of-turn. However, what if the
				client had intended to process their messages in-turn? The server has no way of interpreting when to
				process the messages. Therefore, are we assuming that the server should follow a deterministic approach
				of processing all the messages of client 1 first followed by client 2? Does that seem reasonable and
				acceptable?

				NOTE:	The above question is posted in the discussions on Canvas, however, at the time of this writing it
						has been more than 3 days without getting any response from the professor. Therefore, we reserve
						the right to implement whatever we please.

F.	General Note
		1.	We have implemented all the original requirements in the pdf instructions file. We also understand and have the
			ability to use the systems concepts learned in this class to implement any requirement that you give us.
			Therefore, any of your test cases that fail and are not explicitly defined in the pdf file should have been
			given to us to implement.



