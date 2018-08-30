
#include "../SandEngine/SandEngine.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	auto framework = SPlatformManager::CreateFramework();
	if (framework && framework->Init())
	{
		SIGraphicsInterface* pGraphics = SGraphics::Initialize(EGraphicsInterfaceEnum::DX12);
		if (pGraphics && pGraphics->Initialize(framework, 1920, 1080, false, true))
		{
			SModelLoader* ModelLoader = new SModelLoader();
			ModelLoader->Initialize();
			SSceneManager* SceneManager = new SSceneManager();
			SScene* scene = SceneManager->CreateScene(L"gogo");
			SceneManager->LoadScene(scene);

			SNode* root_node = scene->GetRootNode();

			auto model = ModelLoader->LoadModelFromFile(LR"(boblampclean\boblampclean.md5mesh)");
			model->Scale = {0.3f, 0.3f, 0.3f};
			model->Location = { 0.0f, -10.0f, -10.0f };
			model->Material.Type = EMaterialType::SKINNING;
			root_node->Queue(model);
			
			auto model2 = ModelLoader->LoadModelFromFile(LR"(Jet_Animation.FBX)");
			model2->Location = { 8.0f, 0.0f, 0.0f };
			model2->Scale = { 0.2f, 0.2f, 0.2f };
			model2->Material.Type = EMaterialType::TEXTURE;
			root_node->CreateChild()->Queue(model2);
			//node->Queue(model2);

			SLighting* light1 = new SLighting();
			light1->Direction = SVector3(-1, -2, -3);
			light1->Position = SVector3(10, 10, 10);
			light1->Strength = SVector3(1, 1, 1);

			root_node->Queue(light1);

			do
			{
				if (framework->Tick() == false)
					break;
				SceneManager->Tick();
			} while (true);
		}
		SGraphics::Fianalize();
	}

	return 0;
}
