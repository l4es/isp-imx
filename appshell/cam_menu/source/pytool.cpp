/******************************************************************************\
|* Copyright (c) 2020 by VeriSilicon Holdings Co., Ltd. ("VeriSilicon")       *|
|* All Rights Reserved.                                                       *|
|*                                                                            *|
|* The material in this file is confidential and contains trade secrets of    *|
|* of VeriSilicon.  This is proprietary information owned or licensed by      *|
|* VeriSilicon.  No part of this work may be disclosed, reproduced, copied,   *|
|* transmitted, or used in any way for any purpose, without the express       *|
|* written permission of VeriSilicon.                                         *|
|*                                                                            *|
\******************************************************************************/

#include "pytool.hpp"


static PyObject* g_pName = NULL;
static PyObject* g_pModule =NULL;
static PyObject* g_pDict = NULL;
static int py_initialized = 0;

int py_init(void)
{
    int ret = 0;
    int sensor_init = 0;

	if( py_initialized == 0 )
    {
        Py_Initialize();
        if (!Py_IsInitialized())
        {
            ret = -1;
            return ret;
        }
        PyRun_SimpleString("import sys");
        PyRun_SimpleString("sys.path.append('./')");
        PyRun_SimpleString("print sys.path");
    
        
        g_pName = PyString_FromString("py_shell_tool");
        g_pModule = PyImport_Import(g_pName);
        if (!g_pModule)
        {
            printf("Py_init: Can't find py_shell_tool.py\n");
            ret = -1;
            return ret;
        }
    
        g_pDict = PyModule_GetDict(g_pModule);
        if (!g_pDict)
        {
            printf("Can't find Dict\n");
            ret = -1;
            return ret;
        }
    
        if(g_pName) Py_INCREF(g_pName);
        if(g_pModule) Py_INCREF(g_pModule);
        if(g_pDict) Py_INCREF(g_pDict);
        py_initialized = 1;
    }

    printf("py init done\n");
    return ret;
}

void py_deinit(void)
{
    if(g_pDict) Py_DECREF(g_pDict);
    if(g_pModule) Py_DECREF(g_pModule);
    if(g_pName) Py_DECREF(g_pName);

    Py_Finalize();
    py_initialized = 0;
}


unsigned int nwl_csi_i2c_write(int bus_id, uint16_t slave_addr, unsigned int address, unsigned int data)
{
    unsigned int ret_val;
    unsigned int ret;
    PyObject* pFunc = NULL;
    PyObject* pArgs = NULL;
    PyObject* pRet = NULL;

    const char * pfunc_name = "nwl_csi_i2c_write";
    pFunc = PyDict_GetItemString(g_pDict, pfunc_name);
    if (!pFunc || !PyCallable_Check(pFunc))
    {
        printf("Can't find functon:%s\n", pfunc_name);
        ret = -1;
        return ret;
    }

    pArgs = Py_BuildValue("iiii", bus_id, slave_addr<<1, address, data);

    pRet = PyEval_CallObject(pFunc, pArgs);

    if(pRet == NULL)
    {
        printf("%s: pRet get null\n", __func__);
        return -1;
    }

    ret = PyArg_Parse(pRet,"i", &ret_val);

    return ret_val;

}




unsigned int nwl_csi_i2c_read(int bus_id, uint16_t slave_addr, unsigned int address)
{
    unsigned int ret_val;
    unsigned int ret;
    PyObject* pFunc = NULL;
    PyObject* pArgs = NULL;
    PyObject* pRet = NULL;

    //unsigned char * pfunc_name = "nwl_csi_i2c_read";
    const char * pfunc_name = "nwl_csi_i2c_read";
    pFunc = PyDict_GetItemString(g_pDict, pfunc_name);
    if (!pFunc || !PyCallable_Check(pFunc))
    {
        printf("Can't find functon:%s\n", pfunc_name);
        ret = -1;
        return ret;
    }

    pArgs = Py_BuildValue("iii", bus_id, slave_addr<<1, address);

    pRet = PyEval_CallObject(pFunc, pArgs);

    if(pRet == NULL)
    {
        printf("%s: pRet get null\n", __func__);
        return -1;
    }

    ret_val = 0x60;
    ret = PyArg_Parse(pRet,"i", &ret_val);
    
    printf("%s: recv data 0x%x, ret:%d\n", __func__,  ret_val, ret);
    return ret_val;

}



unsigned int cam_app_la(int arg)
{
    unsigned int ret_val;
    unsigned int ret;
    PyObject* pFunc = NULL;
    PyObject* pArgs = NULL;
    PyObject* pRet = NULL;

    const char * pfunc_name = __func__;
    printf("%s: open func name: %s\n", __func__, pfunc_name);

    pFunc = PyDict_GetItemString(g_pDict, pfunc_name);
    if (!pFunc || !PyCallable_Check(pFunc))
    {
        printf("Can't find functon:%s\n", pfunc_name);
        ret = -1;
        return ret;
    }

    pArgs = PyTuple_New(1);
    PyTuple_SetItem(pArgs, 0, Py_BuildValue("i", arg));

    pRet = PyEval_CallObject(pFunc, pArgs);

    if(pRet == NULL)
    {
        printf("%s: pRet get null\n", __func__);
        return -1;
    }

    ret_val = 0x60;
    ret = PyArg_Parse(pRet,"i", &ret_val);
    
    printf("%s: recv data %d, ret:%d\n", __func__,  ret_val, ret);
    return ret_val;

}

