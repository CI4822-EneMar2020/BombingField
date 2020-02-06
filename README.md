## Statement

In a very far, far away country, the Military Command (MC) is planning how to destroy an
enemy battle field. However, in the same field is located a populated area, soy they must be
very careful.

In a simulated scenario, the battle field is described as square area of N x N yards,
where 100 <= N <= 10000. Yards are numbered 0 to N-1.
In this simulated scenario, Military Targets (MT) are marked as negative numbers, where
the number describes the “strength” of the target. So, a -8 target is stronger than an -2 one.

Civilian Targets (CT) are marked in the same fashion, but using positive integers. So, an CT of 8
is more important than an CT of 2. Yards with no interest, are marked as 0.

The MC experts program a series of attacks, with special bombs that can destroy square
areas. Each of such bomb, has two (2) parameters: the size of the area to destroy and the
power itself. Power is an integer number, so its effect on the field is to decrease the number of
each yard affected by the bomb in “power” units, if the yard is an CT, and to increase the
damage, in the case of MT.

After a number of attacks, MC wants to know:
- How many MT where totally destroyed
- How many MT where partially destroyed, i.e., final number is lower than 0
- How many MT were not touched
- Hoh many CT where affected partially by the bombing
- How many CT where totally destroyed
- How many CT where not touched
Your mission is to write a parallel program that helps MC to take such information

## Input Format
Simulated scenarios values are to be read from standard input. The first number
describes the size N of battle field. In a second line, the number T of targets, including both CT
and MT. Then, T lines, with three integers: coordinates X, Y (0<= X,Y <= N-1) and value of the
target (positive for CT, negative if a MT).
After the T lines, a number B describing the number of attacks planned by the MC. Each
attack is described by four numbers: coordinates X, Y (0<= X,Y <= N-1), the size R of the
square radius of the bomb, and the power (P) of the bomb. Square radius means: given X and
Y coordinates of the bomb, each yard within (X-R,Y-R) and (X+R,Y+R) is affected by the bomb
in P units.

The number B of attacks has no limits, and certain X,Y coordinates could be repeated.
Bombs has effect only in the battle field, so coordinates less than 0 or greater than N-1 must not
be considered.

It is guaranteed there are at least one CT and one MT, and that no CT’s neither MT’s has
the same coordinates.