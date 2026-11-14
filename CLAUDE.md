# UE5MassTest

Unreal Engine **5.9** project experimenting with the Mass Entity framework (spawning placeables, selection via signals, raycast move targets, custom nav-path following).

## Engine & build

- Engine: Launcher install at `X:\Unreal_Installs\UE_5.9` (registered via the Epic Launcher manifest, `EngineAssociation: "5.9"`). Other versions live alongside it in `X:\Unreal_Installs\UE_5.x`; `X:\UnrealEngine` is a UE6 source build, not 5.9.
- Build the editor target from the command line:
  ```
  X:\Unreal_Installs\UE_5.9\Engine\Build\BatchFiles\Build.bat UE5MassTestEditor Win64 Development -Project="X:\Unreal_Projects\Internal\UE5MassTest\UE5MassTest.uproject" -WaitMutex -NoHotReloadFromIDE
  ```
  Close the editor first (Live Coding locks the DLL). If UBT fails with "BuildRules\...ModuleRules.dll is being used by another process", Rider is regenerating project files — wait for it to finish.
- Only the Editor target has been verified on 5.9; the Game target has not been built yet.

## Layout

- `Source/UE5MassTest` — game module (processors, traits, fragments, subsystems).
  - `MassNavigationExtensions/MassNavigationPath` — custom nav-path request/follow (added to public include paths in Build.cs).
  - `MassActorsExtensions` — custom translators (pawn movement / floating pawn movement <-> Mass).
- `Plugins/MassExtensions`, `Plugins/NoiseGen` (noise generation, wind grid), `Plugins/PluginTest_0206_2001` (test plugin, but a dependency of the game module).

## Mass API conventions on 5.9 (migrated from 5.4)

- MassEntity is an engine runtime module (no plugin entry); StructUtils is merged into CoreUObject. Core element types (`FMassFragment`, `FMassTag`, `FMassEntityHandle`, `FTransformFragment`) live in the **MassCore** module.
- Processors override `ConfigureQueries(const TSharedRef<FMassEntityManager>&)` and `InitializeInternal(UObject&, const TSharedRef<FMassEntityManager>&)` — the old no-arg/`Initialize(UObject&)` versions are `final`.
- `FMassEntityQuery::ForEachEntityChunk(Context, ...)` and `GetNumMatchingEntities()` no longer take the entity manager.
- Observers use `ObservedTypes.Add(...)` and `ObservedOperations = EMassObservedOperationFlags::...`.
- 5.9 headers include less transitively: `MassEntityUtils.h` (for `TMassExternalSubsystemTraits`, `UE::Mass::Utils::GetEntityManagerChecked`), `MassProcessingContext.h` (for `FMassProcessingContext`), `MassCommonTypes.h` (for `UE::Mass::ProcessorGroupNames`).

## Known open items

- `PlaceableSpawnerSpectatorPawn.cpp` still calls the deprecated single-threaded `FMassEntityManager::Initialize()` (C4996) — will break in the next engine release; needs `FMassEntityManager_InitParams_Concurrent`.
- Blueprint/asset fixups after the 5.4 -> 5.9 upgrade are to be done from inside the 5.9 editor (check Message Log / output log on first open).
- Migration work is on branch `upgrade-5.9`. The old copy at `D:\Unreal\UE5MassTest` is obsolete once this one is verified.
