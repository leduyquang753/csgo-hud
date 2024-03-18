# Lê Duy Quang's CSGO HUD

A custom heads-up display implementation for spectating _Counter-strike: Global offensive_ competitive matches,
written in C++ using Win32 API and Direct2D with the aim to be less of a hit to the computer's resources.

![The start of the round. Showing player stats and team buy pane.](https://imgur.com/NOYfgUp.png)
![Round live, players are being damaged. Showing utility summary and round history.](https://imgur.com/wIjRQVR.png)
![Bomb being defused. Showing the bomb timers and bomb location.](https://imgur.com/cOmIFlC.png)
![The end of the round. Showing win/lose component.](https://imgur.com/pWOupU2.png)

## Features

- HUD elements:
	- Team names and score.
	- Round phase clock.
	- Round number display.
	- Player information: spectator slot, name, health, all held guns and grenades (with active status and empty
	magazine indication), armor type, C4/defuse kit indicator, money; kills, deaths, equipment value, money spent in
	current round; flash and burning indication.
	- Team statistics: total amount of utilities (grenades), total equipment value, total money spent in current round.
	- Active player information: name, health, armor, match statistics (kills, deaths, assists, MVPs, score), current
	weapon name and ammo.
	- Two-level minimap, showing player positions and orientations, flash and burning indications, bomb carrier,
	dropped/planted bomb, grenades being thrown, thrown grenade effects (explosion, flash, smoke, fire), bomb
	explosion effect.
	- Bomb timer, showing the site that the bomb is planted, and defuse timer.
	- Winning and losing team display, showing round win condition (elimination, timeout, detonation, defuse), amount
	of income for each team, win streak for winning team, lose bonus level for losing team.
	- Timeout status display and match points count display.
	- Round history display.
- Utility elements:
	- Alignment corners.
	- FPS counter.
- Customizations:
	- Team colors and default names.
	- Font family.
	- Display formattings.
	- Weapon names.

## Usage

1. Download [the assets package](https://drive.google.com/file/d/1Pcxbf-g8BBbPgqIk1PzXQQBnYo4fRdCv/view?usp=sharing),
which includes the default configuration files, and extract the contents into the same location as the executable.
Adjust the configurations as necessary.
2. Place the file `gamestate_integration_hud.cfg` into the `csgo\cfg` folder in the game installation directory.
Adjust the port value accordingly if you chose to use a different port in the HUD's configuration.
3. Launch the game and the HUD. The HUD will show as a transparent, overlaying window. Move the game's window so that
it aligns with the HUD window, then you can press the corresponding key to turn off the alignment corners.
4. Spectate a match normally, the game will send data to the HUD through HTTP which the HUD will process and display
accordingly.
5. You can press various keys to control certain aspects of the HUD as needed.
6. To close the HUD, you can press Alt+F4 while focusing on the HUD window, or press close from the window preview or
taskbar button menu.

## Configurations

### `Configuration.json`

- `httpServerPort`: The port of the HUD's HTTP server that the will receive JSON data from the game.
- `resolution`: The resolution of the HUD window in pixels, which should match the game's.
- `scaling`: The scaling factor of the HUD's elements, defined as a fraction. The HUD is primarily designed around
the 1280 . 720 resolution, so you may want to base the scaling on this.
- `startingPositionCenter`: If `true`, the HUD's base position will be to the center of the screen; otherwise it will
be to the top-left corner.
- `offset`: The offset of the HUD window in pixels from the base position. This is useful so that you can launch the
HUD and have it positioned perfectly on top of the game when the game is left at the default position after launch.
- `fontFamily`: The font family name used for text elements, one that has been installed into your PC. The font used
in the game is "Stratum2".
- `fontOffsetRatio`: The vertical offset ratio which will be multiplied with the font size to offset drawn text such
that it is positioned correctly vertically. Technically, this should be the ratio of the vertical distance from the
top line of the font to the top of the "0" digit compared to the vertical distance from the top line to the bottom
line of the font. You can choose to just fiddle with this number until it looks nice.
- `fontLineHeightRatio`: The ratio of the vertical distance from the top to the bottom of the "0" digit compared to
the vertical distance from the top line to the bottom line of the font. This is used to center the text vertically.
Just as above you can fiddle until it looks sorta correct.
- `defaultTeamNames`: When teams don't use a custom name, these will be the default names to display.
- `timings`: The time in whole milliseconds of each phase in the match. Used to show a gauge in the background of
the clock. `0` can be used to indicate the time is not available.
	- `freezeTime`: The time at the beginning of each round when all players stand still at their starting positions
	and buy equipment.
	- `mainTime`: The time available for the terrorists to either plant the bomb or eliminate all counter-terrorists.
	- `bombTime`: The time delay from when the bomb is planted until it starts its detonation sequence.
	- `roundEnd`: The time at the end of each round when the outcome has been determined and players may still
	roam freely to either scavenge or continue the fights.
	- `halfTime`: The time at the end of the first half, before teams switch sides.
- `formatting`: Options to change how certain HUD elements are presented.
	- `addCurrentClipToTotalAmmo`: If `true`, the current number of rounds in the current gun's magazine is added to
	the reserve ammo amount to show the total number of rounds left for the gun. If `false`, only the number of
	reserve rounds is shown.
	- `decimalSeparatorComma`: If `true`, the decimal separator for fractional numbers is the comma; otherwise it's
	the dot.
	- `showSecondsSign`: When the time amount is less than one minute, if `true`, a double quote denoting seconds
	will be appended to the number of seconds. This can serve to reduce ambiguity when only a single number is shown.
	- `dollarSignAfter`: When displaying money amounts, if `true`, the dollar sign will be appended after the number;
	otherwise it will be prepended before the number.
	- `dollarSignWithSpace`: Whether to add a separating space between the dollar sign and the number when displaying
	money amounts.
	- `showTenthPlayerAsZero`: There are typically ten players in a match, the first nine of which are numbered from
	1 to 9 as usual. If `true`, the tenth player will be numbered 0, which corresponds to the typical keyboard key
	used to spectate the player. If `false`, they will be numbered 10 to follow natural numbering.
- `colors`: Each configurable color is specified as an array of 4 decimal numbers from 0 to 1, each number in order
specifies red, green, blue and alpha.
	- `ctPrimary`: The primary color of the counter-terrorists team. Used for highlighted backgrounds and text.
	- `ctSecondary`: The secondary color of the counter-terrorists team. Used for minor backgrounds.
	- `ctSmoke`: The color of the smoke clouds thrown by counter-terrorists on the minimap.
	- `tPrimary`: The primary color of the terrorists team.
	- `tSecondary`: The secondary color of the terrorists team.
	- `tSmoke`: The color of the smoke clouds thrown by terrorists on the minimap.
	- `damage`: The color used to show the amount of damage received recently of each player.
	- `notEnoughTimeToDefuse`: This color is used instead of the counter-terrorists' primary color in the bomb timer
	component when the bomb will detonate before the defuse process has the time to finish.
- `keybindings`: Each key is specified as a hexadecimal code (without `0x`) which can be obtained from
[this page](https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes).
	- `toggleHud`: Toggles the visibility of all HUD elements. When off, the HUD window becomes completely invisible.
	- `toggleCorners`: Toggles the visibility of the four alignment corners.
	- `toggleForceShowTeamBuy`: Normally, a pane showing total team buy amount and equipment value is shown at the
	beginning of each round in place of the active player's information. When on this forces the pane to show also
	outside of that time frame.
	- `toggleUtility`: Toggles the summary of each team's utilities (grenades).
	- `toggleRoundHistory`: Toggles the round history pane, which replaces both the active player's info and the team
	buy pane to the bottom of the HUD.
	- `toggleFps`: Toggles an FPS counter to the top-left corner of the HUD so as to verify the HUD is running
	smoothly.
	- `toggleKeybindings`: Toggles whether all the keybindings above are active. This is useful when some other work
	needs to be done beside controlling the HUD.

### `Weapon names.json`

This allows changing the name of each weapon to be displayed in the HUD.

### Minimaps

The `Minimaps` directory contains the minimap images for each map. Each map has a subfolder with the same name as
that map's in-game identifier, containing firstly `radar.png` which is the minimap image itself and secondly a
`Configuration.json`:

- `name`: The display name of the map. Currently not used, but future versions will.
- `topLeft`: The X and Y coordinates corresponding to the top-left corner of the minimap image.
- `scale`: The number of in-game units each pixel of the minimap image corresponds to, specified as a fraction.
- `lowerLevel`: This section is present if the minimap depicts two levels and describes properties of the lower
level.
	- `offset`: The offset in pixels compared to the normal upper level.
	- `separationHeight`: The Z coordinate to act as the center of the transition between the two levels.
	- `transitionRange`: The height range centered on the `separationHeight` that entities transition between the two
	levels.
- `bombsites`: Describes bombsite locations.
	- `bombsiteACenter`: The coordinates of bombsite A.
	- `bombsiteBCenter`: The coordinates of bombsite B.
	- `distinguishingAxis`: The axis, `"x"`, `"y"` or `"z"` specifying the axis to distinguish the two bombsites when
	determing which bombsite the bomb is planted.
	- `distinguishingValue`: The value on the chosen `distinguishingAxis` that puts each of the two bombsites to one
	side.

## Building

The project uses [SIMDJSON](https://github.com/simdjson/simdjson) as the parser for JSON data that the game sends
through HTTP. Place the library's release source files into the `json` directory of the source tree.

The project is built using Visual studio 2022 with the Desktop development with C++ workload with no additional
configuration needed.

## Contributing

This project is currently in its early stages of development by a single person and would benefit greatly from
additional cooperators, or simply some interest from the community will be also very appreciated so that motivation
can fuel up the fingers rattling on the keyboard.