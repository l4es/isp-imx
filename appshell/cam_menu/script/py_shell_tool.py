#!/usr/bin/python
import sys
import subprocess

#input address, data in int, return int
def nwl_csi_i2c_read(bus_id, slave_addr, address):
    handle_r = subprocess.Popen("./i2c_read.rb %s %s %s"%(hex(bus_id), hex(slave_addr), hex(address)), stdout=subprocess.PIPE, shell=True)
    #handle_r = subprocess.Popen("./i2c_read.rb %s"%(hex(address)), stdout=subprocess.PIPE, shell=True)
    data_str = handle_r.stdout.read()
    #print data_str
    #return 5
    return int(data_str)

def nwl_csi_i2c_test(address):
    handle_r=subprocess.Popen("./ls", stdout=subprocess.PIPE, shell=True)
    data_str = handle_r.stdout.read()
    return 0x68

def cam_app_la(arg):
    handle_r=subprocess.Popen("./cam_app %s"%(hex(arg)), shell=True)
    data_str = handle_r.stdout.read()
    return 0

#input address, data in int, return int
def nwl_csi_i2c_write(bus_id, slave_addr, address, data):
    handle_w=subprocess.Popen("./i2c_write.rb %s %s %s %s"%(hex(bus_id), hex(slave_addr), hex(address), hex(data)), stdout=subprocess.PIPE, shell=True)
    ret_str = handle_w.stdout.read()
    #print ret_str
    return int(ret_str)

def test_func():
    print sys.argv
    arg_len = len(sys.argv)
    print "mode: 0 for read, 1 for write"
    cam_app_la(0)
    mode = sys.argv[1] if arg_len > 1 else '0'
    address_str = sys.argv[2] if arg_len > 2 else '0x3502'
    data_str = sys.argv[3] if arg_len > 3 else '0x20'
    #print "%s,%s,%s"%(mode, address_str, data_str)

    address = int(address_str, 16)
    data = int(data_str, 16)

    if mode == '0':
        readdata = nwl_csi_i2c_read(0, 0x6c, address)
        print "read: adddress 0x%x, data:0x%x"%(address, readdata)
    else:
        print "write: address 0x%x, data:0x%x"%(address, data)
        ret = nwl_csi_i2c_write(0, 0x6c, address, data)
        print "write return:%d"%ret

    nwl_bus_init_settings(0, 1)

if __name__ == "__main__":
    ret=cam_app_la(2)
    print(ret)


