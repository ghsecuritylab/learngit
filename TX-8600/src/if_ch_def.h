#ifndef  __IF_CH_DEF_H
#define  __IF_CH_DEF_H
//===========================================================================================
// Globle Interface And Chanal Def Moudle
//===========================================================================================
/*
 --------------------------------------------------------------------------------
 //Ethernet MII Interface And Chanal Define Moulde
 --------------------------------------------------------------------------------*/
// EthFun Client Define
enum ETH_RX_CLIENT_T {
	//ETH_AUDIO_DATA=0,
	ETH_RX_XTCP_DATA,
	ETH_RX_CLENT_TOTAL			//Total Client 2
};

// EthFun Client Define
enum ETH_TX_CLIENT_T {
	//ETH_AUDIO_DATA=0,
	ETH_TX_XTCP_DATA,
	//ETH_TX_AUD_TRAINSMIT,
	ETH_TX_CLENT_TOTAL			//Total Client 2
};


enum ETH_CFGCLIENT_T {
	ETH_AUDIO_CFG=0,
	ETH_XTCP_CFG,
	ETH_CFGCLENT_TOTAL		//Total Client 2
};
//--------------------------------------------------------------------------------
#ifdef __GLOBAL_CLIENT_
//
#define ETH_IF_CH_DEF \
/* Ethernet config interfaces client total */\
ethernet_cfg_if i_eth_cfg[ETH_CFGCLENT_TOTAL];\
/* Ethernet config interfaces total */\
ethernet_rx_if i_eth_rx_lp[ETH_RX_CLENT_TOTAL];\
ethernet_tx_if i_eth_tx_lp[ETH_TX_CLENT_TOTAL];\
/* smi interfaces */\
smi_if i_smi;
//
#endif	//__GLOBAL_CLIENT_
//===============================================================================================
/*
 --------------------------------------------------------------------------------
 //XTCP Interface And Chanal Define Moulde
 --------------------------------------------------------------------------------*/
// XTCP Client Define
enum XTCP_CLIENT_T {
	XTCP_USER=0,
	XTCP_CLENT_TOTAL		//Total Client 1 
};
//--------------------------------------------------------------------------------
#ifdef __GLOBAL_CLIENT_
//
#define XTCP_IF_CH_DEF \
xtcp_if	i_xtcp_user[XTCP_CLENT_TOTAL];
//
#endif	//__GLOBAL_CLIENT_
//=============================================================================================
/*
 --------------------------------------------------------------------------------
 //FLASH Interface And Chanal Define Moulde
 --------------------------------------------------------------------------------*/
// FLASHClient Define
enum FLASH_CLIENT_T {
	FLASH_XTCP=0,
	FLASH_AUDIO,
	FLASH_CLENT_TOTAL //Total Client 2
};
//--------------------------------------------------------------------------------
#ifdef __GLOBAL_CLIENT_
//
#define FLASH_IF_CH_DEF \
flash_if i_flash[FLASH_CLENT_TOTAL];

#endif	//__GLOBAL_CLIENT_
//=============================================================================================
/*
 --------------------------------------------------------------------------------
 //FLASH Interface And Chanal Define Moulde
 --------------------------------------------------------------------------------*/
// FLASHClient Define
enum UART_CLIENT_T {
	UART_USER,
	UART_CLENT_TOTAL //Total Client 2
};
//--------------------------------------------------------------------------------
#ifdef __GLOBAL_CLIENT_
//
#define UART_IF_CH_DEF \
/*uart tx part*/\
uart_tx_buffered_if i_uart_tx[UART_CLENT_TOTAL];

#endif	//__GLOBAL_CLIENT_
//=============================================================================================
/*
 --------------------------------------------------------------------------------
 //AUDIO Interface And Chanal Define Moulde
 --------------------------------------------------------------------------------*/
// FLASHClient Define
enum AUDIOCFG_CLIENT_T {
	AUDIOCFG_ETH,
	AUDIOCFG_CLIENT_TOTAL //Total Client 1
};

#ifdef __GLOBAL_CLIENT_
//
#define ETH_AUDIO_IF_CH_DEF \
/*eth audio config control*/\
ethaud_cfg_if	i_ethaud_cfg[AUDIOCFG_CLIENT_TOTAL];

#endif	//__GLOBAL_CLIENT_


#endif  //__IF_CH_DEF_H

