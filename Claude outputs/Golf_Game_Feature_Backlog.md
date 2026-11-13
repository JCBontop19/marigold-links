# Feature Backlog — Marigold Links (Unreal)

Everything floated during design and build sessions that isn't built yet. Rewritten for the Unreal version on Nov 9, 2026: pixel-art tasks (tee tiles, blend tiles, Aseprite work, camera limits) are gone, and the course art items are done. Check items off as you build them. Organized roughly by system. The step-by-step port plan lives in `Golf_Game_Unreal_Port_Checklist.md`.

---

## Course and World

- [x] **Paved cart paths:** spline meshes with crisp edges (a curb and nicer material can come later)
- [x] Clubhouse exterior blockout on the course; [ ] polish (pitched roof, windows, sign, flower boxes)
- [x] Clubhouse interior blockout (Office, Lobby, Pro Shop, Kitchen/Bar) with a fade door; [ ] lighting, locked Kitchen/Bar door, polish
- [ ] Country club upgrade: Lobby stairs, second floor with guest bedrooms, expanded Pro Shop
- [ ] Fairway/green shape polish: fix doglegs and gaps, widen with Grow Cm, square tee boxes, fringe rings around greens
- [ ] Hole-by-hole detail pass using Hole 1 as the template (props, tee signs with hole number and par, benches, ball washers)
- [ ] Marigold flower patches around the greens and clubhouse (foliage tool)
- [x] Remove the temporary viewing platform; PlayerStart on the ground near the clubhouse
- [ ] Hazards behave as hazards: block the player from wading into ponds (or add a penalty trigger), keep the player out of bunkers except to rake
- [ ] Town scene with a hardware store for equipment purchases (or a stand at the clubhouse if scope needs trimming)
- [ ] Ambient audio (birds, wind, mower) and a warm music loop

## Mowing and Maintenance (hands-on loop)

- [ ] **Course condition from coverage:** working on all nine holes (`HoleCoverageZone`, `CourseCoverageSubsystem`); still to do: tune restore strength, then satisfaction and revenue should read the condition
- [ ] Equipment tiers change the mower (cut width, speed) and unlock new tools (weed whacker, aerator, sprayer)
- [ ] Hazard system: `sandNeedsRaking` (random daily chance, binary), `waterCondition` (slow decay, separate rate), rake/clean interaction with an "Press E" prompt, player interacts from just outside the hazard
- [ ] Grass tufts on neglected holes, player collects them
- [ ] Compost bin: raw grass converts over time; compost spent on flower beds (ties into the `flowerBedTheme` field)
- [ ] Open design question: does collecting overgrowth also restore the hole's condition, or are they independent systems?
- [ ] Mower visuals and feel: sound, vibration or camera bob, trail-edge polish on the stripes
- [ ] Soften the stripe edge and calm the neon mown colors in `M_Turf`

## Clubhouse

- [ ] Build real Kitchen and Bar room contents once they're unlockable
- [ ] Furniture placement UI (the `PlaceItem` / `RemoveItem` backend exists)
- [ ] Locked-room signs

## Office and Staff

- [ ] Applications desk and panel: two applicants, hire one, reject the other
- [ ] More `ApplicationPairData` pairs authored
- [ ] Browse-the-rejected-pile UI (`rejectedApplicants` already stores everyone passed over)
- [ ] Walkie-talkie UI: roster with role and assigned hole, "page" action, assign and fire from the panel
- [ ] Currently only Groundskeeper hires do anything (see Staff Depth)

## Staff Depth (deferred to later)

- [ ] Multi-stat skill system per staff member (e.g. Sociability, Intelligence, a "Special Skill")
- [ ] Placement-matching: staff perform better when placed in a role or area that fits their skills
- [ ] Staff promotion: pay to raise a staff member's skill ceiling
- [ ] Pro Shop and Instructor roles do something
- [ ] Morale affects performance, wages or quitting (it only decreases from the midnight penalty today)
- [ ] Visible staff models and simple pathfinding (optional, only if wanted)

## UI

- [x] Report panel
- [ ] Hole unlock panel, hardware store panel (see the Port Checklist, Phase 5)
- [ ] Interaction prompt widget
- [ ] New fonts (OFL or CC0 only) that suit the low-poly look
- [ ] Shared widget style defined once
- [ ] Hide the HUD whenever another panel is open

## Player and Feel

- [ ] Movement speed upgrades as a progression system
- [ ] First-person hands and mower animation (the player has no body or hands yet)
- [ ] Footstep sounds that change with surface (fairway, rough, gravel path, sand)

## Systems and Tech

- [ ] Save/load (`USaveGame` for all runtime state, plus a plan for the cut mask)
- [x] `GolfSystemTest` passes against the C++ systems (66/66, Nov 9); keep it in its own test level
- [ ] Performance pass: foliage culling distances, landscape LOD distance scale, shadow cost of the tree canopy
- [ ] Packaged build test on a second machine

## Release and Legal

- [ ] Check the commercial license on every Fab pack in use
- [ ] Read the current Unreal EULA royalty terms
- [ ] Steam page, store art and screenshots (keep a `Docs/` folder of screenshots in the repo)
- [ ] Final polish after graduation

## Not Started At All (from the original GDD, broader scope)

- [ ] Guests/NPCs
- [ ] Guest and member satisfaction system
- [ ] Tournaments
- [ ] Swing minigame (lessons, tournaments, testing holes)

---

## Done since the Godot backlog

- Course art: real tee boxes, fairway/rough/green/sand/path blends, fringe, water, bunkers, trees and flags are all built for the Unreal course
- Crisp fairway and green shapes from the coverage zones (procedural shape mask), spline-mesh cart paths
- Coverage zones on all nine holes feeding hole condition
- Clubhouse exterior and interior blockouts with a working fade door; desks are interactable
- Mowing loop: mower, cut mask, direction stripes, daily regrowth
- Day/night lighting tied to the day cycle
- DayCycle clock and HUD

---

*Living document. Add new ideas here as they come up, check off or delete things as they get built.*
