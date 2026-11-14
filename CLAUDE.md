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
- Register every processor query in the constructor (`: EntityQuery(*this)`), never via `RegisterWithProcessor` inside `ConfigureQueries` — 5.9 initializes registered queries *before* calling `ConfigureQueries`, and adding requirements to an uninitialized query asserts ("Modifying requirements before initialization is not supported"). Don't copy-assign one query into another.
- Processors created manually with `NewObject` (e.g. `MassSelectedProcessor` in `MassRaycastHitSubsystem`) must be initialized with `CallInitialize(Owner, EntityManager.AsShared())` before `UE::Mass::Executor::Run`.
- Mass processing can still be in flight (parallel phases on worker threads) while game-thread code such as Blueprint input events runs. Synchronous entity APIs (`CreateEntity`, `BatchCreateEntities`, `UMassSpawnerSubsystem::SpawnEntities`, ...) then assert ("called during mass processing"). Check `Manager.IsProcessing() || Manager.GetObserverManager().IsLocked()` and defer via `Manager.Defer().PushCommand<FMassDeferredCreateCommand>(...)` — see `DeferSpawnIfMassIsProcessing` in `PlaceableSpawnerSpectatorPawn.cpp` (same pattern as the engine's `UMassAgentSubsystem`).
- `FMassEntityQuery::ForEachEntityChunk(Context, ...)` and `GetNumMatchingEntities()` no longer take the entity manager.
- Observers use `ObservedTypes.Add(...)` and `ObservedOperations = EMassObservedOperationFlags::...`.
- 5.9 headers include less transitively: `MassEntityUtils.h` (for `TMassExternalSubsystemTraits`, `UE::Mass::Utils::GetEntityManagerChecked`), `MassProcessingContext.h` (for `FMassProcessingContext`), `MassCommonTypes.h` (for `UE::Mass::ProcessorGroupNames`).

## Entity configs & actor representation

- `BP_CharacterVisualization` (actor LOD for `DA_BP_CharacterMousClickEntity`) has a **MassAgentComponent with its own entity config** (parent = the data asset) that **overrides** the Agent Movement/Orientation/Capsule Collision Sync traits. Edit sync settings there, not only in the data asset — the log shows "Overriding ... trait with another instance".
- Height convention: entity transform = feet. Placement uses the engine's **Agent Feet Location Sync** trait (root = entity + capsule half height); **Agent Capsule Collision Sync** has `bSyncTransform = false` (it would put the capsule *center* on the ground). The character mesh is at Z = -88 (standard ACharacter layout).
- Skinned-instance animation: visualization LODs are High = actor, Medium = `SkinnedMeshInstance` (`SK_citizen_female_11` + `/Game/Blueprints/MassEntity/Animation/TP_CitizenLocomotion`: 0 Idle, 1 Walk, 2 Run), Low/Off = None. `UMassLocomotionAnimationTrait` / `UMassLocomotionAnimationProcessor` (`MassLocomotionAnimation.h`) pick the sequence from velocity; the ISKM only applies new auto-play data when the sequence index changes, so play rate is set at the switch.

## Known open items

- `UMassRaycastHitSubsystem::SetMoveTarget` runs `USelectedEntitiesProcessor` directly via `UE::Mass::Executor::Run` from an input event; if a parallel phase is in flight this races with worker-thread processors. Not observed crashing yet — candidate for deferring if it does.
- NoiseGen: `FWindNoiseCalculator` (`Plugins/NoiseGen/Source/NoiseGen/Public/WindNoiseCalculator.h`) has uninitialized UPROPERTYs (`NoiseType`, `Offset`, `Rotation`) reported at startup.
- Blueprint/asset fixups after the 5.4 -> 5.9 upgrade are to be done from inside the 5.9 editor (check Message Log / output log on first open).
- Migration work is on branch `upgrade-5.9`. The old copy at `D:\Unreal\UE5MassTest` is obsolete once this one is verified.
