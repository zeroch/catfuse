#!/usr/bin/python
import MySQLdb
import socket
import thread
import hashlib
import time

MAX_REPLICA = 6 #The maximum number of Replica Nodes here
REPLICA_COUNT = MAX_REPLICA #Keep track of how many replica can still register until table is full
REPLICA_PER_FILE = MAX_REPLICA / 2 #Number of Replica to upload a file to
RANDOM_NUMBER = 4 #Used to randomly determine ReplicaID to upload a file to in whichReplicaID Function

#static socket to connect with master
masterSock = None

def updateTable(objID,Version):
	con = MySQLdb.connect('localhost','catfuser','catfuser','testdb')
	with con:
		cur = con.cursor()
		cur.execute("DELETE FROM PhotoObjects WHERE ObjID = \'%s\' AND Version < %s"%(objID,Version))

def whichReplicaID(objHash):
	#Under the assumption that six replicas have registered with the database, this function takes the objHash
	#and perform calculations to determine what replicas to upload the objHash to.
	global REPLICA_PER_FILE,RANDOM_NUMBER,MAX_REPLICA
	value = 0
	replicaList = list()

	for i in range(0,RANDOM_NUMBER):
		value = ord(objHash[i])

	for i in range(0,REPLICA_PER_FILE):
		a = (value%MAX_REPLICA)+i
		if a > MAX_REPLICA:
			a = a - MAX_REPLICA
		if a == 0: a = a + MAX_REPLICA
		replicaList.append(a)

	return replicaList

def dbPOST(objID,Version,objHash):
	updateTable(objID,Version)
	replicaList = whichReplicaID(objHash)
	#msg = ""
	#for replicaID in replicaList:
	#	msg = msg + "%s,"%replicaID
	#msg = msg[:-1]
	
	try:
		con = MySQLdb.connect('localhost','catfuser','catfuser','testdb')
		with con:
			cur = con.cursor()
			cur.execute("INSERT INTO PhotoObjects VALUES(\'%s\',%s,\'%s\')"%(objID,Version,objHash))
			for replicaID in replicaList:
				cur.execute("INSERT INTO DistFile VALUES(\'%s\',%s)" %(objHash,replicaID))
	except:
		return "POST_ERROR"

	return "POST_OK"
'''
def dbGET(objID):

	try:
		con = MySQLdb.connect('localhost','catfuser','catfuser','testdb')
		with con:
			cur = con.cursor()
			cur.execute("SELECT PathHash FROM PhotoObjects WHERE ObjID = \"%s\" ORDER BY Version DESC LIMIT 1"%objID)
			results = cur.fetchall()
	except:
		return "GET_ERROR"
	return results[0][0]
'''

def dbGET(objIDList):
	msg = ""
	objList = objIDList.split(":")
	try:
		con = MySQLdb.connect('localhost','catfuser','catfuser','testdb')
		with con:
			cur = con.cursor()
			for  objID in objList:
				cur.execute("SELECT PathHash FROM PhotoObjects WHERE ObjID = \"%s\" ORDER BY Version DESC LIMIT 1"%objID)
				objHashrow = cur.fetchall()

				cur.execute("SELECT ReplicaID FROM DistFile WHERE PathHash = \"%s\""% objHashrow[0][0])
				replicaIDrow = cur.fetchall()
				
				cur.execute("SELECT ReplicaIP,ReplicaPORT FROM  Replica WHERE ReplicaID = %s" % replicaIDrow[0][0])
				replicaIP_PORTrow = cur.fetchall()
				print replicaIP_PORTrow[0][0]
				print replicaIP_PORTrow[0][1]
				msg = msg + "%s:%s:%s:%s," % (objID,replicaIDrow[0][0],replicaIP_PORTrow[0][0],replicaIP_PORTrow[0][1])


	except:
		return "GET_ERROR"
	return msg[:-1]

def dbDELETE(objID):
	try:
		con = MySQLdb.connect('localhost','catfuser','catfuser','testdb')
		with con:
			cur = con.cursor()
			cur.execute("DELETE FROM PhotoObjects WHERE ObjID = \"%s\""%objID)
	except:
		return "DELETE_ERROR"
	return "DELETE_OK"

def dbLIST(clientSock,clientAddr,replicaID):
	replicaIP = clientAddr[0]
	replicaPORT = int(clientAddr[1])
	msg = ""
	try:
		con = MySQLdb.connect('localhost','catfuser','catfuser','testdb')
		with con:
			cur = con.cursor()
			#print replicaIP, replicaPORT
			#cur.execute("SELECT ReplicaID FROM Replica WHERE ReplicaIP = \'%s\'" % (replicaIP))
			#print "replica connected"
			#print "replicaID from %s : %s is %s" %(replicaIP,replicaPORT,cur.fetchall()[0][0])
			#print cur.fetchall()
			#replicaID = cur.fetchall()[0][0]

			#print "replicaID is %s" % replicaID
			cur.execute("SELECT PathHash FROM DistFile WHERE ReplicaID = %s" % replicaID)
			rows = cur.fetchall()
			for row in rows:
				cur.execute("SELECT * FROM PhotoObjects WHERE PathHash = \'%s\'" %row[0])
				subrow = cur.fetchall()
				print "subrow = %s" % subrow
				print subrow[0][0]
				print subrow[0][1]
				print subrow[0][2]
				msg = msg + "(%s,%s,%s)" % (subrow[0][0],subrow[0][1],subrow[0][2])
	except:
		return "LIST_ERROR"
	return msg			


def dbREQ(requestFile,clientSock,clientAddr):
	requestList = requestFile.split(":")
	strReq = ""

	for reqfile in requestList:
		strReq = strReq + "," + reqfile
		
	reqMSG = clientAddr[0] + strReq
	thread.start_new_thread(contactMasterNode,(reqMSG,))

	return "REQUEST_OK"

def contactMasterNode(reqMSG):
	global masterSock
	while True:
		try:
			masterSock.send(reqMSG)
			recvdata = masterSock.recv(buf)
			if not recvdata:
				break
			else:
				print recvdata
				break
		except socket.error,e:
			if "Connection refused" in e:
				print "*** Connection Refused ***"
			break



def regREP(clientSock,clientAddr):
	global REPLICA_COUNT
	print "Receive replica connection request from %s : %s \n" % (clientAddr[0],clientAddr[1])
	replicaIP = clientAddr[0]
	replicaPORT = int(clientAddr[1])

	if REPLICA_COUNT != 0:
		replicaID = REPLICA_COUNT
		REPLICA_COUNT = REPLICA_COUNT - 1

		try:
			con = MySQLdb.connect('localhost','catfuser','catfuser','testdb')
			with con:
				cur = con.cursor()
				print "Registering Replica%s IP:%s PORT:%s" % (replicaID,replicaIP,replicaPORT)
				cur.execute("INSERT INTO Replica VALUES(%s,\'%s\',%s)"%(replicaID,replicaIP,replicaPORT))
			msg = "%s" % replicaID
			
		except:
			msg = "REGISTER_ERROR"
	else:
		print "REPLICA TABLE FULL: Cannot provide ID for connection request from %s : %s \n" % (clientAddr[0],clientAddr[1])
		msg = "REPLICA_TABLE_FULL"
	return msg

def masterLIST():
	time.sleep(1)
	master = "localhost"
	port = 3000
	buf = 1234
	global masterSock
	masterSock = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
	masterSock.connect((master,port))
	while True:
		try:
			masterSock.send("Test Connect")
			recvdata = masterSock.recv(buf)
			if recvdata:
				print recvdata
			if recvdata == "Test Connect":
				break
		except socket.error,e:
			if "Connection refused" in e:
				print "--- Connection Refused ---"
				break



	msg = ""
	try:
		con = MySQLdb.connect('localhost','catfuser','catfuser','testdb')
		with con:
			cur = con.cursor()
			cur.execute("SELECT * FROM PhotoObjects")
			filerow = cur.fetchall()
			for row in filerow:
				msg = msg + "(%s,%s,%s)" % (row[0],row[1],row[2])
	except:
		return

	try:
		masterSock.send(msg)
	except socket.error,e:
		if "Connection refused" in e:
			print "--- Connection Refused ---"
	


	


def clientHandler(clientSock,clientAddr):
	try:
		query = clientSock.recv(1024).split(",")
	except:
		print "Invalid Query" 

	if int(query[0]) == 0: # POST
		msg = dbPOST(query[1],int(query[2]),query[3])

	elif int(query[0]) == 1: # GET
		msg = dbGET(query[1])

	elif int(query[0]) == 2: # DELETE
		msg = dbDELETE(query[1])

	elif int(query[0]) == 3: # LIST
		replicaID = query[1]
		msg = dbLIST(clientSock,clientAddr,replicaID)
		if len(msg) == 0:
			msg = "EMPTY"

	elif int(query[0]) == 4: # REQUEST_FILE
		msg = dbREQ(query[1],clientSock,clientAddr)

	elif int(query[0]) == 5: # REGISTER REPLICA 
		msg = regREP(clientSock,clientAddr)

	elif int(query[0]) == 6: # MASTER LIST
		msg = "Dummy Start"

	else:
		msg = "Invalid Query"

	clientSock.send(msg)
	clientSock.close()
	
	if int(query[0]) == 6:
		masterLIST()


def resetDB():
	#m = hashlib.md5()
	#m.update("hashcode")
	#print "******\n"+str(m.hexdigest())+"\n******\n"
	con = MySQLdb.connect('localhost','catfuser','catfuser','testdb')
	with con:
		cur = con.cursor()
		cur.execute("DROP TABLE IF EXISTS PhotoObjects")
		cur.execute("DROP TABLE IF EXISTS Replica")
		cur.execute("DROP TABLE IF EXISTS DistFile")
		cur.execute("CREATE TABLE PhotoObjects(ObjID VARCHAR(255),Version INT,PathHash CHAR(32))")
		cur.execute("CREATE TABLE Replica(ReplicaID INT, ReplicaIP VARCHAR(255), ReplicaPORT INT )")
		cur.execute("CREATE TABLE DistFile(PathHash CHAR(32),ReplicaID INT)")

		#cur.execute("INSERT INTO PhotoObjects Values(\'host\',1,\'%s\') " %(str(m.hexdigest())))
		#cur.execute("INSERT INTO PhotoObjects Values(\'A\',1,\'A\')");
		#cur.execute("INSERT INTO PhotoObjects Values(\'B\',1,\'B\')");
		#cur.execute("INSERT INTO PhotoObjects Values(\'C\',1,\'C\')");

		#cur.execute("SELECT * FROM PhotoObjects")
		#msg = ""
		#rows = cur.fetchall()
		#for row in rows:
		#	msg = msg + "(%s,%s,%s)" % (row[0],row[1],row[2])
	#print msg

def main():
	resetDB()
	host = 'localhost'
	port = 12345
	buf = 1024

	servSock = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
	servSock.bind((host,port))
	servSock.listen(10)

	print "*************************\nDATABASE SERVICE RUNNING\n*************************"

	while True:
		clientSock,clientAddr = servSock.accept()
		thread.start_new_thread(clientHandler,(clientSock,clientAddr))
	servSock.close()

if __name__ == "__main__":
	main()
