# DBSI

The aim is to implement a pointer free structure with an array of keys at each level. 
The program is invoked in the following manner, 

build K P fanout0, fanout1....
where K is the number of keys that need to be inserted in the structure, P is the number of probes, fanout0 is the fanout of root node or level 0, fanout1 is fanout of level 1 and so on till the leaf level. 

The random fucntion from p2random.h library is generates N number of keys in random and pushes them to a[].   

Inserting the keys: 
The insert fucntion passes the number of levels and fanout of each level as a parameter and calls insert_element() which recursviely distributes the keys into each level.


Probing: 

The search function iterates over every level from root to leaf calling bs() for each level passing the key, the current level, the start index and end index to search in the current level. 
bs() uses recursion to retrieve the right index in the right node which is then interpreted into an appropriate range. 
The start_index and end index for each level i is calculated using the math below and then passed to the bs fucntion which returns as result the node to search for in the next level.  
start_index = result*fanout[i]
end_index = start_index + fanout[i] -2 



