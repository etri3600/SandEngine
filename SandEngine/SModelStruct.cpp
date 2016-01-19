#include "SModelStruct.h"
#include "SAnimator.h"

void SModel::Update(double delta)
{
	if (Animator)
	{
		Animator->Update(delta);
	}
}
