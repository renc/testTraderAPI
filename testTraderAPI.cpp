// a simple example demonstrate how to use 
// CThostFtdcTraderApi and CThostFtdcTraderSpi interface.
// this example will demostrate the procedure of an order insertion.

#include <cstdio>
#include <string>
#include <iostream>
#include <locale.h>
#include <cwchar>
#include ".\v6.3.6_20150515_traderapi_win64\tradeapidll\ThostFtdcTraderApi.h"
#include <Windows.h>

// Flag of the order insertion finished or not.
// create a manual reset event with no signal
HANDLE g_hEvent = CreateEvent(NULL, true, false, NULL);

// participant ID
TThostFtdcBrokerIDType g_chBrokerID = "9999";
// user id
TThostFtdcInvestorIDType g_chInvestorID = "067906";//"066718";//
TThostFtdcUserIDType g_chUserID = "067906";//"066718";//
TThostFtdcPasswordType g_chPassword = "123456";

class CSimpleHandler : public CThostFtdcTraderSpi
{
public:
	CSimpleHandler(CThostFtdcTraderApi *pUserApi)
	: m_pUserApi(pUserApi) {}

	~CSimpleHandler() {};

// -----------------------------------------------------
// interfaces from CThostFtdcTraderSpi.

	// after making a succeed connectioin with the CTP server, the 
	// client should send the login request to the CTP server.
	virtual void OnFrontConnected();
	// when the connection between client and the ctp server
	// disconnected, the following function will be called.
	virtual void OnFrontDisconnected(int nReason);

	// after receiving the login request from the client. the ctp
	// server will send the following response to notify the client
	// whether the login success or not.
	virtual void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	// order insertion response
	virtual void OnRspOrderInsert(
		CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo,
		int nRequestID, bool bIsLast);

	// order insertion return
	virtual void OnRtnOrder(CThostFtdcOrderField *pOrder);

	///报单操作请求响应
	virtual void OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	
	// the error notification caused by client rquest
	virtual void OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

// -----------------------------------------------------
// end of interfaces from CThostFtdcTraderSpi.
// -----------------------------------------------------
// non interfaces from CThostFtdcTraderSpi.
private:
	bool IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo);
// -----------------------------------------------------
// end of non interfaces from CThostFtdcTraderSpi.

private:
	CThostFtdcTraderApi *m_pUserApi;
};


bool CSimpleHandler::IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo)
{
	// 如果ErrorID != 0, 说明收到了错误的响应
	bool bResult = ((pRspInfo) && (pRspInfo->ErrorID != 0));
	if (bResult)
		std::wcout << "ErrorRspInfo: ErrorID=" << pRspInfo->ErrorID << ", ErrorMsg=" << pRspInfo->ErrorMsg << std::endl;
	return bResult;
}
// after making a succeed connectioin with the CTP server, the 
// client should send the login request to the CTP server.
void CSimpleHandler::OnFrontConnected()
{
	std::wcout << "OnFrontConnected: " << std::endl;

	std::wcout << "to ReqUserLogin.\n";
	CThostFtdcReqUserLoginField reqUserLogin;

	//printf("BrokerID:");
	//scanf_s("%s", &g_chBrokerID, sizeof(TThostFtdcBrokerIDType));
	strcpy_s(reqUserLogin.BrokerID, sizeof(TThostFtdcBrokerIDType), g_chBrokerID);

	//printf("UserID:");
	//scanf_s("%s", &g_chUserID, sizeof(TThostFtdcUserIDType));
	strcpy_s(reqUserLogin.UserID, sizeof(TThostFtdcUserIDType), g_chUserID);

	//printf("password:");
	//scanf_s("%s", &g_chPassword, sizeof(TThostFtdcPasswordType));
	strcpy_s(reqUserLogin.Password, sizeof(TThostFtdcPasswordType), g_chPassword);

	m_pUserApi->ReqUserLogin(&reqUserLogin, 0);
}

// when the connection between client and the ctp server
// disconnected, the following function will be called.
void CSimpleHandler::OnFrontDisconnected(int nReason)
{
	printf("OnFrontDisconnected.\n");
}

// after receiving the login request from the client. the ctp
// server will send the following response to notify the client
// whether the login success or not.
void CSimpleHandler::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo,
	int nRequestID, bool bIsLast)
{
	printf("OnRspUserLogin: \n");
	std::wcout << "ErrorCode=[" << pRspInfo->ErrorID << "], ErrorMsg=[ " << pRspInfo->ErrorMsg << std::endl;
	std::wcout << "RequestID=[ " << nRequestID << "], Chain=[" << bIsLast << "]\n";

	if (pRspInfo->ErrorID != 0)
	{
		// in case any login failure, the client should handle 
		// this error. 
		printf("Failed to login, error code = %d, error msg = %s, "
			"requestid = %d, chain = %d",
			pRspInfo->ErrorID, pRspInfo->ErrorMsg, nRequestID, bIsLast);
		exit(-1);
	}

	//为了使投资者及时准确的了解自己的交易状况，如可用资金，持仓，保证金占用等，从而及时了解自己的风 险状况，
	//综合交易平台要求投资者在每一个交易日进行交易前都必须对前一交易日的结算结果进行确认。

	/////投资者结算结果确认信息
	CThostFtdcSettlementInfoConfirmField req = { 0 };
	strcpy_s(req.BrokerID, g_chBrokerID);
	strcpy_s(req.InvestorID, g_chInvestorID);	
	m_pUserApi->ReqSettlementInfoConfirm(&req, 0);///投资者结算结果确认

	// 报单 
	// log success, then send order insertion request.
	CThostFtdcInputOrderField ord;
	memset(&ord, 0, sizeof(ord));

	strcpy_s(ord.BrokerID, g_chBrokerID);
	strcpy_s(ord.InvestorID, g_chInvestorID);
	strcpy_s(ord.UserID, g_chUserID);
	strcpy_s(ord.InstrumentID, "ag1609");
	strcpy_s(ord.OrderRef, "000000000001");
	ord.OrderPriceType = THOST_FTDC_OPT_LimitPrice;
	ord.Direction = THOST_FTDC_D_Buy;
	strcpy_s(ord.CombOffsetFlag, "0"); // combination order's offset flag. 
	strcpy_s(ord.CombHedgeFlag, "1"); // combination or hedge flag.
	ord.LimitPrice = 4388;
	ord.VolumeTotalOriginal = 10; // volumn
	ord.TimeCondition = THOST_FTDC_TC_GFD;//有效期类型:当日有效
	strcpy_s(ord.GTDDate, "");
	ord.VolumeCondition = THOST_FTDC_VC_AV;//成交量类型:任何数量
	ord.MinVolume = 1;
	ord.ContingentCondition = THOST_FTDC_CC_Immediately; // trigger condition
	ord.StopPrice = 0;
	ord.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;//强平原因:非强平
	ord.IsAutoSuspend = 0;

	int ret = m_pUserApi->ReqOrderInsert(&ord, 1);
	std::wcout << " 请求 | 发送报单..." << ((ret == 0) ? "成功" : "失败") << std::endl;
}

// order insertion response
void CSimpleHandler::OnRspOrderInsert(
	CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo,
	int nRequestID, bool bIsLast)
{
	// output the order insertion result
	printf("OnRspOrderInsert:\n");
	printf("ErrorCode = [%d], ErrorMsg=[%s].\n",
		pRspInfo->ErrorID, pRspInfo->ErrorMsg);

	// inform the main thread order insertion is over
	SetEvent(g_hEvent);
}

void CSimpleHandler::OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	printf("OnRspOrderAction.\n");
};

// order insertion return
void CSimpleHandler::OnRtnOrder(CThostFtdcOrderField *pOrder)
{
	printf("OnRtnOrder:\n");
	printf("OrderSysID=[%s]\n", pOrder->OrderSysID);
}

// the error notification caused by client rquest
void CSimpleHandler::OnRspError(CThostFtdcRspInfoField *pRspInfo,
	int nRequestID, bool bIsLast)
{
	printf("OnRspError:\n");
	printf("ErrorCode=[%d], ErrorMsg=[%s]\n", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
	printf("RequestID=[%d], Chain=[%d]\n", nRequestID, bIsLast);

	// the client should handle the error 
}


int main()
{
	std::locale::global(std::locale(""));
	std::wcout.imbue(std::locale(""));
	SetConsoleOutputCP(1252);

	std::string ss = u8"中文";
	std::cout << ss << std::endl; 
	std::cout << "---------------------------\n"; 

	// request connect 
	CThostFtdcTraderApi *pUserApi = CThostFtdcTraderApi::CreateFtdcTraderApi();
	CSimpleHandler sh(pUserApi);
	pUserApi->RegisterSpi(&sh);

	pUserApi->SubscribePrivateTopic(THOST_TERT_RESUME);
	pUserApi->SubscribePublicTopic(THOST_TERT_RESUME);
	 
	pUserApi->RegisterFront("tcp://180.168.146.187:10000"); 
	pUserApi->Init(); // connect to server.
	// end of request connect 

	// waiting for the order insertion.
	WaitForSingleObject(g_hEvent, INFINITE);

	// 
	pUserApi->Release();
	
	//
	std::cout << "To Exit ...\n";
	getc(stdin);
	return 0;
}

/*
交易"tcp://180.168.146.187:10000", 假如交易用了这addr,ErrorCode=21, ErrMsg:此交易席位未连接到交易所.
行情"tcp://180.168.146.187:10010"
第一套：
标准CTP：
第一组：Trade Front：180.168.146.187:10000，Market Front：180.168.146.187:10010；【电信】
第二组：Trade Front：180.168.146.187:10001，Market Front：180.168.146.187:10011；【电信】
第三组：Trade Front：218.202.237.33 :10002，Market Front：218.202.237.33 :10012；【移动】
交易阶段(服务时间)：与实际生产环境保持一致
CTPMini1：
第一组：Trade Front：180.168.146.187:10003，Market Front：180.168.146.187:10013；【电信】
第二套：
交易前置：180.168.146.187:10030，行情前置：180.168.146.187:10031；【7x24】
第二套环境仅服务于CTP API开发爱好者，仅为用户提供CTP API测试需求，不提供结算等其它服务。
*/
