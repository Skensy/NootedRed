// VP9 (piano V3, -NRedVP9): risolve i siti del backend VP9 nella cache condivisa corrente.
// Tutte le funzioni toccate sono verificate per intero (byte e riferimenti rel32); i bersagli dati
// fuori da __TEXT passano dalla tabella SectionTargets generata insieme alle patch.
#pragma once
#include <UserCacheResolver.hpp>
#include <UserSharedCache.hpp>
#include <UserVP9DecodeData.hpp>

namespace UserVP9DecodeResolver
{
    using namespace UserVP9DecodeData;
    static constexpr UInt32 ImageCount   = sizeof(ResolverImages) / sizeof(ResolverImages[0]);
    static constexpr UInt32 PatchCount   = sizeof(Patches) / sizeof(Patches[0]);
    static constexpr UInt32 SectionCount = sizeof(SectionTargets) / sizeof(SectionTargets[0]);
    static_assert(PatchCount > 0 && PatchCount <= 32);
    static_assert(MaxPatchSize == UserVideoDecodeData::MaxPatchSize);
    static_assert(sizeof(CacheUUID) == 16);

    struct Plan
    {
        UserVideoDecodeData::Patch     patches[PatchCount]{};
        UInt8                          original[PatchCount][MaxPatchSize]{};
        UInt8                          patched[PatchCount][MaxPatchSize]{};
        UserCacheResolver::ImageBounds images[ImageCount]{};
    };

    enum class Resolution { Ready, Retry, Rejected };

    inline Resolution resolve(UserSharedCache::Input& input, UInt64 base, const UInt8* uuid, Plan& plan, bool trace)
    {
        if (!uuid || memcmp(uuid, CacheUUID, sizeof(CacheUUID)) != 0) { return Resolution::Rejected; }
        if (!UserCacheResolver::resolveImages(input, base, ResolverImages, plan.images)
            || !UserCacheResolver::resolveFunctions(input, base, ResolverImages, ResolverFunctions, Patches,
                                                    ResolverLocations, plan.patches, &plan.original[0][0],
                                                    &plan.patched[0][0], MaxPatchSize, plan.images, true,
                                                    SectionTargets, SectionCount, nullptr, nullptr))
        {
            return input.readError ? Resolution::Retry : Resolution::Rejected;
        }
        static_assert(TraceSite < PatchCount && TraceOffset < Patches[TraceSite].size);
        plan.patched[TraceSite][TraceOffset] = trace ? 1 : 0;
        return Resolution::Ready;
    }
}    // namespace UserVP9DecodeResolver
