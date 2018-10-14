#include "assn3_bluenoise.h"
#include "assn3_bluenoiseBPLibrary.h"
#define CANDITATES_BASE_MULTIPLIER 1

Uassn3_bluenoiseBPLibrary::Uassn3_bluenoiseBPLibrary(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{

}

void Uassn3_bluenoiseBPLibrary::InitializeContainer(FBluenoiseContainer& Container, int32 Seed)
{
	Container.Initialize(Seed);
}

#pragma optimize("", off)
FVector Uassn3_bluenoiseBPLibrary::GetPoint(UPARAM(ref)FBluenoiseContainer & Container, float Min, float Max)
{
	FVector new_point;
	if (Container.m_ExistingPoints.Num() == 0) {
		float x = Min + (Max - Min) * Container.m_Stream.FRand();
		float y = Min + (Max - Min) * Container.m_Stream.FRand();
		new_point = FVector(x, y, 0.0f);
	}
	else {
		int num_canditates = Container.m_ExistingPoints.Num() * (CANDITATES_BASE_MULTIPLIER + 1);
		FVector best_canditate;
		float best_distance_sq = 0.0f;

		for (int i = 0; i < num_canditates; i++) {
			float x = Min + (Max - Min) * Container.m_Stream.FRand();
			float y = Min + (Max - Min) * Container.m_Stream.FRand();

			FVector canditate(x, y, 0.0f);
			float minDist = FLT_MAX;

			for (auto& p : Container.m_ExistingPoints) {

				auto d = FVector::Distance(canditate, p);

				if (d < minDist)
					minDist = d;
			}

			if (minDist > best_distance_sq)
			{
				best_distance_sq = minDist;
				best_canditate = canditate;
			}
			
		}

		new_point = best_canditate;
	}

	Container.m_ExistingPoints.Add(new_point);
	return new_point;
}
#pragma optimize("", on)
