#!/usr/bin/python
import sys
import subprocess

handle_r=subprocess.Popen("ps -auxf|grep gui", stdout=subprocess.PIPE, shell=True)
data_str = handle_r.stdout.read()

process_lst = data_str.split('\n')

for idx in process_lst:
	if idx.find('gui_appl') != -1:
		gui_process = idx.split()
		pid = gui_process[1]
		print("find %s, id: %s\n"%('gui_appl', pid))
		kill_cmd=subprocess.Popen("kill -9 %s"%pid, stdout=subprocess.PIPE, shell=True)
		retmsg = kill_cmd.stdout.read()
		print("kill ret: %s\n"%(retmsg))



