import socket
import time

def sendPost(objID,version,objHash):
	return "0,%s,%s,%s" % (objID,version,objHash)
def sendGet(objID):
	return "1,%s" % objID
def sendDelete(objID):
	return "2,%s" % objID
def sendList():
	return "3"
def sendRequestFile(file1,file2):
	return "4,%s:%s" % (file1,file2)
def sendRegisterReplica():
	return "5"
def replica1():
	pass
def replica2():
	pass
def replica3():
	pass
def replica4():
	pass
def replica5():
	pass
def replica6():
	pass


if __name__ == '__main__':
	host = "localhost"
	port = 12345
	buf = 1234

	#clientSock = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
	#clientSock.connect((host,port))
	#data = ("0,objecthash1,1,version1","0,objecthash2,1,version1","0,objecthash1,2,version2","0,objecthash1,1,version1","0,objecthash1,3,version3")
	'''
	data = ("5", #Register 1st Replica
		    "5", #Register 2nd Replica
		    "5", #3rd
		    "5", #4th
		    "5", #5th
		    "5", #6th
		    "0,ObjectA,1,V1_AObject", #POST Obj1
		    "0,ObjectB,1,V1_BObject", #POST Obj2
		    "0,ObjectC,1,V1_CObject") #POST Obj3
	'''
	data = ("1,ObjectA:ObjectB",)
	SOURCE_PORT = 38524#38520#38525
	for tosend in data:
		clientSock = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
		clientSock.bind((host,SOURCE_PORT))
		clientSock.connect((host,port))
		while True:
			#print "sending" + tosend
			clientSock.send(tosend)
			recvdata = clientSock.recv(buf)
			if not recvdata:
				break
			else:
				print recvdata
		clientSock.close()
	'''	
	data2 = ("5","0,ObjectA,1,V1_AObject","3")#0,ObjectB,1,V1_BObject","0,ObjectC,1,V1_CObject","3")
	clientSock = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
	clientSock.connect((host,port))
	for tosend in data2:
		print "sending %s" % tosend
		clientSock.send(tosend)
		recvdata = clientSock.recv(buf)
		print recvdata
		time.sleep(10)
	'''
