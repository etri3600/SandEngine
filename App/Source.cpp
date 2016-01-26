
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
			SceneManager->Init(pGraphics);

			auto&& model = ModelLoader->LoadModelFromFile(LR"(boblampclean\boblampclean.md5mesh)");
			model.Scale = {0.3f, 0.3f, 0.3f};
			model.Location = { 0.0f, 0.0f, -10.0f };
			model.Rotation = { std::sinf(-45.0f), 0.0f, 0.0f, std::cosf(-45.0f) };
			SceneManager->Queue(model);
			
			SCube cube1;
			cube1.Rotation = { 1.0f, 1.0f, 0.0f, std::cosf(15.0f)};
			cube1.Location = { -0.3f, 0.0f, 0.0f };
			//SceneManager->Queue(cube1);

			SceneManager->Draw();
			
			do
			{
				if (framework->Tick() == false)
					break;
				SceneManager->Tick();
			} while (true);

			SceneManager->Reset();
		}
	}

	return 0;
}
