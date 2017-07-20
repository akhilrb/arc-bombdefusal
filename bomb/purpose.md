## Purpose

Simple configurations like making a bomb glow red while armed and green while quarantined give the game an aesthetic appeal.<br>
This allows for a more audience friendly gameplay including only a bit more complexity.

<br>
## Bombs

The code is written into the bombs which control the bomb state and make it more interactive.<br>

Current planned scheme:

|Colour|Message|Blink Rate|
|---|---|---|
|Red|Armed[Stationary]|1 second|
|Red|Armed[Picked up]|0.5 second|
|Green|Quarantined|2 seconds|

<br>
## Bomb Manager

The bomb manager is manually operated and configures all bombs using a serial.<br>
A limitation is that only 5 bombs can be controlled this way.