# ResearchProjectGP
## Finite state machines and behavior trees

### Goal
I will look into both finite state machines and behavior trees to find out their pro's and cons, then combine these two decision making structures to try and keep as many of these pro's and negate or reduce the cons.

### FSM
#### Structure
Finite state machines, FSM for short, use states and transitions for their logic. A state *does* something, while a transition *checks* something. Normally, every state has at least one transition. One transition would be used to either go away from this state, in case this particular one only needs to be called once at the start of the game, or go to this state, for example when the character dies. States can have mulitple transitions, both incoming and outgoing, and with that, a first problem pops up: FSMs can get really complex really quickly. If you keep in mind that every state *can* have two transitions (back and forth) for every other state, and AI rarely has only 2 or 3 states, the structure of an FSM becomes almost spiderweb-like - only less clean and even less easy to manage. This complexity can make debugging of transitions really, really difficult.

##### Hierarchical FSM
To take away part of this complexity, you can use hierarchical finite state machines. These are basically layered state machines, in which a state can - but doesnt necessarily - hold another FSM. This makes complex behavior simpler to build up, as you can make overarching states that then deal with the current situation by refining the agent's behavior further with another FSM.
In a shooter for example, the AI of a gangmember could look something like this:

![alt text](https://github.com/BlackSilverFox/ResearchProjectGP/blob/main/HierarchicalFSM.png)

With this, you only have to deal with two relatively simple finite state machines, instead of one giant FSM. Tracking down errors and fixing them will also become easier.

#### transition problems - Remembering last used state
If your FSM begins getting complex, another problem pops up. Take this simple scenario: a mouse goes out to search cheese ( = starting state), if he found cheese, he returns to his hole, and if the cursor somes to close, he runs away.

![alt text](https://github.com/BlackSilverFox/ResearchProjectGP/blob/main/SimpleFSM_1.png)

This will give no problems: when the cursor moves away, the mouse will automatically go back to finding cheese. Yet with this small FSM, there is one weird thing going on: once the mouse has cheese, he will not run away from the cursor anymore. If this cursor represents a cat, this mouse is, at the moment, quite suicidal, no? To fix this, we can do this:

![alt text](https://github.com/BlackSilverFox/ResearchProjectGP/blob/main/SimpleFSM_2.png)

Now, the mouse will also run away when he found cheese - great! But hold on, there's still a problem: if the mouse stops running away, what state should it return to? The transitions only check if the cursor is close or far away, not if the mouse is carrying cheese or not. In this small FSM, you could simply add this to the transitions: one transition would become "has cheese and cursor is far away" and the other "has no cheese and cursor is far away". In small scale finite state machines, this is manageable - after all, if you only have a few states and a few transitions, one extra transition won't change that much.
Now imagine having to do this in a complex state machine, with tens of states and many transitions - if you need another transition for every small change in behavior, this machine's transitions will become a hell of bools and enums.
Now, there is a way to avoid this. Stack-based finite state machines will automatically return to the last used state. In the case of the mouse, it would "remember" it was searching cheese, and after running away far enough, would automatically go back to finding cheese. The same goes for already having cheese.

##### Stack-based FSM
Instead of simply keeping the "current state" in the FSM, it will keep a stack of what states are active - any LIFO (Last In First Out) container or container capable of LIFO will do.
Every state is responsible for it's own popping, and for the pushing of another state on this stack. This means that the function `SetState(FSMState state)` in the FSM class will be replaced by these two functions: `PushState(FSMState state)` and `PopState()`.
In the case of the mouse and the cheese, this would mean that going between "search cheese" and "go home" will both pop and push a state, while going to "run" will only push a state. When "run" pops itself when it's not necessary anymore, the state left on the stack will be the previously used state - either "go home" or "search cheese", and the FSM will automatically go back to this state.

![alt text](https://github.com/BlackSilverFox/ResearchProjectGP/blob/main/stackBasdFSM.gif)

*push and pop combinations of states - grey states are the active ones*

*Example heavily based on https://gamedevelopment.tutsplus.com/tutorials/finite-state-machines-theory-and-implementation--gamedev-11867*

#### Transition problems - State oscillation
It can happen that AI switches really fast between two states, causing jittery behavior known as "state oscillation". This is a problem withing the transtions. For example, if on transition checks if `speed > 10.f`, and the returning transition checks if `speed < 10.f`, it is possible for an agent to quickly switch between the two states if it's speed is hovering around this 10.f. Better would be to check for > 11.f and < 9.f.

#### State predictability and complexity - Fuzzy state machines
If you want multiple behaviors and conditions to smoothly work together, you will need a lot of different states. To go back to our gangmember trying to kill the player, you could have these states: shoot player while in car, shoot player while on foot, chase player while in car, shace player while on foot. If we break these down, we get these four different behaviors: shoot player, chase player, drive car, run. What if we could keep these apart and still create all of the needed, more specific behaviors?
For this, a Fuzzy state machine, or FuSM, can be a solution.
An FuSM doesn't have any transitions, instead, it only keeps track of variables that are important to the decision making - things like health, enemies in sight, ammo etc. Based on these values, it will use fuzzy logic to compute what states should be active and to what degree. This makes it possible to have multiple states running at the same time, and like this is would be possible to keep the number of states to a minimum, as you can combine different states into new ones.
With this, a normal, clean-cut and obvious (on/off) FSM becomes more unpredictable, and while the setup of an FuSM might be more difficult than the setup of another kind of FSM, it will keep states and combinations of states simpler once build correctly.

#### Resources used on FSM
* normal / stackbased / hierarchical FSM:
  * https://gamedevelopment.tutsplus.com/tutorials/finite-state-machines-theory-and-implementation--gamedev-11867
  * https://research.ncl.ac.uk/game/mastersdegree/gametechnologies/previousinformation/artificialintelligence1finitestatemachines/2016%20Tutorial%208%20-%20Finite%20State%20Machines.pdf
* Fuzzy state machines and fuzzy logic:
  * https://research.ncl.ac.uk/game/mastersdegree/gametechnologies/previousinformation/artificialintelligence1finitestatemachines/2016%20Tutorial%208%20-%20Finite%20State%20Machines.pdf
  * http://www.byond.com/forum/post/37966
  * https://flylib.com/books/en/2.71.1.296/1/
