#include "TurboNet/TurboServer.h"
#include "TurboNet/TurboClient.h"

void Client()
{
	TurboNet::TurboClient *pClient = new TurboNet::TurboClient;
	pClient->ConnectToServer(2000,5);

	while(true)
	{
		pClient->Update(0.01f);

		if(pClient->GetStatus())
		{
			TurboNet::TurboPeer *pPeer = pClient->GetPeer();

			if(pPeer->Read())
			{
				int a;
				pPeer->GetData()->GetValue("Hello",a);
				printf("\nGot Value : %d",a);
			}

			pPeer->GetData()->AddValue("Hello",35);
			pPeer->Flush();

			if(pPeer->IsInactive())
			{
				pClient->Close();
			}
		}

		Sleep(10);
	}
}

void Server()
{
	TurboNet::TurboServer *pServer = new TurboNet::TurboServer;
	pServer->CreateServer(2000,1,5);

	while(true)
	{
		pServer->Update(0.01f);

		if(pServer->GetStatus())
		{
			TurboNet::TurboPeer *pPeer = (*pServer->GetPeers())[0];

			if(pPeer->Read())
			{
				int a;
				pPeer->GetData()->GetValue("Hello",a);
				printf("\nGot Value : %d",a);
			}

			pPeer->GetData()->AddValue("Hello",24);
			pPeer->Flush();

			if(pPeer->IsInactive())
			{
				pServer->RemovePeer(pPeer);
			}
		}

		Sleep(10);
	}
}

int main()
{
	printf("Hello TurboNetTest");
	printf("\n1. TurboServer\n2. TurboClient");
	int num;
	scanf("%d",&num);

	switch(num)
	{
	case 1:
		Server();
		break;
	case 2:
		Client();
		break;
	default:
		printf("Invalid Entry");
	}

	return 0;
}