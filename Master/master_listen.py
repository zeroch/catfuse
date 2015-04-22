import socket
import thread
import paramiko
import scp

def sendFileToReplica(query):
    print query
    query_list = query.split(',')
    ip = query_list[0]
    file_list = query_list[1:]
    
    ssh = paramiko.SSHClient()

    ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    ssh.connect(ip,username='ubuntu',password='',key_filename='/home/ubuntu/.ssh/'+ip+'.pem')

    scp = scp.SCPClient(ssh.get_transport())
    
    for file in file_list:
        scp.put('/home/ubuntu/fuser/'+file,'')

    ssh.close()


    return "Send OK"

def clientHandler(clientSock,clientAddr):
    try:
        query = clientSock.recv(1024)
    except:
        print "Invalid Query" 

    msg = sendFileToReplica(query)

    clientSock.send(msg)
    clientSock.close()

def main():
    host = 'localhost'
    port = 12346
    buf = 1024

    servSock = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
    servSock.bind((host,port))
    servSock.listen(10)

    print "*************************\nMaster Listening\n*************************"

    while True:
        clientSock,clientAddr = servSock.accept()
        thread.start_new_thread(clientHandler,(clientSock,clientAddr))
    servSock.close()

if __name__ == "__main__":
    main()
