/******************************************************************************\
|* Copyright 2010, Dream Chip Technologies GmbH. used with permission by      *|
|* VeriSilicon.                                                               *|
|* Copyright (c) <2020> by VeriSilicon Holdings Co., Ltd. ("VeriSilicon")     *|
|* All Rights Reserved.                                                       *|
|*                                                                            *|
|* The material in this file is confidential and contains trade secrets of    *|
|* of VeriSilicon.  This is proprietary information owned or licensed by      *|
|* VeriSilicon.  No part of this work may be disclosed, reproduced, copied,   *|
|* transmitted, or used in any way for any purpose, without the express       *|
|* written permission of VeriSilicon.                                         *|
|*                                                                            *|
\******************************************************************************/

/* VeriSilicon 2020 */

/******************************************************************************
 *
 * Original author:
 * Dream Chip Technologies GmbH
 *
 * Additional work by:
 * Liu Su <su.liu@verisilicon.com>
 *
 *****************************************************************************/
/**
 * @file xom_ctrl_common.h
 *
 * @brief
 *   Common stuff used by XOM ctrl API & implementation.
 *
 *****************************************************************************/

#ifndef __XOM_CTRL_COMMON_H__
#define __XOM_CTRL_COMMON_H__

#ifdef __cplusplus
extern "C"
{
#endif

/**
 *  @brief External buffer callback
 *
 *  Callback for external functions. Called when new buffer available.
 *
 */
typedef RESULT (* xomCtrlBufferCb_t)
(
    PicBufMetaData_t    *pBuffer,
    void                *pContext   //!< Opaque sample data pointer that was passed in on xom control creation.
);

/**
 * @brief Handle to xom ctrl process context.
 *
 */
typedef struct xomCtrlContext_s *xomCtrlHandle_t;

/**
 * @brief IDs of supported commands.
 *
 */
typedef enum xomCtrl_command_e
{
    XOM_CTRL_CMD_START          = 0,
    XOM_CTRL_CMD_STOP           = 1,
    XOM_CTRL_CMD_PAUSE          = 2,
    XOM_CTRL_CMD_RESUME         = 3,
    XOM_CTRL_CMD_SHUTDOWN       = 4,
    XOM_CTRL_CMD_PROCESS_BUFFER = 5
} xomCtrlCmdID_t;

/**
 * @brief Data type used for commands (@ref xomCtrl_command_e).
 *
 */
typedef struct xomCtrlCmd_s
{
    xomCtrlCmdID_t CmdID;   //!< The command to execute.
    union
    {
        struct
        {
            xomCtrlBufferCb_t   xomBufferCb;    //!< External buffer callback
            void                *pContext;//!< Sample context passed on to sample callback.
            uint8_t             FrameSkip;     //!< Skip consecutive samples
        } Start;                //!< Params structure for @ref XOM_CTRL_CMD_START.
    } Params;               //!< Params of the command to execute.
} xomCtrlCmd_t;

/**
 *  @brief Command completion signalling callback
 *
 *  Callback for signalling completion of commands which could require further application interaction.
 *
 */
typedef void (* xomCtrlCompletionCb_t)
(
    xomCtrlCmdID_t  CmdId,          //!< The type of command which was completed (see @ref xomCtrl_command_e).
    RESULT          result,         //!< Result of the executed command.
    void            *pUserContext   //!< Opaque user data pointer that was passed in on xom control creation.
);

/* @} xom_ctrl_common */

#ifdef __cplusplus
}
#endif

#endif /* __XOM_CTRL_COMMON_H__ */
