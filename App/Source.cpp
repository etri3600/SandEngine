
#include "../SandEngine/SandEngine.h"


#include <memory>
#include <type_traits>


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

			auto&& model = ModelLoader->LoadModelFromFile(LR"(Lara_Croft_default_outfit\Lara_Croft_default.dae)");
			//SceneManager->Queue(model);
			SCube cube;
			cube.Location = SVector3{1.0f, 0.0f, 0.0f};
			SceneManager->Queue(cube);
			STriangle tri;
			//SceneManager->Queue(tri);

			SceneManager->Draw();
			
			while (true)
			{
				framework->Tick();
				SceneManager->Tick();
				pGraphics->Render();
				pGraphics->Present();
			}

			SceneManager->Reset();
		}
	}

	return 0;
}
