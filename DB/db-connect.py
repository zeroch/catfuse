#!/usr/bin/python
import MySQLdb
import socket
import thread

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

def clientHandler(clientSock,clientAddr):
	try:
		query = clientSock.recv(1024).split(",")
	except:
		print "Invalid Query" 

	if int(query[0]) == 0: # POST
		msg = dbPOST(query[1],int(query[2]),query[3])
	elif int(query[0]) == 1: # GET
		msg = dbGET(query[1])	
	else:
		msg = "Invalid query"

	clientSock.send(msg)
	clientSock.close()

def resetDB():
	con = MySQLdb.connect('localhost','catfuser','catfuser','testdb')
	with con:
		cur = con.cursor()
		cur.execute("DROP TABLE IF EXISTS PhotoObjects")
		cur.execute("CREATE TABLE PhotoObjects(ObjID VARCHAR(255),Version INT,PathHash VARCHAR(255))")
		#cur.execute("INSERT INTO PhotoObjects Values(\'host\',1,\'hash\') ")

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
