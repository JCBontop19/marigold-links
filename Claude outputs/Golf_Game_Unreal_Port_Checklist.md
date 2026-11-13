# Unreal Engine Port Checklist — Marigold Links

Moving from Godot 4.5 (2D pixel art) to Unreal Engine 5.8 (stylized low-poly, first-person, farming-simulator-style hands-on play with management UI on top). The Godot project stays untouched as the working reference. Every ported system gets checked against the numbers in the **Acceptance Tests** section, which come straight from `test.gd`'s verified output.

*Last updated: Nov 9, 2026. Legend: `[x]` done, `[ ]` not done. "(code written)" means the C++ exists in `UnrealSource/` or `Phase5Source/` but still needs confirming in the project (compiled, data assets created, test passing).*

---

## Status at a glance

- **Done and working in the project:** first-person character, mowing system (mower, cut-mask, travel-direction stripes, daily regrowth), stylized turf material, lighting and the day/night sun, the full nine-hole course (landscape, layers, bunkers, ponds, trees, flags, tee markers), paved cart paths as spline meshes, crisp fairway and green shapes from the coverage zones (shape mask), mowing linked to hole condition (all nine holes have coverage zones), DayCycle clock, HUD widget, clubhouse exterior and interior blockouts with a working fade door, desks interactable.
- **Done and passing `GolfSystemTest` (66/66):** the data layer and Data Assets, CourseManager, StaffSystem, DayCycle, ClubhouseSystem.
- **Code written, needs confirming in the project:** the interaction component, the UI manager and panel base classes.
- **Next:** Kitchen/Bar locked door, interior lighting, tee box and green fringe polish, clubhouse polish (roof, windows), then the panels (applications, hole unlock, hardware store) and save/load.

---

## Phase 0 — Decisions and setup

**Decisions made**
- [x] **Naming convention:** class prefixes stay (`A`/`U`/`F`/`E`, required by Unreal Header Tool), variables are camelCase, functions are PascalCase.
- [x] **Course layout:** one continuous course level, no streaming. Add streaming later only if performance forces it.
- [x] **Clubhouse interior:** changed on Nov 12. The interior is built in the same level, under the map (around X = 102000, 64 x 40 m, same size as the exterior), and the door fades and teleports the player (`BP_ClubhouseDoor`). Reason: world subsystems (coverage) stay alive and there is no level loading. Game Instance subsystems keep all state either way.
- [x] **Art direction:** stylized low-poly, one asset family, one palette, warm lighting. Do not drift toward realism.
- [x] **C++ vs Blueprints split:** systems and data in C++, UI layout and level interaction in Blueprints.
- [x] **Name:** Marigold Links (display name only; the folder, `.uproject` and C++ module stay `GolfGame`).

**Setup**
- [x] Install UE 5.8, create a C++ project.
- [x] Source control with Git + LFS, with a proper `.gitignore` (packaged builds and `Content/Fab/` ignored). Pushed to GitHub (`JCBontop19/marigold-links`) with a bare-repo backup.
- [ ] Folder structure: `Content/Data`, `Content/UI`, `Content/Characters`, `Content/Course`, `Content/Clubhouse`, `Source/GolfGame/Systems`, `Source/GolfGame/Data` (partly in place; tidy as content grows).
- [ ] Read the current Unreal EULA royalty terms (this is a game you plan to release).
- [ ] Check commercial-use license on every Fab pack before using it, same standard you applied to fonts.
- [x] Keep the Godot project folder intact as the reference spec.

---

## Phase 1 — Data layer

Godot `.tres` resources become Unreal **Data Assets** (`UPrimaryDataAsset` subclasses).

**Design change made:** Data Assets stay **read-only definitions**. Anything that changes during play lives in **runtime state structs** held by the subsystems. This also makes save/load much simpler later.

- [x] `UHoleData`, `FHoleState`, `UEquipmentData`, `UStaffData`, `FStaffState`, `EStaffRole`, `UApplicationPairData`, `URoomData`, `UFurnitureItemData`, `GolfDataRegistry`, `GolfSettings` (code written in `UnrealSource/Data/`).
- [x] Create the Data Assets in the editor: 9 x HoleData, 4 x EquipmentData, Jake / Maria / Bob / Kyle (StaffData), the application pair, and the registry, with **Project Settings > Game > Golf Game > Data Registry** pointing at it.
- [x] Re-enter all hole data from the table below (holes 1–9).

**Hole data (from your verified test output)**

| Hole | Par | Tier | Cost | Starts unlocked | Fairway |
|---|---|---|---|---|---|
| 1 | 4 | 0 | 0 | yes | 1.00 |
| 2 | 4 | 0 | 150 | no | 0.90 |
| 3 | 5 | 1 | 300 | no | 0.70 |
| 4 | 4 | 2 | 500 | no | 0.40 |
| 5 | 3 | 1 | 250 | no | 0.60 |
| 6 | 4 | 2 | 550 | no | 0.40 |
| 7 | 4 | 2 | 600 | no | 0.30 |
| 8 | 3 | 3 | 700 | no | 0.20 |
| 9 | 5 | 3 | 900 | no | 0.20 |

Godot's Hole 0 (clubhouse exterior) and Hole 10 (clubhouse interior) are dropped. The clubhouse is now a building on the course, so `countsForRevenue` is no longer needed and the revenue calculation simply counts unlocked holes.

---

## Phase 2 — Core subsystems (C++, Game Instance Subsystems)

**CourseManager subsystem** (code written)
- [x] State: hole states, ownedEquipmentTier, playerCash (starts at 1000).
- [x] Load hole definitions through the registry (no hardcoded filenames).
- [x] `GetHole`, `TryUnlockHole`, `PurchaseEquipment`, `ApplyConditionChange` (enum for the stat, clamped 0.0–1.0), cash-changed delegate.
- [x] Confirmed compiled and passing in the project (`GolfSystemTest`: 66 passed, 0 failed, Nov 9).

**StaffSystem subsystem** (code written)
- [x] Roster, rejectedApplicants, `HireStaff`, `FireStaff`, `AssignToHole`, `GetTotalWages`, `GetMaintenanceEffects` (`0.10 * (1.0 + skillLevel)`), `ApplyMoralePenalty`, `RejectApplicant`.
- [x] Confirmed compiled and passing in the project (`GolfSystemTest`).

---

## Phase 3 — DayCycle

- [x] Tickable subsystem in the project: day starts 5:00 (300 minutes), ticks every 10 real seconds advancing 15 in-game minutes.
- [x] Phases: MorningPrep 300–419, OpenHours 420–1139, Close 1140+.
- [x] Exposes `GetCurrentTimeMinutes`, `GetCurrentDay`, `GetCurrentPhase`, and the `OnDayStarted` / `OnPhaseChanged` delegates. Already used by the HUD, the sun (`BP_DayNightLighting`) and grass regrowth.
- [x] Penalty at midnight, close processing, `CloseOutDay`, `dayReportReady` and `midnightPenaltyTriggered` delegates, and the 12-hour time format (code written).
- [x] Close-out and penalty path pass `GolfSystemTest` (day advances, -100 cash, morale 1.00 to 0.85).
- [ ] **Lessons from the Godot bugs:** use `>=` for threshold checks, keep phase transitions as sequential checks, and make sure the clock actually stops when the game is paused or any UI panel is open.

---

## Phase 4 — ClubhouseSystem

- [x] Tier, rooms, built flags, placed items, `CanUnlock`, `BuildRoom`, `UpgradeClubhouseTier`, `PlaceItem` (with the conflict-loop fix), `RemoveItem`, `GetItemsInRoom`, delegates (code written).
- [x] Confirmed compiled and passing in the project (`GolfSystemTest`).

---

## Phase 5 — UI (UMG)

Base classes written in `Phase5Source/` (interaction component, `IInteractable`, `AGolfDesk`, `UGolfUIManager`, `UGolfPanelBase`, `UGolfHudWidget`).

- [x] HUD widget (`WBP_Hud`): day, time, phase, cash. Created and added to the viewport at BeginPlay.
- [ ] Confirm the HUD binds to delegates (no polling) and hides while a panel is open.
- [ ] Choose new fonts (OFL or CC0 only). Pixelify Sans was chosen for pixel art and may not suit the new look.
- [x] Report panel (opens only during Close, closes the day).
- [ ] Application panel (two applicants, hire one, reject the other).
- [ ] Hole unlock panel (dynamic list, buttons disabled by tier/cash).
- [ ] Hardware store panel (four tiers, already-owned tiers locked out).
- [ ] Interaction prompt widget ("Press E to ...").
- [ ] Input mode switching: Game vs Game-and-UI, mouse cursor show/hide, and game pause while panels are open.
- [ ] Shared widget style: fonts and label styles defined once.

---

## Phase 6 — World and gameplay foundation

**Done**
- [x] First-person character (`BP_FirstPersonCharacter`): movement, look, mowing input.
- [x] **Course greybox and beyond:** new 630 m landscape, `M_Turf` layers (Rough, Fairway, Green, Sand, Path) with an unmown look, nine fairways and greens routed as a loop from the clubhouse, fringe, bunkers (dug), five ponds with a stylized water material, rolling terrain with a raised rim, trees (foliage tool, rough only), cart paths (Landscape Splines on an Edit Layer), flagsticks, tee boxes and tee markers, blocking volumes along the rim.
- [x] Pick one low-poly asset family (Stylized Nature Set) and set the scale standard.
- [x] Lighting pass: movable sun tied to the day cycle, real-time sky capture, height fog, fixed post-process exposure.

**Still to do**
- [x] Remove the temporary viewing platform and put the PlayerStart on the ground near the clubhouse and tee 1.
- [x] Clubhouse exterior blockout on the course level (64 x 40 m, 5 m walls, rotated about 20 degrees, porch, 2 m door). Polish still open: pitched roof, windows, trim, sign, flower boxes.
- [x] Clubhouse interior blockout (shell, Lobby, Pro Shop front-left, Office back-left, Kitchen/Bar on the right, 2 m doors with lintels), built under the map at X about 102000.
- [x] Door transition: `BP_ClubhouseDoor` (box trigger, camera fade out, teleport player to `Destination`, set control rotation, fade in, `Busy` guard). Exterior door goes to the lobby target point, interior door to the porch target point.
- [x] Desks converted to interactables (`BP_...Desk`, placed in the Office).
- [ ] Interactables still to convert or build: hardware store stand and the remaining panels (see Phase 5).
- [ ] Locked-room behaviour: Kitchen/Bar door shows "Locked" until unlocked (flag on the door; Pro Shop is open from the start).
- [ ] Interior lighting: a point light per room, different floor color per room.
- [ ] Interior polish later: stairs in the Lobby and the second floor for the country club upgrade (guest bedrooms), Pro Shop expansion.
- [x] Whole-landscape mow test: stripes line up and regrowth works on a hole far from the start.
- [ ] Bunker rim polish, per-hole detail pass using Hole 1 as the template.
- [ ] Fairway and green shape polish (Nov 11 shape mask): holes 6 to 9 zones still need adjusting, close the gaps at doglegs, widen with Grow Cm, tee boxes with Corner Radius 0, optional fringe ring zones around greens (Affects Coverage off).
- [x] **Upgrade the cart paths from painted gravel to a real paved path.** The painted Path layer has soft, stair-stepped edges at the landscape's ~1 m resolution. Pick one:
  - [ ] **Option A, paved material:** change the Path layer color in `M_Turf` (both layer blends) to asphalt or concrete: a grey base with fine noise, and for a sidewalk look, slab joints from `Frac(AbsoluteWorldPosition / slabSize)` drawn as thin darker lines. Cheapest, but the edges stay soft.
  - [x] **Option B, spline meshes (crisper, recommended), done Nov 11:** keep the Landscape Splines and fill each segment's **Spline Meshes** list with a flat paved-path or sidewalk mesh (with slight curbs). The mesh bends along the spline and gives hard edges and real geometry. Needs a path mesh with enough length subdivisions, and a paved material (`M_Pavement`).
  - [ ] Decide asphalt vs. sidewalk (sidewalk slabs suit the cozy, low-poly look). Keep a warm, slightly desaturated grey so it fits the palette.
  - [x] Keep the Path layer under the mesh (or a narrow strip of dirt at the edges) so the mesh doesn't look like it's floating on the grass.
  - [ ] Check the path meets each tee box and green edge cleanly, and add a small ramp or flat apron where it meets the clubhouse.

---

## Phase 7 — Hands-on maintenance gameplay

**Done**
- [x] Mower actor attached to the player, painting a cut-mask render target (`RT_CutMask`, 4096 recommended) with travel-direction stripes.
- [x] Daily regrowth driven by the `OnDayStarted` delegate.
- [x] Mower cut width is already a variable (`cutRadiusCm`), ready to be tied to equipment tiers.

**Still to do**
- [ ] **Course condition from coverage** (code written and hooked up Nov 9, zones placed on all holes: `HoleCoverageZone` + `CourseCoverageSubsystem`). At the start of Close it reads the cut mask once, measures each zone, and calls `ApplyConditionChange` scaled by the owned mower tier. Keep the hole-level condition float; do not simulate individual grass blades.
  - [x] Copy the four files into `Source/GolfGame/Systems/`, regenerate project files, build, and re-run `GolfSystemTest` (must still be 66/66).
  - [x] Call `Configure` (cut mask, MowOrigin (-5000, -5000), MowSize 63000) from a Blueprint at BeginPlay.
  - [x] Place `HoleCoverageZone` boxes over each hole's fairway and green (hole 1 first), then verify with `force = true` and the `LogGolfCoverage` output.
- [ ] Equipment tiers affect the mower or tools (cut width, speed), tied to the hardware store.
- [ ] Hazard system: `sandNeedsRaking` flag with a random daily chance, `waterCondition` with slow decay, player interacts from outside the hazard to rake or clean.
- [ ] Player movement speed upgrades.

---

## Phase 8 — Walkie-talkie staff system

- [ ] Walkie-talkie UI listing the roster with each person's role and assigned hole.
- [ ] "Page" action returns the staff member's location or status.
- [ ] Assign/fire staff from this panel.
- [ ] Staff do not need models or pathfinding yet. Add visible staff later only if wanted.

---

## Phase 9 — Save/load (plan early)

- [ ] `USaveGame` covering: cash, day and time, hole states, equipment tier, roster and staff state, rejected applicants, clubhouse state, and the cut-mask render target (or reset it on load).
- [ ] Because definitions are read-only and all runtime state sits in structs, saving is mostly serializing those structs.

---

## Phase 10 — Documents

- [x] Rewrite the backlog for Unreal (see `Golf_Game_Feature_Backlog.md`).
- [ ] Update the GDD: engine, camera, art direction, the hands-on-versus-management pillar decision, the name, and that this is a personal release project rather than a school deliverable.
- [ ] Keep `GolfSystemTest` in a separate test level, never in the real playthrough (it spends cash and unlocks hole 2).

---

## Phase 11 — Carried-over unbuilt features

From the old backlog, still unbuilt and engine-independent:
- [ ] Guests/NPCs and guest satisfaction (the missing core of the loop; design it right after Phase 4 is confirmed).
- [ ] More ApplicationPairData pairs.
- [ ] Browse-the-rejected-pile UI.
- [ ] Staff depth: multi-stat skills, placement matching, promotions, Pro Shop and Instructor roles, morale effects.
- [ ] Furniture placement UI.
- [ ] Kitchen and Bar real contents.
- [ ] Grass tufts, compost, flower beds (marigolds fit the name).
- [ ] Tournaments and the swing minigame.

---

## Build notes (gotchas already hit)

- **Landscape brush Size is a radius**, not a diameter. Sculpt/paint with **Ctrl**; **Ctrl+Shift** inverts (lower, erase).
- **Orthographic Top view** only works if no material node divides by a zero-length camera vector; `M_Turf` no longer normalizes the camera direction.
- **Mow mask:** `MowOrigin` must be (−5000, −5000) and `MowSize` 63000 to match the landscape in `MPC_Course`. Update both if the landscape changes.
- **Foliage:** per-mesh Density and Radius control how many trees one click paints; restrict with the Rough landscape layer.
- **Splines with Edit Layers on:** a spline paints a layer only after you add a layer in the Splines tool and enable the Landscape Edit Layer Splines option. Until then they are just an editor overlay.
- **Landscape paint resolution is about 1 m**, so narrow paths and small bunkers have soft, stair-stepped edges. Fairways and greens no longer use paint: they come from the shape mask (below). Bunkers still use the Sand paint layer.
- **Shape mask (Nov 11):** every `HoleCoverageZone` with Draw Shape on is drawn as a rounded rectangle into `RT_ShapeMask` (R fairway, G green, B rough) by `CourseShapeRenderer`; `M_Turf` reads R and G for the fairway and green Lerps. Needs `M_ShapeStamp` (User Interface, Additive) and the Course Shapes settings in Project Settings > Golf Game. Console: `golf.RebuildShapes`, `golf.ShapeYawSign`. Zone box = visible fairway shape, so resizing a box resizes the fairway.
- **Cart path spline meshes:** each segment needs its own Spline Meshes entry. Control points must sit on the ground (Ctrl+click places them on it); a point set high with Raise Terrain on pulls the terrain into a tower. Set the point Width to cover the mesh plus the side falloff, and add points on hills and cross-slopes.
- **Clubhouse:** exterior is rotated about 20 degrees (centre about (6314, 44331)), so work out door and porch positions in its local axes. Interior sits under the map; keep it fully enclosed, above Kill Z, and light it with its own lights.

---

## Acceptance tests (from `test.gd`'s verified output)

Run these against each ported system. If a number doesn't match, the port is wrong, not the spec.

**Courses**
- 9 holes loaded. Par total = 36.
- Unlocking hole 4 at tier 0 fails.
- Starting cash 1000. Unlocking hole 2 succeeds and leaves 850.

**Staff**
- Hire Jake (cost 80): cash 770. Hire Maria (cost 200): cash 570. Roster size 2.
- Assign Jake to hole 1: succeeds. Assign Maria to hole 4 (locked): fails. Assign Maria to hole 1: succeeds.
- Total daily wages: 105.
- Combined restore on hole 1: 0.30.

**Day cycle**
- Starts MorningPrep at 300.
- Hole 1 fairway knocked to 0.50. After a full day: 0.75 (0.50 + 0.30 restore − 0.05 decay).
- Revenue 400 (two unlocked revenue holes × 200), wages 105, net +295. Cash 570 → 865.
- Clock does not advance during Close.
- After close-out: day 2, phase MorningPrep, time 300, close timer 0.
- Midnight penalty: cash −100 (1160 → 1060), Maria morale 1.00 → 0.85, day auto-advances.
- Firing Jake: roster 1, wages 70.

**Clubhouse**
- Kitchen (tier 0, cost 200) can unlock; Bar (tier 1, cost 400) cannot at tier 0.
- Build Kitchen: cash 1060 → 860. Building it again fails. Building Bar before the tier upgrade fails.
- Tier upgrade (cost 300): cash → 560. Bar now unlockable; build Bar: cash → 160.
- Bar stool in the Kitchen (wrong room) fails. In the Bar at (1,1): succeeds, cash 160 → 110 (stool cost 50).
- Second stool at (1,1) fails (conflict). At (2,1) succeeds.
- Items in Bar: 2. Remove (1,1): true. Remove it again: false. Items remaining: 1.

---

*Living document. Check items off as they're built; update acceptance numbers if design values change.*
