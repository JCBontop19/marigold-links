# Marigold Links

A cozy first-person golf course management game. You run a 9-hole course: hire and assign staff, maintain the course, upgrade the clubhouse, and keep the business profitable one day at a time.

**Engine:** Unreal Engine 5 (C++ and Blueprints) · **Status:** In development (solo)

Marigold Links started as a 2D pixel-art prototype in Godot 4.5. It is now being rebuilt in Unreal Engine 5 as a stylized low-poly 3D game, with core systems written in C++ and player interaction in Blueprints.

![Gameplay screenshot](docs/screenshot.png)

## Features

- **9-hole course** routed to start and end at the clubhouse, with hazards placed for strategy (forced water carries, driving-zone bunkers), not decoration
- **Staff management:** hire from paired candidates with trade-offs, assign roles, and handle course maintenance
- **Real-time day cycle:** 5 AM prep, 7 AM–7 PM open hours, then close. A full day is about 15–20 real minutes
- **End-of-day revenue report:** the calendar only advances once the player reviews and closes the report
- **Hazard maintenance:** bunkers randomly need raking each day; water quality decays gradually and needs cleaning
- **Upgradable clubhouse** with room tiers and placeable furniture

## Architecture

Gameplay is split into independent manager systems, first built and tested in the Godot prototype and now being ported to C++:

| System | Responsibility |
| --- | --- |
| `CourseManager` | Hole unlocks, equipment ownership |
| `StaffSystem` | Hiring, role assignment, maintenance tasks |
| `DayCycle` | Real-time clock, open/close loop, day advancement |
| `ClubhouseSystem` | Room tiers, furniture placement |

The course is one continuous level, and the clubhouse interior is a separate small level. C++ owns the game state and rules; Blueprints handle interaction and presentation.

## Running the project

1. Install **Unreal Engine 5** through the Epic Games Launcher, plus **Visual Studio** with the "Game development with C++" workload.
2. Clone this repo, right-click the `.uproject` file and choose **Generate Visual Studio project files**.
3. Open the `.uproject` file and let it compile, then press **Play** in the editor.

## Controls

| Action | Input |
| --- | --- |
| Move | WASD |
| Look | Mouse |
| Interact | E |

## Roadmap

- [ ] Finish porting the core systems to C++
- [ ] Multi-stat staff skills (e.g. Sociability, Intelligence) where placement affects revenue and guest satisfaction
- [ ] Town scene with a hardware store for equipment purchases
- [ ] Walkie-talkie UI for directing staff

## Author

Jacob Davis · [GitHub](https://github.com/JCBontop19)
