
#include "../SandEngine/SandEngine.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	auto framework = SPlatformManager::CreateFramework();
	if (framework && framework->Init())
	{
		SIGraphicsInterface* pGraphics = SGraphics::Initialize(GraphicsInterfaceEnum::GI_DX_12);
		if (pGraphics && pGraphics->Initialize(framework, 1920, 1080, false, true))
		{
			SModelLoader* ModelLoader = new SModelLoader();
			ModelLoader->Initialize();
			SSceneManager* SceneManager = new SSceneManager();
			SScene* scene = SceneManager->CreateScene(L"gogo");
			SceneManager->LoadScene(scene);

			SNode* node = scene->GetRootNode();

			auto&& model = ModelLoader->LoadModelFromFile(LR"(boblampclean\boblampclean.md5mesh)");
			model.Scale = {0.3f, 0.3f, 0.3f};
			model.Location = { 0.0f, -10.0f, -10.0f };
			model.Material.Type = MaterialType::SKINNING;
			node->Queue(model);
			
			auto&& model2 = ModelLoader->LoadModelFromFile(LR"(Jet_Animation.FBX)");
			model2.Location = { -2.0f, 0.0f, 0.0f };
			model2.Material.Type = MaterialType::TEXTURE;
			node->CreateChild()->Queue(model2);
			//node->Queue(model2);
			
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
