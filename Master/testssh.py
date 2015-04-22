import paramiko
import scp

ssh = paramiko.SSHClient()

ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
ssh.connect('10.0.0.151',username='ubuntu',password='',key_filename='/home/ubuntu/.ssh/testssh.pem')

scp = scp.SCPClient(ssh.get_transport())
scp.put('list.h','/tmp/list.h')

ssh.close()
