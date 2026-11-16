# Golf Game — Unreal C++ systems (first pass)

Ports of the Godot data layer and four autoloads. **Not compiled yet** (written without an Unreal install to build against), so expect a few first-compile errors; paste them back and they get fixed fast.

## 1. Drop the files in

Copy the `Data`, `Systems` and `Tests` folders into `Source/<YourProject>/`.

Find and replace `GOLFGAME_API` with `<YOURPROJECT>_API` (upper case, e.g. `MYGAME_API`) in every file.

Includes use folder prefixes (`"Data/GolfTypes.h"`, `"Systems/CourseManager.h"`), which resolve from the module root.

## 2. Build.cs

Add `"DeveloperSettings"` to the module dependencies:

```csharp
PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "DeveloperSettings" });
```

Regenerate project files, then build.

## 3. Create the data assets

In the Content Browser: Add > Miscellaneous > Data Asset, pick the class.

- **9 x HoleData** (`UHoleData`): copy values from the Godot `.tres` files / the hole table in the port checklist. Set `startsUnlocked` true for hole 1 only.
- **4 x EquipmentData**: copy the four tiers from Godot (tier, cost, description, affectsStat, restoreAmount).
- **Jake and Maria** (`UStaffData`): both role **Groundskeeper**.
  - Jake: skill 0.2, daily wage 35, hire cost 80
  - Maria: skill 0.8, daily wage 70, hire cost 200
- **1 x GolfDataRegistry** (`UGolfDataRegistry`): add all holes and equipment (and rooms, furniture, application pairs once they exist).

Then: **Project Settings > Game > Golf Game > Data Registry** = your registry asset.

## 4. Run the test

1. Make a blank level, drag in a **GolfSystemTest** actor (Place Actors, search "Golf System Test").
2. In its Details panel set **Jake** and **Maria**.
3. Press Play, open Output Log, filter by `GolfTest`.

Every line is `PASS` or `FAIL` with expected vs. actual. Restart Play to re-run (state resets each session, and the test spends cash and unlocks hole 2, so run it once per session).

## Assumptions to verify against your Godot code

I could not see these exact function bodies, so I matched them to the test numbers:

1. **Maintenance is applied to all three stats** (fairway, rough, green), and **natural decay (-0.05) applies to all three stats of every unlocked hole**. `test.gd` only checks fairway (0.50 + 0.30 - 0.05 = 0.75). If Godot only touched fairway, tell me.
2. **Only Groundskeepers produce maintenance.** Pro Shop and Instructor staff do nothing yet.
3. **PurchaseEquipment fails if the tier is already owned or lower** (matches the "can't buy the tier again" behavior).
4. **`PlaceItem` requires the room to be built**, and `RemoveItem` gives no refund.
5. **Revenue = unlocked holes x 200.** Hole 0 / `countsForRevenue` no longer exist.

## What's here

| File | Port of |
|---|---|
| `Data/GolfTypes.h` | enums + runtime state structs (new) |
| `Data/GolfData.h` | HoleData, EquipmentData, StaffData, ApplicationPairData, RoomData, FurnitureItemData, registry |
| `Data/GolfSettings.h` | Project Settings entry that points at the registry (replaces hardcoded `hole0%d.tres` paths) |
| `Systems/CourseManager.*` | CourseManager autoload |
| `Systems/StaffSystem.*` | StaffSystem autoload |
| `Systems/DayCycleSubsystem.*` | DayCycle autoload |
| `Systems/ClubhouseSystem.*` | ClubhouseSystem autoload |
| `Tests/GolfSystemTest.*` | test.gd (with automatic PASS/FAIL) |

## Notes on behavior

- The clock only ticks while the game is **unpaused**. Panels should call `SetGamePaused(true)` when they open (same as `get_tree().paused`).
- State lives on the Game Instance, so it survives `Open Level` (entering the clubhouse interior).
- UI should **bind to delegates** (`onCashChanged`, `onPhaseChanged`, `onDayReportReady`, ...) instead of polling.

## Data values to enter (from the Godot .tres files)

Equipment (UEquipmentData):
| Asset | Name | Tier | Cost | Stat | Restore |
|---|---|---|---|---|---|
| tierZero | Push Mower | 0 | 0 | Fairway | 0.10 |
| tierOne | Gas Mower | 1 | 300 | Fairway | 0.18 |
| tierTwo | Weed Whacker Upgrade | 2 | 450 | Rough | 0.15 |
| tierThree | Aerator / Sprayer | 3 | 650 | Green | 0.20 |

Staff (UStaffData, all Groundskeeper):
| Name | Skill | Wage | Hire cost |
|---|---|---|---|
| Jake | 0.2 | 35 | 80 |
| Maria | 0.8 | 70 | 200 |
| Bob | 0.2 | 50 | 50 |
| Kyle | 0.3 | 75 | 100 |

Hole 0 (hub) and hole 10 (clubhouse interior): set countsForRevenue = false.

## Hole data (from the Godot hole .tres files)

| # | Name | Par | Tier | Cost | Starts unlocked | Fairway/Rough/Green | Water | Sand | Counts for revenue |
|---|---|---|---|---|---|---|---|---|---|
| 0 | Garden (hub) | 0 | 0 | 0 | yes | 0/0/0 | - | - | NO |
| 1 | Hole 1 | 4 | 0 | 0 | yes | 1.0/1.0/1.0 | - | - | yes |
| 2 | Hole 2 | 4 | 0 | 150 | no | 0.9/0.9/0.9 | - | yes | yes |
| 3 | Hole 3 | 5 | 1 | 300 | no | 0.7/0.7/0.7 | yes | - | yes |
| 4 | Hole 4 | 4 | 2 | 500 | no | 0.4/0.3/0.4 | yes | yes | yes |
| 5 | Hole 5 | 3 | 1 | 250 | no | 0.6/0.6/0.6 | - | yes | yes |
| 6 | Hole 6 | 4 | 2 | 550 | no | 0.4/0.4/0.3 | yes | - | yes |
| 7 | Hole 7 | 4 | 2 | 600 | no | 0.3/0.4/0.3 | - | yes | yes |
| 8 | Hole 8 | 3 | 3 | 700 | no | 0.2/0.3/0.2 | yes | yes | yes |
| 9 | Hole 9 | 5 | 3 | 900 | no | 0.2/0.2/0.2 | yes | yes | yes |
| 10 | Clubhouse (interior) | 0 | 0 | 0 | yes | 0/0/0 | - | - | NO |

Holes 1-9 par total = 36.

## Application pair (applicationPair1.tres)
- pairId: (blank in Godot; suggest "pair1")
- applicantA: Bob, applicantB: Kyle
- flavorText: "Two groundskeepers have applied. One's cheap and works that way, the other's experienced but expensive."


## Course shape mask (Nov 11)
`CourseShapeRenderer.h/.cpp` draws every `HoleCoverageZone` (bDrawShape on) into a render target as a rounded rectangle:
R = fairway, G = green, B = rough. M_Turf reads that mask for the fairway and green look.
Needs: RT_ShapeMask (RGBA8, 4096), M_ShapeStamp (User Interface, Additive), and the Course Shapes entries
in Project Settings > Game > Golf Game. Console: `golf.RebuildShapes`, `golf.ShapeYawSign 1|-1`.

## Equipment tiers drive the mower (Nov 13)

`UEquipmentData` has two new fields, `cutRadiusCm` and `moveSpeedMultiplier`.
`UCourseManager::GetOwnedEquipmentData()` returns the data asset for the owned tier. The mower Blueprint reads it at BeginPlay and on `onEquipmentTierChanged`.

## Save and load (Nov 15)

`UGolfSaveGame` (Data/GolfSaveGame.h) holds everything saved. `UGolfSaveSubsystem` saves and loads it, autosaves to slot `Auto` at the start of each day, and fires `onGameLoaded` after a load. Each system has `WriteToSave` / `ReadFromSave`. The cut mask is not saved. Console: `golf.Save [slot]`, `golf.Load [slot]`.

## Guests and satisfaction (Nov 15)

`UGuestSystem` (Systems/GuestSystem.h) replaces the flat 200-per-hole revenue. At the start of Close processing, before maintenance and decay, DayCycle calls `ProcessDay()`:

- hole satisfaction = fairway x 0.35 + rough x 0.15 + green x 0.50
- arrivals per hole = 8 x lerp(0.5, 1.5, reputation)
- paying guests per hole = round(arrivals x lerp(0.25, 1, satisfaction)); revenue = paying guests x 25
- reputation moves 20% of the way toward the day's average satisfaction

`FDayReport` gained guests, satisfaction and reputation (refresh any Break DayReport node). Reputation is saved. Console: `golf.SetReputation <0..1>`. The constants are at the top of GuestSystem.h; tune by feel. GolfSystemTest expectations changed (revenue 375, net +270).
