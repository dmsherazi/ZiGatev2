/*###############################################################################
#
# MODULE:      BDB
#
# COMPONENT:   bdb_state_machine.c
#
# DESCRIPTION: BDB handling of APP_vGenCallback and bdb_taskBDB
#              
#
###############################################################################
#
# This software is owned by NXP B.V. and/or its supplier and is protected
# under applicable copyright laws. All rights are reserved. We grant You,
# and any third parties, a license to use this software solely and
# exclusively on NXP products [NXP Microcontrollers such as JN514x, JN516x, JN517x].
# You, and any third parties must reproduce the copyright and warranty notice 
# and any other legend of ownership on each  copy or partial copy of the software.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"  
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
# POSSIBILITY OF SUCH DAMAGE. 
# 
# Copyright NXP B.V. 2015-2016. All rights reserved
#
###############################################################################*/

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
#include "jendefs.h"
#include "bdb_api.h"
#include "bdb_start.h"
#if ((defined BDB_SUPPORT_FIND_AND_BIND_INITIATOR) || (defined BDB_SUPPORT_FIND_AND_BIND_TARGET))
#include "bdb_fb_api.h"
#endif
#if (defined BDB_SUPPORT_TOUCHLINK)
#include "bdb_tl.h"
#endif
#if (defined BDB_SUPPORT_NWK_STEERING)
#include "bdb_ns.h"
#endif
#if (defined BDB_SUPPORT_NWK_FORMATION)
#include "bdb_nf.h"
#endif
#if (defined BDB_SUPPORT_OOBC)
#include "bdb_DeviceCommissioning.h"
#endif
#include "dbg.h"
#include "bdb_fr.h"

//FRED
#include "app_common.h"
#include "SerialLink.h"
#include "app_Znc_cmds.h"
#include "zps_struct.h"
//FRED
#include <string.h>
#include <stdlib.h>
/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/
PUBLIC tsBDB sBDB;
extern ZPS_NwkDevice tmpNtActv[200];
extern uint8	u8SizeTmpNtActv;

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/


/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

/****************************************************************************
 *
 * NAME: APP_vGenCallback
 *
 * DESCRIPTION:
 * Event handler called by ZigBee PRO Stack
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void APP_vGenCallback(uint8 u8Endpoint, ZPS_tsAfEvent *psStackEvent)
{
    BDB_tsZpsAfEvent sZpsAfEvent;
	uint8     				u8LinkQuality;
	u8LinkQuality=psStackEvent->uEvent.sApsDataIndEvent.u8LinkQuality;

//manage src
	uint64 srcIEEE = ZPS_u64NwkNibFindExtAddr((void *)ZPS_pvNwkGetHandle(), psStackEvent->uEvent.sApsDataIndEvent.uSrcAddress.u16Addr);
	if ((srcIEEE !=0) && (psStackEvent->uEvent.sApsDataIndEvent.uSrcAddress.u16Addr!=0) )
	{
		if (APP_ExistDevice(srcIEEE))
		{
			uint8 tmp = APP_GetIndexDevice(srcIEEE);
			tmpNtActv[tmp].u8LinkQuality = u8LinkQuality;

		}else{
			tmpNtActv[u8SizeTmpNtActv].u16ShortAddr=psStackEvent->uEvent.sApsDataIndEvent.uSrcAddress.u16Addr;
			tmpNtActv[u8SizeTmpNtActv].u64IEEEAddr=srcIEEE;
			tmpNtActv[u8SizeTmpNtActv].u8LinkQuality=u8LinkQuality;
			tmpNtActv[u8SizeTmpNtActv].u8Type=2;
			u8SizeTmpNtActv++;
		}
	}
//manage dst
	uint64 dstIEEE = ZPS_u64NwkNibFindExtAddr((void *)ZPS_pvNwkGetHandle(), psStackEvent->uEvent.sApsDataIndEvent.uDstAddress.u16Addr);

	if (dstIEEE!=0 && (psStackEvent->uEvent.sApsDataIndEvent.uDstAddress.u16Addr!=0) )
	{
		if (APP_ExistDevice(dstIEEE))
		{
			uint8 tmp = APP_GetIndexDevice(dstIEEE);
			tmpNtActv[tmp].u8LinkQuality = u8LinkQuality;

		}else{
			tmpNtActv[u8SizeTmpNtActv].u16ShortAddr=psStackEvent->uEvent.sApsDataIndEvent.uDstAddress.u16Addr;
			tmpNtActv[u8SizeTmpNtActv].u64IEEEAddr=dstIEEE;
			tmpNtActv[u8SizeTmpNtActv].u8LinkQuality=u8LinkQuality;
			tmpNtActv[u8SizeTmpNtActv].u8Type=2;
			u8SizeTmpNtActv++;
		}
	}
	/*DBG_vPrintf(1, "srcshort: %04x: IEEE %016llx - dstshort: %04x: IEEE %016llx",
			psStackEvent->uEvent.sApsDataIndEvent.uSrcAddress.u16Addr,
			ZPS_u64NwkNibFindExtAddr((void *)ZPS_pvNwkGetHandle(), psStackEvent->uEvent.sApsDataIndEvent.uSrcAddress.u16Addr),
			psStackEvent->uEvent.sApsDataIndEvent.uDstAddress.u16Addr,
			ZPS_u64NwkNibFindExtAddr((void *)ZPS_pvNwkGetHandle(), psStackEvent->uEvent.sApsDataIndEvent.uDstAddress.u16Addr)
	        );*/
	DBG_vPrintf(TRACE_BDB, "BDB: APP_vGenCallback [EP:%d src:%d dst:%d status:%d time:%ld cluster:%02x - type:%d] \n",
    					u8Endpoint,
    					psStackEvent->uEvent.sApsDataIndEvent.u8SrcEndpoint,
    					psStackEvent->uEvent.sApsDataIndEvent.u8DstEndpoint ,
    					psStackEvent->uEvent.sApsDataIndEvent.eStatus ,
    					psStackEvent->uEvent.sApsDataIndEvent.u32RxTime ,
    					psStackEvent->uEvent.sApsDataIndEvent.u16ClusterId,
    					psStackEvent->eType);

    if (((u8Endpoint > 1) &&
    		(psStackEvent->uEvent.sApsDataIndEvent.u8SrcEndpoint != psStackEvent->uEvent.sApsDataIndEvent.u8DstEndpoint)) &&
    		(psStackEvent->eType==ZPS_EVENT_APS_DATA_INDICATION) &&
			(psStackEvent->uEvent.sApsDataIndEvent.u16ClusterId != HVAC_CLUSTER_ID_THERMOSTAT)) //Fix duplicate DATAIND with different EP + Free persistent APDU
    {

    	/*u16Length =  0;
    	ZNC_BUF_U8_UPD  ( &au8LinkTxBuffer [ 0 ],  u8Endpoint,     u16Length );
    	ZNC_BUF_U8_UPD  ( &au8LinkTxBuffer [ u16Length ],  psStackEvent->eType,     u16Length );
    	ZNC_BUF_U16_UPD ( &au8LinkTxBuffer [ u16Length ],  psStackEvent->uEvent.sApsDataIndEvent.uSrcAddress.u16Addr,     u16Length );
    	vSL_WriteMessage ( 0x9997,
    	                                   u16Length,
    	                                   au8LinkTxBuffer,
    	                                   0);*/
    	if (psStackEvent->uEvent.sApsDataIndEvent.hAPduInst!=NULL)
    	    		PDUM_eAPduFreeAPduInstance(psStackEvent->uEvent.sApsDataIndEvent.hAPduInst);
    }else{
		/* Before passing to queue; copy is required to combine two arguments from stack */

		sZpsAfEvent.u8EndPoint = u8Endpoint;
		memcpy(&sZpsAfEvent.sStackEvent, psStackEvent, sizeof(ZPS_tsAfEvent));

		/* Post the task to break stack context */
		if(ZQ_bQueueSend(sBDB.hBdbEventsMsgQ, &sZpsAfEvent) == FALSE)
		{
			DBG_vPrintf(TRACE_BDB, "BDB: Failed to post zpsEvent %d \n", psStackEvent->eType);
			if (sZpsAfEvent.sStackEvent.eType == ZPS_EVENT_APS_DATA_INDICATION)
	        {
	            /* otherwise, BDB task passed ZCL to vZCL_HandleDataIndication or ZDP to ZDO servers which would free it */
	            PDUM_eAPduFreeAPduInstance(sZpsAfEvent.sStackEvent.uEvent.sApsDataIndEvent.hAPduInst);
	        }
	        else if (sZpsAfEvent.sStackEvent.eType == ZPS_EVENT_APS_INTERPAN_DATA_INDICATION)
	        {
	            /* otherwise, BDB task passed it to vZCL_HandleInterPanIndication which would free it */
	            PDUM_eAPduFreeAPduInstance(sZpsAfEvent.sStackEvent.uEvent.sApsInterPanDataIndEvent.hAPduInst);
	        }
	        else if (sZpsAfEvent.sStackEvent.eType == ZPS_EVENT_APS_ZGP_DATA_INDICATION)
	        {
	            /* otherwise, BDB task passed it to vZCL_HandleZgpDataIndication which would free it */
	            PDUM_eAPduFreeAPduInstance(sZpsAfEvent.sStackEvent.uEvent.sApsZgpDataIndEvent.hAPduInst);
	        }
		}
    }
}
/****************************************************************************
 *
 * NAME: bdb_taskBDB
 *
 * DESCRIPTION:
 * Processes events from the bdb event queue
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void bdb_taskBDB(void)
{
    BDB_tsZpsAfEvent sZpsAfEvent;
    BDB_tsBdbEvent sBDBEvent = {0};

    while(ZQ_bQueueReceive(sBDB.hBdbEventsMsgQ, &sZpsAfEvent))
    {
        /* Do not transmit link key for know node rejoins and BDB_JOIN_USES_INSTALL_CODE_KEY is TRUE */
        #if ((BDB_SET_DEFAULT_TC_POLICY == TRUE) &&  (BDB_JOIN_USES_INSTALL_CODE_KEY == TRUE))
            if((ZPS_ZDO_DEVICE_COORD == ZPS_eAplZdoGetDeviceType()) &&
               (ZPS_EVENT_TC_STATUS == sZpsAfEvent.sStackEvent.eType) &&
               (0x00 == sZpsAfEvent.sStackEvent.uEvent.sApsTcEvent.u8Status))
            {
                uint64 u64DeviceAddr;

                u64DeviceAddr = ZPS_u64NwkNibGetMappedIeeeAddr(ZPS_pvAplZdoGetNwkHandle(),
                                                               sZpsAfEvent.sStackEvent.uEvent.sApsTcEvent.uTcData.pKeyDesc->u16ExtAddrLkup);
                DBG_vPrintf(TRACE_BDB,"ZPS_EVENT_TC_STATUS %016llx \n",u64DeviceAddr);
                ZPS_bAplZdoTrustCenterSetDevicePermissions(u64DeviceAddr,
                                                           ZPS_DEVICE_PERMISSIONS_JOIN_DISALLOWED);
            }
        #endif
        /* Call BDB state machines */
        BDB_vFrStateMachine(&sZpsAfEvent); // Failure Recovery

        BDB_vInitStateMachine(&sZpsAfEvent);

        #if (defined BDB_SUPPORT_NWK_STEERING)
            BDB_vNsStateMachine(&sZpsAfEvent);
        #endif

        #if (defined BDB_SUPPORT_NWK_FORMATION)
            BDB_vNfStateMachine(&sZpsAfEvent);
        #endif

        #if (defined BDB_SUPPORT_FIND_AND_BIND_INITIATOR)
            vFbZdpHandler(&sZpsAfEvent.sStackEvent);
        #endif

        #if (defined BDB_SUPPORT_OOBC)
            BDB_vOutOfBandCommissionHandleEvent(&sZpsAfEvent);
        #endif

        /* Call application */
        sBDBEvent.eEventType = BDB_EVENT_ZPSAF;
        memcpy(&sBDBEvent.uEventData.sZpsAfEvent,&sZpsAfEvent,sizeof(BDB_tsZpsAfEvent));
        APP_vBdbCallback(&sBDBEvent);
    }
}

/****************************************************************************
 *
 * NAME: BDB_vZclEventHandler
 *
 * DESCRIPTION:
 *
 * PARAMETERS:      Name            RW  Usage
 *
 * RETURNS:
 *
 * NOTES:
 *
 ****************************************************************************/
PUBLIC void BDB_vZclEventHandler(tsBDB_ZCLEvent *psEvent)
{
    DBG_vPrintf(TRACE_BDB, "BDB ZCL Event %d\n", psEvent->eType);
    if ( (psEvent->eType > BDB_E_ZCL_EVENT_NONE)  &&
         (psEvent->eType < BDB_E_ZCL_EVENT_IDENTIFY_QUERY))
    {
        #if (defined BDB_SUPPORT_TOUCHLINK)
            BDB_vTlStateMachine(psEvent);
        #endif
    }
    else if (psEvent->eType >= BDB_E_ZCL_EVENT_IDENTIFY_QUERY)
    {
        #if ((defined BDB_SUPPORT_FIND_AND_BIND_INITIATOR) || (defined BDB_SUPPORT_FIND_AND_BIND_TARGET))
            BDB_vFbZclHandler(psEvent->eType,psEvent->psCallBackEvent);// identify query response or identify
        #endif
    }
}

/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/


/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
