#!/usr/bin/python
import MySQLdb
import socket
import thread
import hashlib

def updateTable(objID,Version):
	con = MySQLdb.connect('localhost','catfuser','catfuser','testdb')
	with con:
		cur = con.cursor()
		cur.execute("DELETE FROM PhotoObjects WHERE ObjID = \'%s\' AND Version < %s"%(objID,Version))
		 

def dbPOST(objID,Version,objHash):
	updateTable(objID,Version)	
	try:
		con = MySQLdb.connect('localhost','catfuser','catfuser','testdb')
		with con:
			cur = con.cursor()
			cur.execute("INSERT INTO PhotoObjects VALUES(\'%s\',%s,\'%s\')"%(objID,Version,objHash))
	except:
		return "POST_ERROR"
	return "POST_OK"

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

def dbDELETE(objID):
	try:
		con = MySQLdb.connect('localhost','catfuser','catfuser','testdb')
		with con:
			cur = con.cursor()
			cur.execute("DELETE FROM PhotoObjects WHERE ObjID = \"%s\""%objID)
	except:
		return "DELETE_ERROR"
	return "DELETE_OK"

def dbLIST():
	msg = ""
	try:
		con = MySQLdb.connect('localhost','catfuser','catfuser','testdb')
		with con:
			cur = con.cursor()
			cur.execute("SELECT * FROM PhotoObjects")
			rows = cur.fetchall()
			for row in rows:
				msg = msg + "(%s,%s,%s)" % (row[0],row[1],row[2])
	except:
		return "LIST_ERROR"
	return msg			

<<<<<<< HEAD
=======
def dbREQ(requestFile,clientSock,clientAddr):
	requestList = requestFile.split(":")
	strReq = ""

	for reqfile in requestList:
		strReq = strReq + "," + reqfile
		
	reqMSG = clientAddr[0] + strReq
	thread.start_new_thread(contactMasterNode,(reqMSG,))

	return "REQUEST_OK"

def contactMasterNode(reqMSG):
	master = "localhost"
	port = 12346
	buf = 1234

	clientSock = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
	clientSock.connect((master,port))

	while True:
		try:
			clientSock.send(reqMSG)
			recvdata = clientSock.recv(buf)
			if not recvdata:
				break
			else:
				print recvdata
		except socket.error,e:
			print "Cannot contact the master node"
			break

	clientSock.close()

>>>>>>> ec63cb4a15de600ef29ad60912e2a955d9ade4e0
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
		msg = dbLIST()
		if len(msg) == 0:
<<<<<<< HEAD
			msg = "EMPTY" 	
=======
			msg = "EMPTY"
	elif int(query[0]) == 4: # REQUEST_FILE
		msg = dbREQ(query[1],clientSock,clientAddr) 	
>>>>>>> ec63cb4a15de600ef29ad60912e2a955d9ade4e0
	else:
		msg = "Invalid Query"

	clientSock.send(msg)
	clientSock.close()

def resetDB():
	#m = hashlib.md5()
	#m.update("hashcode")
	#print "******\n"+str(m.hexdigest())+"\n******\n"
	con = MySQLdb.connect('localhost','catfuser','catfuser','testdb')
	with con:
		cur = con.cursor()
		cur.execute("DROP TABLE IF EXISTS PhotoObjects")
		cur.execute("CREATE TABLE PhotoObjects(ObjID VARCHAR(255),Version INT,PathHash CHAR(32))")
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
