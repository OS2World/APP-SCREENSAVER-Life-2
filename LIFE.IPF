:userdoc.
:h1 res=128.Extended help
:artwork name='logo.bmp' align=center.
:font facename='Tms Rmn' size=16x14.
:xmp.
Life Saver - An OS/2 2.x Screen Saving Utility
Copyright (c) 1993 by Larry Salomon, Jr.
All rights reserved.
:exmp.
:font facename=default.
:p.:hp2.Life Saver:ehp2. is a screen saver that runs in the background without
disturbing your other applications.  As long as the foreground session is the
Presentation Manager, Life Saver will prevent monitor burnout by switching to
a black screen with the :link reftype=hd res=4096.Game of
Life:elink. in progress after a specified amount of time has elapsed with no
activity from the mouse or keyboard.
:p.Related information
:ul compact.
:li.:link reftype=hd res=4096.Game of Life:elink.
:li.:link reftype=hd res=4097.Settings:elink.
:li.:link reftype=hd res=4098.Buttons:elink.
:eul.
:h1 res=4096 hide.Game of Life
:p.The Game of Life is an ancient computer science exercise in which you have
a two-dimensional &odq.universe&cdq. containing spaces that either contain an
&odq.organism&cdq. or are empty. After each unit of time, the entire board is
scanned and the next state of the board is determined according to the
following rules&colon.
:p.If the current space is occupied and the number of occupied adjacent spaces
is...
:ul compact.
:li....0 or 1 the organism dies of loneliness.
:li....2 or 3 the organism continues to live.
:li....4 or more the organism dies of overcrowding.
:eul.
:p.If the current space is unoccupied and the number of occupied adjacent
spaces is...
:ul compact.
:li....0 or 1 the space remains empty because there are not enough parent
organisms.
:li....2 or 3 a new organism is born.
:li....4 or more the space remains empty because of overcrowding.
:eul.
:p.The spawning and death of organisms happens instantaneously; thus, an
organism spawned in a turn cannot then be a parent to an adjacent space in
the same turn.
:p.It is highly likely that, after a finite amount of time, the universe
enters statis and does not evolve any further.  To avoid this,
&odq.floaters&cdq. are introduced periodically.  A floater is a group of
organisms that, by nature of their arrangement, cycle through a small set of
patterns and move a space or more in the process.
:p.It is also possible that the universe will die altogether; nothing is done
about this condition.
:h1 res=4097 hide.Settings
:p.The settings that can be adjusted are listed below&colon.
:dl tsize=15 compact break=fit.
:dt.Timeout
:dd.The amount of inactivity (measured in minutes&colon.seconds) that must
elapse before the screen saver assumes control.
:dt.Disable...
:dd.Specifies that inactivity should not be monitored.
:edl.
:p.Remember that changes to the settings do :hp2.not:ehp2. take effect until
either Set or Apply is also selected.
:h1 res=4098 hide.Buttons
:p.The buttons at the bottom of the Settings panel are listed below&colon.
:dl tsize=15 compact break=fit.
:dt.Set
:dd.Saves the current settings to be used the next time Life Saver is started.
:dt.Apply
:dd.Uses the current settings but does not save them.
:dt.Undo
:dd.Resets the setting values to reflect the values currently being used.
:dt.Help
:dd.Displays this help.
:edl.
:euserdoc.
