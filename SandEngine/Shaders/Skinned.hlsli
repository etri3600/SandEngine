#define MAX_BONES 128

column_major matrix CalcBoneTransform(column_major matrix boneTransforms[MAX_BONES], uint4 bones, float4 weights)
{
	column_major matrix boneTransform = mul(boneTransforms[bones[0]], weights[0]);
	boneTransform += mul(boneTransforms[bones[1]], weights[1]);
	boneTransform += mul(boneTransforms[bones[2]], weights[2]);
	boneTransform += mul(boneTransforms[bones[3]], weights[3]);

	return boneTransform;
}