# HUD Widgets With C++

This project uses a small modular HUD layer so each HUD feature can live in its own C++ component.

## Overview

- `APlayerHudController` creates the HUD only for the local player.
- `UHudWidget` owns the root screen widget and rebuilds its child components.
- `UHudWidgetComponent` is the base type for modular HUD pieces.
- `ULifeHudWidgetComponent` is the first example component.

## Life Widget Example

The life widget reads the current pawn's `UStatsComponent` and displays health as a white bar over `Assets/life.png`.

It is configured with:

- `ScreenOffset` for bottom-left placement
- `WidgetSize` for the overall widget footprint
- `BarOffset` for bar placement inside the texture
- `BarSize` for the fine-tuned bar length and thickness

## Adding A New HUD Component

1. Create a new class derived from `UHudWidgetComponent`.
2. Override `BuildWidget`.
3. Use the owner widget's `WidgetTree` to create your Slate/UMG widgets.
4. Add your new component to the `Components` array on `UHudWidget` or to the controller's HUD setup.
5. Read gameplay state from the owning pawn or its components, not from the server directly.

## Example Pattern

For a future inventory widget, you can follow the same pattern as life:

- derive `UInventoryHudWidgetComponent` from `UHudWidgetComponent`
- read the pawn's inventory component
- create the inventory panel in `BuildWidget`
- add the component to the HUD widget's component list

## Notes

- The HUD is only created for `IsLocalController()`, so it stays client-side.
- The life bar should be easy to tune by changing only the component defaults.
- To add more HUD features later, create more `UHudWidgetComponent` subclasses instead of growing one monolithic widget.
